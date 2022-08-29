#include "assembler.h"
#include "tokenParser.h"
#include <fstream>
#include <iostream>
#include "symbolTable.h"
#include <queue>

map<string, int> Assembler::InstructionCode = {
    {"halt", 0},
    {"int", 16},
    {"iret", 32},
    {"call", 48},
    {"ret", 64},
    {"jmp", 80},
    {"jeq", 81},
    {"jne", 82},
    {"jgt", 83},
    {"xchg", 96},
    {"add", 112},
    {"sub", 113},
    {"mul", 114},
    {"div", 115},
    {"cmp", 116},
    {"not", 128},
    {"and", 129},
    {"or", 130},
    {"xor", 131},
    {"test", 132},
    {"shl", 144},
    {"shr", 145},
    {"ldr", 160},
    {"str", 176},
    {"push", 192},
    {"pop", 193}
};

map<string, int> Assembler::RegCode = {
    {"r0", 0},
    {"r1", 1},
    {"r2", 2},
    {"r3", 3},
    {"r4", 4},
    {"r5", 5},
    {"r6", 6},
    {"sp", 6},
    {"r7", 7},
    {"pc", 7},
    {"psw", 8}
};

Assembler::Assembler(string i, string o){
  this->in = i;
  this->out = o;
	this->curSection = "";
	this->curSectionPointer = nullptr;
	this->symbolTable = new SymbolTable();
	this->locationCounter = 0;
}

void Assembler::assembly(){

  ifstream ins;
	ins.open(this->in);

  TokenParser tokenP(ins);
  assemblyText = tokenP.getAssembly();

	ins.close();
  
	try{

		pass();

		if(!checkSymbols()){
			cout << "Symbol undefined" << endl;
			return;
		}
		doRelocations();
		write();

		writeBin();

  } catch(runtime_error r){
		cout << "Runtime error " << r.what() << endl;
	} catch(exception e){
    cout << "Desio se izuzetak" << endl;
  }

}

void Assembler::pass(){

	string curToken;

	for(const auto &line: assemblyText){

		queue<string> tokenStream;
		for(const auto &token: line){
			tokenStream.push(token);
		}

		curToken = tokenStream.front();
		TokenType tt = TokenParser::parseToken(curToken);

		if(tt==TokenType::LABEL){
			processLabel(curToken);
			tokenStream.pop();

			if(tokenStream.empty()){
				continue;
			}else{
				curToken = tokenStream.front();
				tt = TokenParser::parseToken(curToken);
			}
			
		}

		//labela obradjena, nastavljamo sa obradom linije koda
		tokenStream.pop();

		if(tt == TokenType::LABEL){
			throw runtime_error("Label already defined on this line");
		}else if(tt == TokenType::GLB){
			processGlobal(tokenStream);
		}else if(tt == TokenType::EXT){
			processExtern(tokenStream);
		}else if(tt == TokenType::SECTION){
			processSection(curToken, tokenStream);
		}else if(tt == TokenType::SKIP){
			curToken = tokenStream.front();
			tokenStream.pop();
			if(!tokenStream.empty()) throw runtime_error("Illegal skip directive");
			processSkip(stoi(curToken));
		}else if(tt == TokenType::DIRECTIVE){
			processDirective(curToken, tokenStream);
		}else if(tt == TokenType::END){
			processEnd();
			break;
		}else if(tt == TokenType::INSTRUCTION){
			processInstruction(curToken, tokenStream);
		}


	} // for ciklus, obrada jedne linije koda

}

void Assembler::processLabel(string labelNamee){
	string labelName = labelNamee.substr( 0, labelNamee.size()-1 );

	if( !symbolTable->isThere(labelName) ){
		Symbol* newSym = new Symbol(labelName, curSection, Scope::LOCAL, SymbolType::NOTYP, locationCounter);
		symbolTable->addSymbol(labelName, newSym);
	}else {
		if(symbolTable->isDefined(labelName)){
			throw runtime_error("Symbol already defined");
		}else{
			Symbol* mySym = symbolTable->getSymbol(labelName);
			mySym->setProperties(curSection, locationCounter);
		}
	}

}

void Assembler::processGlobal(queue<string>& tokenStream){
	string curSym;
	while( !tokenStream.empty() ){
		curSym = tokenStream.front();
		tokenStream.pop();
		if( symbolTable->isThere(curSym) ) throw runtime_error("Symbol already " + curSym + " declared");
		Symbol* newSym = new Symbol(curSym, Scope::GLOBAL, SymbolType::NOTYP);
		symbolTable->addSymbol(curSym, newSym);
	}
}

void Assembler::processExtern(queue<string>& tokenStream){
	string curSym;
	while( !tokenStream.empty() ){
		curSym = tokenStream.front();
		tokenStream.pop();
		if( symbolTable->isThere(curSym) ) throw runtime_error("Symbol already " + curSym + " declared");
		Symbol* newSym = new Symbol(curSym, "UND", Scope::GLOBAL, SymbolType::NOTYP, 0);
		symbolTable->addSymbol(curSym, newSym);
	}
}

void Assembler::processSection(string curToken, queue<string>& tokenStream){
	if(curSection != "") {
		symbolTable->getSymbol(curSection)->setSize(locationCounter);
		locationCounter = 0;
	}
	
	if(curToken==".text" || curToken==".bss" || curToken==".data"){
		if(!tokenStream.empty()) throw runtime_error("Invalid section" + curToken + "declaration");
	}else{
		curToken = tokenStream.front();
		tokenStream.pop();
		if(!tokenStream.empty()) throw runtime_error("Invalid section" + curToken + "declaration");
	}

	Symbol* newSym = new Symbol(curToken, curToken, Scope::GLOBAL, SymbolType::SCTN, 0);
	symbolTable->addSymbol(curToken, newSym);

	curSection = curToken;
	Section* newSect = new Section(curSection);
	curSectionPointer = newSect;
	sections.insert({curSection, newSect});
}

void Assembler::processSkip(int x){
	locationCounter += x;
	//Section* sect = sections.find(curSection)->second;
	//cout << "Skip instrukcija " << curSection << x << endl;
	curSectionPointer->addZerros(x);
}

void Assembler::processEnd(){
	if(curSection != "") {
		symbolTable->getSymbol(curSection)->setSize(locationCounter);
		locationCounter = 0;
		curSection = "";
	}
}

void Assembler::processDirective(string dir, queue<string>& tokenStream){
	string curToken = tokenStream.front();
	tokenStream.pop();
	if(dir == ".word"){
		SymLit typ;
		while(1){
			
			typ = TokenParser::parseWord(curToken);
			if(typ==SymLit::LIT || typ==SymLit::LITHEX){
				curSectionPointer->addLiteral(curToken, typ);
				locationCounter += 2;
			}else if(typ==SymLit::SYM){
				curSectionPointer->addZerros(2);
				addRel(curToken, RelType::R_386_16, locationCounter, 0);
				locationCounter += 2;
			}
			if(tokenStream.empty()) break;
			curToken = tokenStream.front();
			tokenStream.pop();
		}
		// ubaci sve tokene iz streama
	}else if(dir == ".ascii"){
		curSectionPointer->addString(curToken.substr(1, curToken.size()-2));
		locationCounter += (curToken.size()-2);
		if(!tokenStream.empty()) throw runtime_error("Illegal .ascii directive");
	}
}

void Assembler::processInstruction(string inst, queue<string>& tokenStream){
	int instCode = InstructionCode.find(inst)->second;

	curSectionPointer->addByte(instCode);
	locationCounter+=1;

	if(!inst.compare("halt") || !inst.compare("ret") || !inst.compare("iret")){
		// halt, iret, ret
		return;
	}
	if(!inst.compare("not") || !inst.compare("int") || (instCode&0xF0) == 0xC0){
		// not, int, push, pop
		string regD = tokenStream.front();
		tokenStream.pop();
		int regDCode = RegCode.find(regD)->second;
		curSectionPointer->addByte(regDCode<<4);
		locationCounter+=1;
		return;
	}

	if(!inst.compare("ldr") || !inst.compare("str")){
		// ldr, str
		string regD = tokenStream.front();
		tokenStream.pop();
		int regDCode = RegCode.find(regD)->second;
		int regSCode = 0, addrCode = 0;

		string regS = "", operand = "";
		
		string op1 = tokenStream.front();
		tokenStream.pop();

		if( op1[0] == '$' ) {
			operand = op1.substr( 1, op1.size() - 1 );
			addrCode = 0;
		}else if( op1[0] == '%' ){
			operand = op1.substr( 1, op1.size() - 1 );
			addrCode = 5;
			regSCode = 7;
		}else if( op1[0] == '[' ){
			if( tokenStream.empty() ){
				regS = op1.substr( 1, op1.size() - 2 );
				regSCode = RegCode.find( regS )->second;
				addrCode = 2;
			}else{
				regS = op1.substr( 1, op1.size() - 1 );
				regSCode = RegCode.find( regS )->second;
				string op2 = tokenStream.front();
				tokenStream.pop();
				operand = op2.substr( 0, op2.size() - 1 );
				addrCode = 3;
			}
		}else{
			SymLit sl = TokenParser::parseWord(op1);
			if(sl == SymLit::REG){
				regS = op1;
				regSCode = RegCode.find( regS )->second;
				addrCode = 1;
			}else{
				operand = op1;
				addrCode = 4;
			}
		}

		curSectionPointer->addByte( (regDCode << 4) | regSCode );
		curSectionPointer->addByte( addrCode );
		locationCounter += 2;

		if(operand == "") return;

		SymLit sl = TokenParser::parseWord(operand);
		if( sl == SymLit::LIT || sl == SymLit::LITHEX){
			curSectionPointer->addLiteral( operand, sl, 0 );
		}else if( sl == SymLit::SYM ){
			curSectionPointer->addZerros(2);
			if(addrCode == 5) addRel(operand, RelType::R_386_PC16, locationCounter, -2);
			else addRel(operand, RelType::R_386_16, locationCounter, 0);
		}

		locationCounter+=2;
		return;
	}

	if(!inst.compare("xchg") || (instCode&0xF0) == 0x80 || (instCode&0xF0) == 0x70 || (instCode&0xF0) == 0x90){
		// xchg, logic, aritmeticki, pomeracki
		string regD = tokenStream.front();
		tokenStream.pop();
		string regS = tokenStream.front();
		tokenStream.pop();

		int regDCode = RegCode.find(regD)->second;
		int regSCode = RegCode.find(regS)->second;

		curSectionPointer->addByte((regDCode<<4)|regSCode);
		locationCounter+=1;
		return;
	}

	if(!inst.compare("call") || (instCode&0xF0) == 0x50){
		// call, jumpovi
		int regDCode = 15;
		int regSCode = 0;
		int addrCode = 0;

		string regS = "", operand = "";

		string op1 = tokenStream.front();
		tokenStream.pop();

		if(op1[0] == '%'){
			operand = op1.substr( 1, op1.size() - 1 );
			addrCode = 5;
			regSCode = 7;
		}else if(op1[0] == '*'){
			if(op1[1] == '['){
				if(tokenStream.empty()){
					regS = op1.substr(2, op1.size() - 3);
					regSCode = RegCode.find(regS)->second;
					addrCode = 2;
				}else{
					regS = op1.substr(2, op1.size() - 2);
					regSCode = RegCode.find(regS)->second;
					string op2 = tokenStream.front();
					tokenStream.pop();
					operand = op2.substr(0, op2.size()-1);
					addrCode = 3;
				}
			}else{
				SymLit sl = TokenParser::parseWord(op1.substr(1, op1.size() - 1));
				if(sl == SymLit::REG){
					regS = op1.substr(1, op1.size() - 1);
					regSCode = RegCode.find(regS)->second;
					addrCode = 1;
				}else{
					operand = op1.substr(1, op1.size() - 1);
					addrCode = 4;
				}
			}
		}else{
			operand = op1;
			addrCode = 0;
		}

		curSectionPointer->addByte( (regDCode << 4) | regSCode );
		curSectionPointer->addByte( addrCode );
		locationCounter += 2;

		if(operand == "") return;

		SymLit sl = TokenParser::parseWord(operand);

		if(sl == SymLit::LIT || sl == SymLit::LITHEX){
			curSectionPointer->addLiteral(operand, sl, 0);
		}else{
			// pravimo relokacioni zapis
			if(addrCode == 5) addRel(operand, RelType::R_386_PC16, locationCounter, -2);
			else addRel(operand, RelType::R_386_16, locationCounter, 0);
			curSectionPointer->addZerros(2);
			locationCounter += 2;
		}

	}

}

void Assembler::addRel(string symName, RelType relType, int lc, int addend){

	if( !symbolTable->isThere(symName) ){
		Symbol* newSym = new Symbol(symName, Scope::LOCAL, SymbolType::NOTYP);
		symbolTable->addSymbol(symName, newSym);
	}

	Symbol* mySymbol = symbolTable->getSymbol(symName);

	if(mySymbol->isDefined() && relType == RelType::R_386_PC16 && mySymbol->getSection() == curSection){
		int jmpOff = mySymbol->getOffset() - lc + addend;
		curSectionPointer->addByte(lc, jmpOff);
		curSectionPointer->addByte(lc + 1, jmpOff >> 8);
		return;
	}

	if(mySymbol->getScope() == Scope::GLOBAL){
		Relocation rel(lc, relType, mySymbol, addend);
		curSectionPointer->push(rel);
	}else{
		if(mySymbol->isDefined()){
			Symbol* mySectSym = symbolTable->getSymbol( mySymbol->getSection() );
			Relocation rel(lc, relType, mySectSym, mySymbol->getOffset()+addend);
			curSectionPointer->push(rel);
		}else{
			TempRelocation rel(symName, curSection, lc, addend, relType);
			tempRelocations.push_back(rel);
		}
	}

}

bool Assembler::checkSymbols(){
	return symbolTable->checkSymbols();
}

void Assembler::write(){
	ofstream outs;
	outs.open(out);

	outs << symbolTable << endl;
	outs.flush();

	for(auto & sec: sections){
		outs << sec.second << endl;
	}
	outs.flush();

	for(auto & sec: sections){
		outs << sec.second->getRelTable() << endl;
	}
	outs.flush();

	outs.close();

}

void Assembler::doRelocations(){
	Symbol* curSym;
	Section* curSect;
	for(auto &tr:tempRelocations){
		curSym = symbolTable->getSymbol(tr.symbol);
		curSect = sections.find(tr.section)->second;
		if(tr.relType == RelType::R_386_PC16 && tr.section == curSym->getSection()){
			int jmpOff = curSym->getOffset() - tr.locCounter + tr.addend;
			curSect->addByte(tr.locCounter, jmpOff);
			curSect->addByte(tr.locCounter + 1, jmpOff >> 8);
		}else{
			Symbol* mySect = symbolTable->getSymbol(curSym->getSection());
			Relocation rel(tr.locCounter, tr.relType, mySect, tr.addend + curSym->getOffset());
			curSect->push(rel);
		}
	}
}


void Assembler::writeBin(){
	string outb = out.substr(0, out.size() - 1) + "dat";

	ofstream outsb(outb, ios::binary);

	symbolTable->writeBin(outsb);

	size_t s = sections.size(), size_s;
	outsb.write((char*) &s, sizeof(s));
	for(auto& sec:sections){
		size_s = sec.first.size();
		outsb.write((char*) &size_s, sizeof(size_s));
		outsb.write(&sec.first[0], size_s);
		sec.second->writeBin(outsb);
	}

	outsb.close();

}


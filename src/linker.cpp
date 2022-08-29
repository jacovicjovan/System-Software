#include "linker.h"
#include <iostream>
#include <fstream>
#include "tokenParser.h"
#include <regex>
#include "segentry.h"
#include <iomanip>

Linker::Linker(vector <string> ins, string out, int typ){
  inFiles = ins;
  outFile = out;
  type = typ;
  symbolTable = new SymbolTable();
}

Linker::Linker(vector <string> ins, string out, int typ, vector<SegmentEntry> bases){
  inFiles = ins;
  outFile = out;
  type = typ;
  symbolTable = new SymbolTable();
  baseAdds = bases;
}

void Linker::link(){

  for(int i = 0; i < inFiles.size(); i++){
    linkFile(i);
  }

  for(int i = 0; i < inFiles.size(); i++){
    processEntry(i);
  }

  if(type == 1) {
    writeBinObj();
    writeObj();
    return;
  }

  if(!isExecutable()){
    cout << "Not all symbols are defined" << endl;
    return;
  }

  try{ setBaseAdds(); }
  catch(runtime_error re){ 
    cout << re.what() << endl;
    return;
  }

  updateSymbolTable();
  
  doRelocations();
  writeBinHex();
  writeObj();

}

void Linker::linkFile(int x){
  string file = inFiles[x];
  int dot = file.find_first_of('.', 0);
  string fileName = file.substr(0, dot + 1) + "dat";

  ifstream ins;
  ins.open(fileName, ios::binary);

  SymbolTable* symTable = new SymbolTable();

  symTable->readBin(ins);

  size_t s, size_s;
  map<string, Section*> fileSections;
  ins.read((char*)&s, sizeof(s));
  for(int i = 0; i < s; i++){
    string sect;
    ins.read((char*) &size_s, sizeof(size_s));
    sect.resize(size_s);
    ins.read(&sect[0], size_s);
    Section* newSect = new Section(sect);
    newSect->readBin(ins, symTable);
    fileSections.insert({sect, newSect});
  }

  ins.close();

  inSymTabs.push_back(symTable);
  inSections.push_back(fileSections);

}

void Linker::writeHex(){
  
  ofstream outs;
  outs.open(outFile);

  //outs << symbolTable << endl;

  char n = (char) 0;

  int base, size, curBase, curPtr, curPtr2;
  string name;
  vector<char> content;
  for(auto& seg:baseAdds){
    curPtr = 0;
    base = seg.getBase();
    size = seg.getSize();
    name = seg.getName();
    content = sections.find(name)->second->getVector();

    outs << hex << setw(4) << setfill('0') << base << ":\t";
    int rpt = base+size;
    int diff = base;
    for(int i = base; i < rpt; i++){
      outs << hex << setw(2) << setfill('0') << (0xFF & content[i-diff]);
      if( (i%8)!=7 ) outs << " ";
      else {
        base+=(8-base%8);
        outs << endl;
        outs << hex << setw(4) << setfill('0') << base << ":\t";
      }
      if(i==rpt-1) outs << endl;
    }

  }

  outs.flush();

  outs.close();

}

void Linker::writeBinHex(){
  int dot = outFile.find_first_of('.', 0);

  string outBinFile = outFile.substr(0, dot+1) + "dat2";
  ofstream outsb(outBinFile, ios::binary);

  size_t size_b = baseAdds.size();
  outsb.write((char*) &size_b, sizeof(size_b));
  for(int i = 0; i < size_b; i++){
    string name = baseAdds[i].getName();
    int base = baseAdds[i].getBase();
    int size = baseAdds[i].getSize();
    
    size_t size_s = name.size();
    outsb.write( (char*)&size_s, sizeof(size_s) );
    outsb.write(&name[0], size_s);
    outsb.write((char*)&base, sizeof(int));
    outsb.write((char*)&size, sizeof(int));
  }

  size_t size_sect = sections.size();
  outsb.write( (char*)&size_sect, sizeof(size_sect) );

  for(auto& curSect:sections){
    curSect.second->writeBinCont(outsb);
  }

  outsb.close();
}

void Linker::processEntry(int x){
  vector<Symbol*> sectsToIns;
  vector<Symbol*> symsToIns;
  vector<Symbol*> undefs;
  
  string symName;
  Symbol* curSym;
  for(int i = 0; i < inSymTabs[x]->symbols.size(); i++){
    curSym = inSymTabs[x]->getSymbolByNum(i);
    if(curSym == nullptr) continue;
    symName = curSym->name;/*
  }
  for(auto& sym:inSymTabs[x]->symbols){
    symName = sym.first;
    curSym = sym.second;*/
    if(curSym->getScope()==Scope::LOCAL) continue;
    if(curSym->getType()==SymbolType::SCTN) {
      sectsToIns.push_back(curSym);
      continue;
    }
    if(!curSym->getSection().compare("UND")) undefs.push_back(curSym);
    else symsToIns.push_back(curSym);
  }

  // obrada sekcija
  for(auto curSect:sectsToIns){
    string sectName = curSect->name;
    auto it = sections.find(sectName);
    if(it == sections.end()){
      Section* mySect = inSections[x].find(sectName)->second;
      sections.insert({sectName, mySect});
      symbolTable->addSymbol(sectName, curSect);
      orderedSections.push_back(sectName);
    }else{
      Section* mySect = inSections[x].find(sectName)->second;
      Section* sectToUpd = it->second;
      int curSize = sectToUpd->size();
      for(int j = 0; j < symsToIns.size(); j++){
        if(!symsToIns[j]->getSection().compare(sectName)){
          symsToIns[j]->updateOffs(curSize);
        }
      }

      sectToUpd->append(mySect->getVector());
      mySect->getRelTable()->updRelocs(curSize);
      sectToUpd->getRelTable()->append(mySect->getRelTable()->getVector());
      symbolTable->getSymbol(sectName)->setExpSize(sectToUpd->size());

    };
  }

  for(auto curSymb:symsToIns){
    Symbol* inSym = symbolTable->getSymbol(curSymb->name);
    if(inSym == nullptr){
      symbolTable->addSymbol(curSymb->name, curSymb);
    }else{
      if(!inSym->getSection().compare("UND")){
        inSym->setProperties(curSymb->getSection(), curSymb->getOffset());
      }else{
        throw runtime_error("Symbol " + curSymb->name + " already defined");
      }
    }
  }

  for(auto curSymb:undefs){
    Symbol* inSym = symbolTable->getSymbol(curSymb->name);
    if(inSym == nullptr) symbolTable->addSymbol(curSymb->name, curSymb);
  }

}

bool Linker::isExecutable(){
  return symbolTable->allDef();
}

void Linker::writeBinObj(){
  int dot = outFile.find_first_of('.', 0);

	string outb = outFile.substr(0, dot + 1) + "dat";

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

void Linker::writeObj(){

	ofstream outs;
	outs.open(outFile);

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

void Linker::setBaseAdds(){
  int s = baseAdds.size();
  int segSize = 0;
  for(int i = 0; i < s; i++){
    string name = baseAdds[i].getName();
    if(symbolTable->getSymbol(name)==nullptr) 
      throw runtime_error("Section " + name + " does not exist");
    segSize = symbolTable->getSymbol(name)->size;
    if( (i < s-1) && (baseAdds[i].getBase()+segSize > baseAdds[i+1].getBase()))
      throw runtime_error("Invalid section " + name + " base address");
    baseAdds[i].setSize(segSize);
  }

  setBaseAddsRest();

}

void Linker::setBaseAddsRest(){
  for(int i = 0; i < orderedSections.size(); i++){

    string name = orderedSections[i];
    if(isBaseAdded(name)) continue;
    Symbol* curSym = symbolTable->getSymbol(name);
    int sectSize = curSym->size;
    int curBase = 0;
    
    int ind = 0;
    bool bigest = true;
    for(int i = 0; i < baseAdds.size(); i++){
      if(curBase + sectSize <= baseAdds[i].getBase()){
        ind = i;
        bigest = false;
        break;
      }else{
        curBase = baseAdds[i].getBase() + baseAdds[i].getSize();
      }
    }
    SegmentEntry seg(name, curBase, sectSize);
    if(bigest) baseAdds.push_back(seg);
    else baseAdds.insert(baseAdds.begin()+ind, seg);

  }
}

bool Linker::isBaseAdded(string name){
  for(auto& bs:baseAdds) if(!bs.getName().compare(name)) return true;
  return false;
}

void Linker::updateSymbolTable(){
  for(auto it:symbolTable->symbols){
    Symbol* curSym = it.second;
    int base = findOffset(curSym->getSection());
    curSym->updateOffs(base);
  }
}

void Linker::doRelocations(){
  for(auto it:sections){
    //cout << it.second->getRelTable() << endl;
    int base = findOffset( it.second->getName() );
    //it.second->doRelocations(base);
    it.second->doRelocations(base, symbolTable);
  }
}

int Linker::findOffset(string sect){
  for(auto seg:baseAdds){
    if(!seg.getName().compare(sect)) return seg.getBase();
  }
  return 0;
}

bool addSectAddr(string arg, vector<SegmentEntry> &sectAdd){
  regex place{"--place=[\\.]?[a-zA-Z][a-zA-Z0-9_]*@0x[0-9A-Fa-f]*"};
  if(!regex_match(arg, place)) return false;
  int indEq = 0, indAt = 0;
  for(int i = 0; i < arg.size(); i++){
    if(arg[i]=='=') indEq = i;
    else if(arg[i]=='@') {
      indAt = i;
      break;
    }
  }
  string sectName = arg.substr(indEq+1, indAt-indEq-1);
  string sectAddr = arg.substr(indAt+1, arg.size()-indAt);
  unsigned int sectAddrInt = stoul(sectAddr, nullptr, 16);

  int ind = 0;
  bool bigest = true;
  for(int i = 0; i < sectAdd.size(); i++){
    if(sectAddrInt > sectAdd[i].getBase()) continue;
    ind = i;
    bigest = false;
    break;
  }
  SegmentEntry seg(sectName, sectAddrInt);
  if(sectAdd.size() == 0 || bigest) sectAdd.push_back(seg);
  else sectAdd.insert(sectAdd.begin() + ind, seg);
  
  return true;
}

int main(int argc, char* argv[]){

  if(argc<5){
    cout << "Not enough arguments";
    return 1;
  }
  int type;
  string s = argv[1], out;
  vector<string> strs;
  if(!s.compare("-hex")) type = 0;
  else if(!s.compare("-relocatable")) type = 1;
  else{
    cout << "Second argument should be -hex or -relocatable" << endl;
    return 3;
  }

  int ind = 0;
  for(int i = ind; i < argc; i++){
    string cmp = argv[i];
    if(!cmp.compare("-o")){
      ind = i;
      break;
    }
  }
  if(ind == 0 || ind == 1 || ind == argc || ind == argc-1 ){
    cout << "Missing or invalid -o argument" << endl;
    return 2;
  }

  vector<SegmentEntry> segMap;
  for(int i = 2; i < ind; i++){
    if(!addSectAddr(argv[i], segMap)) {
      cout << "Invalid " << argv[i] << " argument" << endl;
      return 4;
    }
  }

  out = argv[ind+1];
  
  for(int i = ind+2; i < argc; i++){
    strs.push_back(argv[i]);
  }
  
  Linker* myLinker = new Linker(strs, out, type, segMap);
  myLinker->link();

  return 0;
}


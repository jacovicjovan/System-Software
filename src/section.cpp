#include "section.h"
#include "enums.h"
#include <iomanip>
#include <fstream>

Section::Section(string namee){
  name = namee;
  relTable = new RelocationTable(namee);
}

RelocationTable* Section::getRelTable(){
  return relTable;
}

void Section::push(Relocation rel){
  relTable->push(rel);
}

void Section::addByte(char ch){
  sec.push_back(ch);
}

void Section::addByte(int lc, int x){
  int x1 = x&0xFF;
  sec[lc] = x1;
}

void Section::addByte(int x){
  int x1 = x&0xFF;
  sec.push_back(x1);
}

void Section::addZerros(int x){
  for(int i = 0; i < x; i++){
    //cout << "Ubacujemo nule u sec " << sec.size() << endl;
    sec.push_back(0);
  }
}

void Section::addString(string s){
  for(int i = 0; i < s.size(); i++){
    sec.push_back(s[i]);
  }
}

int Section::size(){
  return sec.size();
}

void Section::append(vector <char> app){
  for(int i = 0; i < app.size(); i++){
    sec.push_back(app[i]);
  }
}

vector<char> Section::getVector(){
  return sec;
}

void Section::addLit(int x, int r){
  int x1 = x&0xFF;
  int x2 = (x>>8)&0xFF;
  
  if(r == 0){
    sec.push_back(x1);
    sec.push_back(x2);
  }else{
    sec.push_back(x2);
    sec.push_back(x1);
  }
}

string Section::getName(){
  return name;
}

void Section::addHexLit(string s, int r){
  unsigned int x = stoul(s, nullptr, 16);
  addLit(x, r);
}

void Section::addLiteral(string s, SymLit sl, int r){
  if(sl == SymLit::LIT) addLit(stoi(s), r);
  else if(sl == SymLit::LITHEX) addHexLit(s, r);
}

ostream& operator<<(ostream& os, const Section* s){
  os << "#" << s->name << endl;
  //os << s->sec.size();

  for(int i = 0; i<s->sec.size(); i++){
    if(i>0 && i%8==0) os << endl;
    os << hex << setfill('0') << setw(2) << (s->sec[i] & 0xFF) << " ";
  }

  return os;
}

void Section::doRelocations(int sectOffset, SymbolTable* symTable){
  for(int i = 0; i < relTable->relTable.size(); i++){
    Relocation r = relTable->relTable[i];
    Symbol* curSym = symTable->getSymbol(r.symbol->name);
    if(r.type == RelType::R_386_16) {
      int val = curSym->offset + r.addend;
      addLitToPos(val, r.offset);
      // cout << "Relokacija R386 16 sekcija " << name << sectOffset << endl;
      // cout << r.symbol->name << " sa vrednoscu " << curSym->offset << " na ofsetu " << r.offset << endl;
    }else if(r.type == RelType::R_386_PC16){
      int val = curSym->offset + r.addend - (r.offset + sectOffset);
      addLitToPos(val, r.offset);
      // cout << "PC Relokacija sekcija " << name << sectOffset << endl;
      // cout << val << " na ofsetu " << r.offset << endl;
    }

  }

}

void Section::doRelocations(int sectOffset) {
  for(int i = 0; i < relTable->relTable.size(); i++){
    Relocation r = relTable->relTable[i];
    if(r.type == RelType::R_386_16) {
      int val = r.symbol->offset + r.addend;
      addLitToPos(val, r.offset);
      cout << "Relokacija R386 16 sekcija " << name << sectOffset << endl;
      cout << r.symbol->name << " sa vrednoscu " << r.symbol->offset << " na ofsetu " << r.offset << endl;
    }else if(r.type == RelType::R_386_PC16){
      int val = r.symbol->offset + r.addend - (r.offset + sectOffset);
      addLitToPos(val, r.offset);
      cout << "PC Relokacija sekcija " << name << sectOffset << endl;
      cout << val << " na ofsetu " << r.offset << endl;
    }
  }
}

void Section::doRelocations(){
  for(int i = 0; i < relTable->relTable.size(); i++){
    Relocation r = relTable->relTable[i];
    if(r.type == RelType::R_386_16) {
      int val = r.symbol->offset + r.addend;
      addLitToPos(val, r.offset);
    }else if(r.type == RelType::R_386_PC16){
      int val = r.symbol->offset + r.addend - r.offset;
      addLitToPos(val, r.offset);
    }
  }
}

void Section::addLitToPos(int lit, int pos){
  int x1 = lit&0xFF;
  int x2 = (lit>>8)&0xFF;

  sec[pos] = x1;
  sec[pos+1] = x2;
}

void Section::writeBin(ofstream& ofs){

  size_t size = sec.size();
  ofs.write((char*) &size, sizeof(size));

  for(int i = 0; i < size; i++){
    ofs.write(&sec[i], sizeof(char));
  }
  relTable->writeBin(ofs);


}

void Section::writeBinCont(ofstream& ofs){

  size_t size_s = name.size();
  ofs.write( (char*)&size_s, sizeof(size_s) );
  ofs.write(&name[0], size_s);

  size_t size = sec.size();
  ofs.write((char*) &size, sizeof(size));

  for(int i = 0; i < size; i++){
    ofs.write(&sec[i], sizeof(char));
  }
}

void Section::readBinCont(ifstream& ifs){
  size_t size_cont;
  ifs.read( (char*)&size_cont, sizeof(size_cont) );
  char c[size_cont];
  for(int i = 0; i < size_cont; i++){
    ifs.read(&c[i], sizeof(char));
    sec.push_back(c[i]);
  }
}

void Section::readBin(ifstream& ifs, SymbolTable* table){

  size_t size;
  ifs.read((char*) &size, sizeof(size));
  char c[size];
  for(int i = 0; i < size; i++){
    ifs.read(&c[i], sizeof(char));
    sec.push_back(c[i]);
  }
  relTable->readBin(ifs, table);

}


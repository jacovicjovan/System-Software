#include "symbolTable.h"
#include <iostream>
#include <fstream>

SymbolTable::SymbolTable() {}

void SymbolTable::addSymbol(string name, Symbol* sym){
  symbols.insert({name, sym});
}

Symbol* SymbolTable::getSymbol(string name){
  auto it = symbols.find(name);
  if(it==symbols.end()) return nullptr;
  return it->second;
}

Symbol* SymbolTable::getSymbolByNum(int num){
  for(auto &it:symbols){
    if(it.second->num == num) return it.second;
  } return nullptr;
}

bool SymbolTable::allDef(){
  for(auto& sym:symbols){
    if(!sym.second->getSection().compare("UND")) return false;
  }
  return true;
}

bool SymbolTable::isDefined(string sym){
  //cout << "Funkcija isDefined " << sym << endl;
  auto it = this->symbols.find(sym);
  //cout << "Dohvacen iterator" << endl;
  if(it->second->isDefined()) return true;
  return false;
}

bool SymbolTable::isThere(string sym){
  //cout << "Funkcija isThere" << endl;
  if(this->symbols.find(sym) == this->symbols.end()){
    //cout << "Vracamo false" << endl;
    return false;
  }
  //cout << "Vracamo true" << endl;
  return true;
}

ostream& operator<<(ostream& os, const SymbolTable* st){
  os << "#.symtab" << endl;
  os << "Num" << "\t" << "Value" << "\t" << "Size" << "\t";
  os << "Type" << "\t" << "Bind" << "\t" << "Ndx" << "\t";
  os << "Name" << endl;

  for(auto const& sym:st->symbols){
    os << sym.second << endl;
  }

  return os;
}

bool SymbolTable::checkSymbols(){
  for(auto &sym:symbols){
    if( !sym.second->isDefined() ) return false;
  }
  return true;
}

void SymbolTable::writeBin(ofstream& ofs){
  size_t size = symbols.size();
  ofs.write((char*) &size, sizeof(size));

  for(auto &sym:symbols){
    ofs.write((char*) &sym.second->num, sizeof(int));
    ofs.write((char*) &sym.second->offset, sizeof(int));
    ofs.write((char*) &sym.second->size, sizeof(int));
    ofs.write((char*) &sym.second->symbolType, sizeof(SymbolType));
    ofs.write((char*) &sym.second->scope, sizeof(Scope));
    size = sym.second->section.size();
    ofs.write((char*) &size, sizeof(size));
    ofs.write(&sym.second->section[0], size);
    size = sym.second->name.size();
    ofs.write((char*) &size, sizeof(size));
    ofs.write(&sym.second->name[0], size);
  }
}


void SymbolTable::readBin(ifstream& ifs){
  size_t size, size_s;
  ifs.read((char*) &size, sizeof(size));
  int num, offset, mySize;
  Scope scope;
  SymbolType symbolType;
  string section, name;
  for(int i = 0; i < size; i++){
    ifs.read((char*) &num, sizeof(int));
    ifs.read((char*) &offset, sizeof(int));
    ifs.read((char*) &mySize, sizeof(int));
    ifs.read((char*) &symbolType, sizeof(SymbolType));
    ifs.read((char*) &scope, sizeof(Scope));
    ifs.read((char*) &size_s, sizeof(size_s));
    section.resize(size_s);
    ifs.read(&section[0], size_s);
    ifs.read((char*) &size_s, sizeof(size_s));
    name.resize(size_s);
    ifs.read(&name[0], size_s);
    symbols.insert({name, new Symbol(name, section, scope, symbolType, offset, mySize, num)});
  }

}

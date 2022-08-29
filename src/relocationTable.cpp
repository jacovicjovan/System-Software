#include "relocationTable.h"
#include <fstream>

RelocationTable::RelocationTable(string namee){
  name = namee;
}

void RelocationTable::push(Relocation rel){
  relTable.push_back(rel);
}

ostream& operator<<(ostream& os, const RelocationTable* rt){
  if(rt->relTable.size() == 0) return os;
  os << "#.rela" << rt->name << endl;
  os << "Offset\tType\tSymbol\tAddend" << endl;

  for(int i = 0; i < rt->relTable.size(); i++){
    os << rt->relTable[i] << endl;
  }

  return os;
}

void RelocationTable::writeBin(ofstream& os){
  size_t size = relTable.size();

  os.write((char*) &size, sizeof(size));
  for(int i = 0; i < size; i++){
    relTable[i].writeBin(os);
  }
}

void RelocationTable::readBin(ifstream& is, SymbolTable* table){
  size_t size, size_s;
  is.read((char*) &size, sizeof(size));
  int offset, addend;
  RelType relType;
  string symbol;
  for(int i = 0; i < size; i++){
    is.read((char*) &size_s, sizeof(size_s));
    symbol.resize(size_s);
    is.read(&symbol[0], size_s);
    is.read((char*) &offset, sizeof(int));
    is.read((char*) &addend, sizeof(int));
    is.read((char*) &relType, sizeof(RelType));
    Symbol* sym = table->getSymbol(symbol);
    Relocation rel(offset, relType, sym, addend);
    relTable.push_back(rel);
  }
}

void RelocationTable::updRelocs(int s){
  for(auto& rel:relTable){
    rel.updateRel(s);
  }
}

vector<Relocation> RelocationTable::getVector(){
  return relTable;
}

void RelocationTable::append(vector <Relocation> relocs){
  for(int i = 0; i < relocs.size(); i++){
    relTable.push_back(relocs[i]);
  }
}


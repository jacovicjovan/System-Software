#include "relocation.h"
#include <iomanip>
#include <fstream>

Relocation::Relocation(int offsett, RelType typee, Symbol* sym, signed int add){
  offset = offsett;
  type = typee;
  symbol = sym;
  addend = add;
}

Relocation::Relocation(int offsett, int add, RelType typee, string symName, SymbolTable* table){
  offset = offsett;
  type = typee;
  addend = add;
  symbol = table->getSymbol(symName);
}

void Relocation::updateRel(int s){
  offset += s;
}

void Relocation::writeBin(ofstream& os){
  string symName = symbol->name;
  size_t size = symName.size();
  os.write((char*) &size, sizeof(size));
  os.write(&symName[0], size);
  os.write((char*) &offset, sizeof(int));
  os.write((char*) &addend, sizeof(int));
  os.write((char*) &type, sizeof(RelType));
}


ostream& operator<<(ostream& os, const Relocation rl){
  os << hex << setfill('0') << setw(4) << rl.offset << '\t';
  if(rl.type == RelType::R_386_16) os << "R_386_16";
  else if(rl.type == RelType::R_386_PC16) os << "R_386_PC16";
  else os << "R_386_16I";
  os << "\t" << rl.symbol->name;
  os << "\t\t" << dec << rl.addend; 
  return os;
}

TempRelocation::TempRelocation(string namee, string sectionn, int lc, int add, RelType rel){
  symbol = namee;
  section = sectionn;
  locCounter = lc;
  addend = add;
  relType = rel;
}



#ifndef RELOCATION_H
#define RELOCATION_H

#include "enums.h"
#include "symbol.h"
#include "symbolTable.h"
#include <string>

using namespace std;

class TempRelocation{
  public:
  TempRelocation(string, string, int, int, RelType);
  string symbol;
  string section;
  int locCounter;
  int addend;
  RelType relType;
};


class Relocation{
public:
  Relocation(int, RelType, Symbol*, int);
  Relocation(int, int, RelType, string, SymbolTable*);

  void updateRel(int);

  void writeBin(ofstream&);
  void readBin(ifstream&);

  friend class Section;
  friend ostream& operator<<(ostream&, const Relocation r);
private:
  int offset;
  RelType type;
  Symbol* symbol;
  signed int addend;
};

#endif

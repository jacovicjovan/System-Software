#ifndef RELTABLE_H
#define RELTABLE_H

#include "relocation.h"
#include "symbolTable.h"
#include <vector>

class RelocationTable{
public:
  RelocationTable(string namee);

  void push(Relocation rel);

  void updRelocs(int);

  vector<Relocation> getVector();
  void append(vector<Relocation>);

  friend ostream& operator<<(ostream&, const RelocationTable*);

  void writeBin(ofstream&);
  void readBin(ifstream&, SymbolTable*);

  friend class Section;

private:
  string name;
  vector<Relocation> relTable;
};

#endif
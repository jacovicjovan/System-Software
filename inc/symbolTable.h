#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "symbol.h"
#include <map>
#include <string>

class SymbolTable {
public:
  
  SymbolTable();

  void addSymbol(string name, Symbol* sym);
  Symbol findSymbol(string name);
  bool isDefined(string sym);
  bool isThere(string sym);

  bool checkSymbols();
  bool allDef();

  Symbol* getSymbol(string name);

  Symbol* getSymbolByNum(int);

  void writeBin(ofstream&);
  void readBin(ifstream&);

  friend ostream& operator<<(ostream&, const SymbolTable* st);

  map<string, Symbol*> symbols;

};

#endif
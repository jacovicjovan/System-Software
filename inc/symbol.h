#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>
#include <iostream>
#include "enums.h"

using namespace std;

class Symbol {
public:
  Symbol(string namee, string sectionn, Scope scopee, SymbolType typee, int offsett);
  Symbol(string namee, string sectionn, Scope scopee, SymbolType typee, int offsett, int, int);
  Symbol(string namee, Scope scopee, SymbolType typee);

  void define();
  bool isDefined();
  void setProperties(string sectionn, int offsett);
  void setSize(int s);

  Scope getScope();
  int getOffset();
  string getSection();
  SymbolType getType();

  void updateOffs(int);
  void setExpSize(int);

  static int id;

  friend ostream& operator<<(ostream&, const Symbol* s);

  string name;
  string section;
  Scope scope;
  SymbolType symbolType;

  int num;
  int offset;
  int size;

  bool defined;
};


#endif
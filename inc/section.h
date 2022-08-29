#ifndef SECTION_H
#define SECTION_H

#include <vector>
#include <string>
#include <iostream>
#include "enums.h"
#include "relocationTable.h"

using namespace std;

class Section{
public:
  Section(string);
  void addByte(char);
  void addByte(int);
  void addString(string);
  void addZerros(int x);
  void addLit(int x, int r = 0);
  void addHexLit(string x, int r = 0);
  void addLiteral(string s, SymLit sl, int r = 0);

  void addByte(int, int);

  void addLitToPos(int, int);

  void doRelocations();
  void doRelocations(int);
  void doRelocations(int, SymbolTable*);

  RelocationTable* getRelTable();

  void append(vector<char>);
  vector<char> getVector();

  void push(Relocation);

  int size();

  string getName();

  void writeBin(ofstream&);
  void readBin(ifstream&, SymbolTable*);

  void writeBinCont(ofstream&);
  void readBinCont(ifstream&);

  friend ostream& operator<<(ostream&, const Section* st);

private:
  string name;
  vector<char> sec;
  RelocationTable* relTable;
};

#endif
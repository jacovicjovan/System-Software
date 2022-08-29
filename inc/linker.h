#ifndef LINKER_H
#define LINKER_H

#include <vector>
#include <string>
#include <map>
#include "symbolTable.h"
#include "section.h"
#include "segentry.h"

using namespace std;

class Linker{
public:
  Linker(vector<string>, string, int);
  Linker(vector<string>, string, int, vector<SegmentEntry>);

  void link();
  void linkFile(int);
  //void write();
  void processEntry(int);

  void setBaseAdds();
  void setBaseAddsRest();

  bool isExecutable();
  bool isBaseAdded(string);

  void updateSymbolTable();
  int findOffset(string);

  void doRelocations();

  void writeBinObj();
  void writeObj();

  void writeHex();
  void writeBinHex();

private:
  vector<string> inFiles;
  string outFile;
  int type;

  vector<string> orderedSections;

  vector<SymbolTable*> inSymTabs;
  vector<map<string, Section*>> inSections;

  vector<SegmentEntry> baseAdds;

  SymbolTable* symbolTable;
  map<string, Section*> sections;
};

#endif
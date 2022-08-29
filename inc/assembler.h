#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <vector>
#include <queue>
#include <map>
#include "relocationTable.h"
#include "section.h"

class SymbolTable;

using namespace std;

class Assembler {
public:
  Assembler(string i, string o);

  void assembly();

  void pass();

private:
  void processLabel(string labelName);
  void processGlobal(queue<string>& tokenStream);
  void processExtern(queue<string>& tokenStream);
  void processSection(string secName, queue<string>& tokenStream);
  void processSkip(int x);
  void processEnd();
  void processDirective(string dir, queue<string>& tokenStream);
  void processInstruction(string, queue<string>&);
  void addRel(string, RelType, int, int);

  bool checkSymbols();
  void write();
  void doRelocations();

  void writeBin();

  string in;
  string out;

  vector< vector<string> > assemblyText;
  SymbolTable* symbolTable;
  string curSection;
  Section* curSectionPointer;
  int locationCounter;

  static map<string, int> InstructionCode;
  static map<string, int> RegCode;

  vector<TempRelocation> tempRelocations;
  map<string, Section*> sections;
};

#endif
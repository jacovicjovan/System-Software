#ifndef EMULATOR_H
#define EMULATOR_H

#include <vector>
#include <string>
#include <map>
#include "section.h"
#include "segentry.h"
using namespace std;

#define SP 6
#define PC 7
#define PSW 8

class Emulator{
public:
  Emulator(string);

  void emulate();

  void readBin();
  void initMem();
  void writeOut();

  void readRegs();
  short readAddr();
  void readOperand(int);

  short readWord(uint16_t);
  void writeWord(uint16_t, short);
  short readByte(uint16_t);
  void readInstruction(short);
  void processRets(short);

  void processTwoBytesIns(short);
  void processThreeBytesIns(short, int); // str ??
  void processLogics(short);
  void processAryts(short);
  void processOtherTwoBytes(short);

  void store(int);
  void load(int);
  void jumps(short, int);

  void push(int);
  void pop(int);

  void setN();
  void resetN();
  void setC();
  void resetC();
  void setO();
  void resetO();
  void setZ();
  void resetZ();

private:
  bool emulating = false;
  short r[9];
  short regS, regD, operand;

  char* mem;

  string in;
  vector<SegmentEntry> baseAdds;
  map<string, Section*> sections;
};

#endif
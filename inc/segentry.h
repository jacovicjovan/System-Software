#ifndef SEGENTRY_H
#define SEGENTRY_H

#include <string>
using namespace std;

class SegmentEntry{
public:
  SegmentEntry(string, int b = 0, int s = 0);
  int getSize();
  int getBase();
  string getName();

  void setSize(int b);
  void setBase(int s);
private:
  string name;
  int base;
  int size;
};

#endif
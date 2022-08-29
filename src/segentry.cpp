#include "segentry.h"

SegmentEntry::SegmentEntry(string nam, int  b, int s){
  name = nam;
  base = b;
  size = s;
}

int SegmentEntry::getBase(){ return base; }

int SegmentEntry::getSize(){ return size; }

string SegmentEntry::getName(){ return name; }

void SegmentEntry::setBase(int b) { base = b; }

void SegmentEntry::setSize(int s) { size = s; }


#include "symbol.h"

int Symbol::id = 0;

Symbol::Symbol(string namee, string sectionn, Scope scopee, SymbolType typee, int offsett, int sizee, int numm){
  this->name = namee;
  this->section = sectionn;
  this->scope = scopee;
  this->offset = offsett;
  this->num = numm;
  this->symbolType = typee;
  this->size = sizee;
  this->defined = true;
}

Symbol::Symbol(string namee, string sectionn, Scope scopee, SymbolType typee, int offsett){
  this->name = namee;
  this->section = sectionn;
  this->scope = scopee;
  this->offset = offsett;
  this->num = id++;
  this->symbolType = typee;
  this->size = 0;
  this->defined = true;
}

Symbol::Symbol(string namee, Scope scopee, SymbolType typee){
  this->name = namee;
  this->scope = scopee;
  this->symbolType = typee;
  this->size = 0;
  this->num = id++;
  this->defined = false;
}

void Symbol::setProperties(string sectionn, int offsett){
  this->section = sectionn;
  this->offset = offsett;
  this->defined = true;
}

void Symbol::setSize(int s){
  this->size += s;
}

void Symbol::define(){
  defined = true;
}

bool Symbol::isDefined(){
  return this->defined;
}

Scope Symbol::getScope(){
  return scope;
}

int Symbol::getOffset(){
  return offset;
}

string Symbol::getSection(){
  return section;
}

SymbolType Symbol::getType(){
  return symbolType;
}

void Symbol::updateOffs(int s){
  this->offset += s;
}

void Symbol::setExpSize(int s){
  this->size = s;
}

ostream& operator<<(ostream &os, const Symbol* s){
  os << s->num << "\t";
  os << hex << s->offset << "\t" << s->size << "\t";
  if(s->symbolType == SymbolType::NOTYP) os << "NOTYP" << "\t";
  else os << "SCTN" << "\t";
  if(s->scope == Scope::GLOBAL) os << "GLOB" << "\t";
  else os << "LOC" << "\t";
  os << s->section << "\t" << s->name;
  
  return os;
}


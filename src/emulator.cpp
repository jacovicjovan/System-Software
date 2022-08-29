#include "emulator.h"
#include <iostream>
#include <fstream>
#include <iomanip>

Emulator::Emulator(string ins){
  in = ins;
  readBin();
  mem = new char[0x10000];
  initMem();
}

void Emulator::initMem(){
  for(auto& seg:baseAdds){
    string curSectName = seg.getName();
    vector<char> curVect = sections.find(curSectName)->second->getVector();
    int base = seg.getBase();
    int size_vec = seg.getSize();
    for(int i = 0; i < size_vec; i++){
      mem[base + i] = curVect[i];
    }
  }
  r[PC] = readWord(0);
  r[SP] = 0xFF00;
  r[PSW] = 0;
}

void Emulator::emulate(){
  emulating = true;
  while(emulating){
    try{
      readInstruction(r[PC]);
    }catch(runtime_error re){
      cout << re.what() << endl;
      break;
    }
  }

}

void Emulator::readInstruction(short pos){
  short instCode = readByte(pos);
  switch(instCode){
    case 0:
      // halt
      r[7] += 1;
      writeOut();
      emulating = false;
      break;
    case 32:
    case 64:
      // iret, ret
      r[7] += 1;
      processRets(instCode);
      break;
    case 16:
    case 96:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 144:
    case 145:
    case 192:
    case 193:
      // int xchg push pop arith logics test pomer
      readRegs();
      r[PC] += 2;
      processTwoBytesIns(instCode);
      break;
    default:
      // call jmps ldr str
      readRegs();
      int addr = readAddr();
      readOperand(addr);
      r[PC] += 3;
      if(!(addr == 1 || addr == 2)) r[PC] += 2;
      processThreeBytesIns(instCode, addr);
      break;
  }
}

void Emulator::readRegs(){
  short registers = readByte(r[PC]+1);
  regS = registers & 0x000F;
  regD = (registers>>4)&0x000F;
}

short Emulator::readAddr(){
  return readByte( r[PC]+2 );
}

void Emulator::readOperand(int add){
  if(add!=1 && add!=2) operand = readWord(r[PC] + 3);
}

void Emulator::processTwoBytesIns(short instCode){
  switch(instCode){
    case 128: // not
    case 129: // and
    case 130: // or
    case 131: // xor
    case 132: // test
      processLogics(instCode);
      break;
    case 112: // add
    case 113: // sub
    case 114: // mul
    case 115: // div
    case 116: // cmp
      processAryts(instCode);
      break;
    default:
      processOtherTwoBytes(instCode);
      break;
  }
}

void Emulator::processThreeBytesIns(short instCode, int addr){
  switch (instCode)
  {
  case 80: // jmp
  case 81: // jeq
  case 82: // jne
  case 83: // jgt
  case 48: // call
    jumps(instCode, addr);
    break;
  case 160: // ldr
    load(addr);
    break;
  case 176: // str
    store(addr);
    break;
  default:
    break;
  }
}

void Emulator::jumps(short instCode, int addr){
  short val;
  if(addr == 0) val = operand;
  else if(addr == 1) val = r[regS];
  else if(addr == 2) val = readWord(r[regS]);
  else if(addr == 3) val = readWord(r[regS] + operand);
  else if(addr == 4) val = readWord(operand);
  else if(addr == 5) val = r[regS] + operand;
  if(instCode == 81 && !r[PSW]&0x0001) return; // jeq
  else if(instCode == 82 && r[PSW]&0x0001) return; // jne
  else if(instCode == 83 && r[PSW]&0x0008) return; // jgt
  else if(instCode == 48) push(PC); // call
  
  r[PC] = val;
}

void Emulator::load(int addr){
  if(addr == 0) r[regD] = operand;
  else if(addr == 1) r[regD] = r[regS];
  else if(addr == 2) r[regD] = readWord(r[regS]);
  else if(addr == 3) r[regD] = readWord(r[regS] + operand);
  else if(addr == 4) r[regD] = readWord(operand);
  else if(addr == 5) r[regD] = r[regS] + operand;
}

void Emulator::store(int addr){
  if(addr == 0) throw runtime_error("STR instruction and immediate addressing not allowed");
  else if(addr == 1) r[regS] = r[regD];
  else if(addr == 2) writeWord(r[regS], r[regD]);
  else if(addr == 3) writeWord(r[regS] + operand, r[regD]);
  else if(addr == 4) writeWord(operand, r[regD]);
  else if(addr == 5) throw runtime_error("STR ins and reg dir pom addressing not allowed");
}

void Emulator::processLogics(short instCode){
  switch(instCode){
    case 128:
      r[regD] = ~r[regD];
      break;
    case 129:
      r[regD] = r[regD] & r[regS];
      break;
    case 130:
      r[regD] = r[regD] | r[regS];
      break;
    case 131: 
      r[regD] = r[regD] ^ r[regS];
      break;
    case 132:
      short temp = r[regD] & r[regS];
      if(temp < 0) { setN(); resetZ(); }
      else if(temp == 0) { setZ(); resetN(); } 
      break;
  }
}

void Emulator::processAryts(short instCode){
  switch (instCode){
  case 112:
    r[regD] += r[regS];
    break;
  case 113:
    r[regD] -= r[regS];
    break;
  case 114:
    r[regD] *= r[regS];
    break;
  case 115:
    if(r[regS] == 0) throw runtime_error("Division by zero not allowed");
    r[regD] /= r[regS];
    break;
  case 116:
    short temp = r[regD] - r[regS];
    if(temp < 0) { setN(); resetZ(); }
    else if(temp == 0) { setZ(); resetN(); }
    resetO();
    resetC();
    if( ((r[regD] > 0) && (r[regS] < 0 ) && (temp < 0)) || ((r[regD] < 0) && (r[regS] > 0 ) && (temp > 0)) ) setO();
    uint16_t rd = r[regD], rs = r[regS];
    int tmp = rd - rs;
    if(tmp < 0) setC();
    break;
  }
}

void Emulator::processOtherTwoBytes(short instCode){
  uint16_t rd, rs;
  uint32_t tmp;
  short temp;
  switch (instCode){
  case 16: // int
    //cout << "Pozvan INT " << (r[regD]%8) * 2  << " PC = " << r[PC] << endl;
    push(PC);
    push(PSW);
    r[PC] = readWord( (r[regD]%8) * 2 );
    break;
  case 96: // xchg
    temp = r[regD];
    r[regD] = r[regS];
    r[regS] = temp;
    break;
  case 144: // shl
    r[regD] <<= r[regS];
    if(r[regD] < 0) { setN(); resetZ(); }
    else if(r[regD] == 0) { setZ(); resetN(); }
    resetC();
    rd = r[regD];
    rs = r[regS];
    tmp = rd << rs;
    if(tmp > 0xFFFF) setC();
    break;
  case 145: // shr
    r[regD] >>= r[regS];
    if(r[regD] < 0) { setN(); resetZ(); }
    else if(r[regD] == 0) { setZ(); resetN(); }
    resetC();
    rd = r[regD];
    rs = r[regS];
    tmp = rd >> rs;
    if(tmp > 0xFFFF) setC();
    break;
  case 192: // push
    push(regD);
    break;
  case 193: // pop
    pop(regD);
    break;
  default:
    break;
  }
}

void Emulator::processRets(short instCode){
  if(instCode == 32){ // iret
    //cout << "Pozvan iret" << endl;
    pop(PSW);
  } pop(PC);
  //cout << "Povratak na " << r[PC] << endl;
}

void Emulator::pop(int reg){
  if(reg > 8 || reg < 0) throw runtime_error("Invalid register");
  r[reg] = readWord(r[6]);
  r[6] += 2;
}

void Emulator::push(int reg){
  if(reg > 8 || reg < 0) throw runtime_error("Invalid register");
  r[6] -= 2;
  writeWord(r[6], r[reg]);
}

void Emulator::writeWord(uint16_t pos, short word){
  mem[pos + 1] = (word >> 8);
  mem[pos] = word&0x00FF;
}

void Emulator::setN(){ r[8] |= 0x0008; }
void Emulator::setC(){ r[8] |= 0x0004; }
void Emulator::setO(){ r[8] |= 0x0002; }
void Emulator::setZ(){ r[8] |= 0x0001; }
void Emulator::resetN(){ r[8] &= 0xFFF7; }
void Emulator::resetC(){ r[8] &= 0xFFFB; }
void Emulator::resetO(){ r[8] &= 0xFFFD; }
void Emulator::resetZ(){ r[8] &= 0xFFFE; }

void Emulator::writeOut(){
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state: psw = 0x" << hex << setw(4) << setfill('0') << r[PSW] << endl;
  for(int i = 0; i < 8; i++){
    cout << "r" << i << " = 0x" << hex << setw(4) << setfill('0') << r[i];
    if(i == 3 || i == 7) cout << endl;
    else cout << "\t";
  }
}

void Emulator::readBin(){
  int dot = in.find_first_of('.', 0);
  string file = in.substr(0, dot+1) + "dat2";
  ifstream ifs(file, ios::binary);

  size_t size_b;
  ifs.read( (char*)&size_b, sizeof(size_b) );

  for(int i = 0; i < size_b; i++){
    size_t size_s;
    int size, base;
    string name;
    ifs.read( (char*)&size_s, sizeof(size_s) );
    name.resize(size_s);
    ifs.read( &name[0], size_s );
    ifs.read( (char*)&base, sizeof(int) );
    ifs.read( (char*)&size, sizeof(int) );

    SegmentEntry* segEnt = new SegmentEntry(name, base, size);
    baseAdds.push_back(*segEnt);

  }

  string name;

  size_t num_sect, str_len;
  ifs.read( (char*)&num_sect, sizeof(size_t) );
  for(int i = 0; i < num_sect; i++){
    ifs.read( (char*)&str_len, sizeof(size_t) );
    name.resize(str_len);
    ifs.read( &name[0], str_len );
    Section* newSect = new Section(name);
    newSect->readBinCont(ifs);
    sections.insert({name, newSect});
  }

  ifs.close();
}

short Emulator::readWord(uint16_t pos){
  short lit = ((mem[pos+1]&0xFF)<<8)|(mem[pos]&0xFF);
  return lit;
}

short Emulator::readByte(uint16_t pos){
  return 0x0000 | (mem[pos]&0xFF);
}

int main(int argc, char* argv[]){

  if(argc!=2){
    cout << "Invalid arguments" << endl;
    return 1;
  }

  string in = argv[1];

  Emulator* emulator = new Emulator(in);
  emulator->emulate();

  return 0;
}

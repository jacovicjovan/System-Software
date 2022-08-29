OBJ = relocation.cpp relocationTable.cpp section.cpp segentry.cpp symbol.cpp symbolTable.cpp tokenParser.cpp

AS_OBJ = assembler.cpp main.cpp $(OBJ)
LN_OBJ = linker.cpp $(OBJ)
EM_OBJ = emulator.cpp $(OBJ)

CC = g++

vpath %.cpp ./src
vpath %.h ./inc
vpath %.o tests
vpath %.hex tests
vpath %.dat tests
vpath %.dat2 tests

CPPFLAGS = -std=c++11 -I inc

all : assembler linker emulator

assembler: $(AS_OBJ)
	$(CC) $(CPPFLAGS) $^ -o $@

linker: $(LN_OBJ)
	$(CC) $(CPPFLAGS) $^ -o $@

emulator: $(EM_OBJ)
	$(CC) $(CPPFLAGS) $^ -o $@

clean:
	rm tests/*.o tests/*.hex tests/*.dat tests/*.dat2 assembler linker emulator
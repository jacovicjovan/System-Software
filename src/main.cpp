#include <iostream>
#include "assembler.h"
#include <string>

using namespace std;

int main(int argc, char* argv[]){

	if(argc!=4){
		cout << "Illegal number of arguments" << endl;
		return 0;
	}/*else if(argv[1]!="-o"){
		cout << "Illegal instruction: try assembler -o outputFile inputFile";
		return 0;
	}*/

	string out = argv[2];
	string in = argv[3];

	Assembler* mojAsembler = new Assembler(in, out);

	mojAsembler->assembly();

	return 0;
}

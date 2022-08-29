#ifndef TOKENPARSER_H
#define TOKENPARSER_H

#include <fstream>
#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include "enums.h"

using namespace std;

class TokenParser
{
public:
    TokenParser(ifstream &in);

    void split(string line, const char *delim, vector<string> &tokens);
    void removeWhiteSpacesFromStart(string &line);
    static TokenType parseToken(const string& parse);
    static SymLit parseWord(const string& parse);
    vector<vector<string>>& getAssembly();
    static const char *DELIMETERS;


private:
    vector<vector<string>> inputAssemmbly;
};


#endif

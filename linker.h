#ifndef _LINKER_H_
#define _LINKER_H_

#include "section.h"
#include "symboltable.h"

class Linker {
public:
    static SymbolTable* symbolTable;
    static std::map<string, Section*> sections;
	static bool end;
	static void processFile(std::ifstream &inputFile);
    static void link(std::ofstream &outputFile, bool isHex, std::map<string, int> places);
};

#endif
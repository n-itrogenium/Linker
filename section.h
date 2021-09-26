#ifndef _SECTION_H_
#define _SECTION_H_

#include <string>
#include <vector>
#include <map>
#include "relocation.h"
#include "symboltable.h"

using std::vector;
using std::string;

typedef char byte; 
class Section {
public:
	string name;
	int size;
	vector<int8_t> bytes;
	vector<Relocation*> relocationTable;

	Section(string name);
	void addByte(int8_t newByte);
	int checkByte(int byteIndex);
	bool isInstruction(int byteIndex);
	static void updateRelocationOrdinal(std::map<string, Section*> sections, SymbolTable *symbolTable);
	static void updateOffsets(std::map<string, Section*> sections, SymbolTable *symbolTable);
	static void printHex(std::ofstream& outfile, std::map<string, Section*> sections, int startAddr);
	static void printRelocationTable(std::ofstream& outfile, std::map<string, Section*> sections);
	static void printSections(std::ostream& outfile, std::map<string, Section*> sections);
};

#endif


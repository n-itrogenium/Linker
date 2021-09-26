#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <string>
#include <map>
using std::string;

class Symbol {
public:
	static int staticID;
	int ordinal = ++staticID;
	string name;
	string section;
	int offset;
	int size;
	char scope;
	bool defined;
	bool absolute;
	Symbol(string name, string section, int offset, char scope);
};

class SymbolTable {
public:
	std::map<string, Symbol*> sectionTable;
	std::map<string, Symbol*> table;
	Symbol* find(string name);
	Symbol* find(int ordinal);
	void insert(Symbol* symbol, bool isSection);
	void insert(string name, string section, int locationCounter, char scope, bool isSection);
	void updateOrder();
	bool isDefined();
	//void updateValues();
	void printTable(std::ofstream &outfile);
};

#endif


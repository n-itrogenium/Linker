#include "symboltable.h"
#include <iomanip>
#include <fstream>
#include <iostream>

using std::setw;
using std::setfill;

int Symbol::staticID = 0;

Symbol* SymbolTable::find(string name) {
   std::map<string, Symbol*>::iterator i = table.find(name);
   if (i == table.end()) {
	   i = sectionTable.find(name);
	   if (i == sectionTable.end())
	   		return nullptr;
   }
   return i->second;
}

void SymbolTable::insert(string name, string section, int locationCounter, char scope, bool isSection) {
	if (isSection)
		sectionTable[name] = new Symbol(name, section, locationCounter, scope);
	else 
		table[name] = new Symbol(name, section, locationCounter, scope);
}

void SymbolTable::insert(Symbol *symbol, bool isSection) {
	if (isSection)
		sectionTable[symbol->name] = symbol;
	else 
		table[symbol->name] = symbol;
}

bool SymbolTable::isDefined() {
	std::map<string, Symbol*>::iterator i;
	for (i = table.begin(); i != table.end(); i++)
		if (!i->second->defined)
			return false;
	return true;
}

/*void SymbolTable::updateValues() {
	std::map<string, Symbol*>::iterator i;
    for (i = table.begin(); i != table.end(); i++)
        i->second->offset += sectionTable[i->second->section]->offset;
}*/

void SymbolTable::printTable(std::ofstream &outfile) {
	outfile << "============================SYMBOL TABLE==============================" << std::endl;
	outfile << setw(15) << setfill(' ') << "Label";
	outfile << setw(15) << setfill(' ') << "Section";
	outfile << setw(10) << setfill(' ') << "Value";
	outfile << setw(10) << setfill(' ') << "Size";
	outfile << setw(10) << setfill(' ') << "Scope";
	outfile << setw(10) << setfill(' ') << "Ordinal";
	outfile << std::endl << "----------------------------------------------------------------------" << std::endl;

	std::map<string, Symbol*>::iterator i;
	int ordinal = 1;
	for (i = sectionTable.begin(); i != sectionTable.end(); i++) {
		char scope = (i->second->scope == 'E') ? 'G' : i->second->scope;
		outfile << setw(15) << setfill(' ') << i->second->name;
		outfile << setw(15) << setfill(' ') << i->second->section;
		outfile << setw(10) << setfill(' ') << std::hex << i->second->offset << std::dec;
		outfile << setw(10) << setfill(' ') << std::hex << i->second->size << std::dec;
		outfile << setw(10) << setfill(' ') << scope;
		i->second->ordinal = ordinal++;
		outfile << setw(10) << setfill(' ') << i->second->ordinal;
		outfile << std::endl;
	}
	for (i = table.begin(); i != table.end(); i++) {
		char scope = (i->second->scope == 'E') ? 'G' : i->second->scope;
		outfile << setw(15) << setfill(' ') << i->second->name;
		outfile << setw(15) << setfill(' ') << i->second->section;
		outfile << setw(10) << setfill(' ') << std::hex << i->second->offset << std::dec;
		outfile << setw(20) << setfill(' ') << scope;
		i->second->ordinal = ordinal++;
		outfile << setw(10) << setfill(' ') << i->second->ordinal;
		outfile << std::endl;
	}
	outfile << std::endl << std::endl << std::endl;
}

Symbol::Symbol(string name, string section, int offset, char scope) {
	this->name = name;
	this->offset = offset;
	this->scope = scope;
	this->section = section;
	this->defined = (section != "?");
	this->absolute = (section == "abs");
}

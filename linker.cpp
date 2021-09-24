#include "linker.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

using std::cerr;
using std::cout;
using std::endl;

SymbolTable* Linker::symbolTable = new SymbolTable();
std::map<string, Section*> Linker::sections = { };

void Linker::processFile(std::ifstream &inputFile) {
    string line;

    for (int i = 0; i < 3; i++)
        std::getline(inputFile, line);

    SymbolTable *fileSymbolTable = new SymbolTable();
    
    // SYMBOL TABLE
    while (std::getline(inputFile, line)) {

        if (line.empty())
            break;

        std::istringstream iss(line);
        string label, section, _offset, size, ordinal;
        char scope;
        int offset;

        iss >> label >> section >> _offset;
        bool isSection = (label == section);
        if (isSection) iss >> size;
        iss >> scope >> ordinal;

        std::stringstream ss;
		ss << std::hex << _offset;
		ss >> offset;

        Symbol* newSymbol = new Symbol(label, section, offset, scope);
        newSymbol->ordinal = stoi(ordinal);
        fileSymbolTable->insert(newSymbol, isSection);

        // GLOBAL SYMBOL
        if (scope == 'G') { 
            Symbol* _symbol = symbolTable->find(label);
            if (_symbol != nullptr && _symbol->defined) {
                cerr << "ERROR! Symbol already in use: " << label << endl;
                exit(3);
            } else
                symbolTable->insert(newSymbol, false);
            
            // Update symbol offset
            if (!newSymbol->absolute && newSymbol->defined) {
                Symbol* _section = fileSymbolTable->find(section);
                newSymbol->offset += _section->offset;
            }
        }

        // SECTION
        if (isSection) {
            newSymbol->size = stoi(size);
            Symbol* _symbol = symbolTable->find(label);

            // Update section offset
            if (_symbol != nullptr) {
                newSymbol->offset = _symbol->offset + _symbol->size;
                _symbol->size += newSymbol->size;
                sections[label]->size = _symbol->size;
            } else {
                symbolTable->insert(newSymbol, true);
                Section *section = new Section(label);
                section->size = newSymbol->size;
                sections[label] = section;
            }
        }
    }


    for (int i = 0; i < 3; i++)
        std::getline(inputFile, line);

    // RELOCATION TABLE
    while (std::getline(inputFile, line)) {

        if (line.empty())
            continue;

        std::istringstream iss(line);
        string sectionName;

        iss >> sectionName;

        if (sectionName != "Section:")
            break;

        iss >> sectionName;
        Section *section = sections[sectionName];

        for (int i = 0; i < 2; i++)
            std::getline(inputFile, line);

        while(std::getline(inputFile, line)) {
            if (line.empty())
                break;
            std::istringstream iss(line);
            string offset, relType, value;
            iss >> offset >> relType >> value;
            Relocation *rel = new Relocation();
            rel->offset = stoi(offset);
            rel->type = (relType == "ABS") ? ABS : PC_REL;
            rel->value = stoi(value);
            section->relocationTable.push_back(rel);
        }
    }

    
}

void Linker::link(std::ofstream& outputFile) {
    if (!symbolTable->isDefined()) {
		cerr << "ERROR! Symbols not defined" << endl;
		//exit(3);
	}

    // SLAGANJE SEKCIJA
    int currentLocation = 0;
    std::map<string, Symbol*>::iterator i;
    for (i = symbolTable->sectionTable.begin(); i != symbolTable->sectionTable.end(); i++) {
        if (i == symbolTable->sectionTable.begin())
            i->second->offset = 0;
        i->second->offset += currentLocation;
        currentLocation += i->second->size;
        
        std::map<string, Symbol*>::iterator j;
        for (j = symbolTable->table.begin(); j != symbolTable->table.end(); j++) {
            if (j->second->section == i->second->name)
                j->second->offset += i->second->offset;
        }
    }

    symbolTable->printTable(outputFile);
    Section::printRelocationTable(outputFile, sections);
}
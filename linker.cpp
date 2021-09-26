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
                if (newSymbol->defined) {
                    cerr << "ERROR! Symbol already in use: " << label << endl;
                    exit(3);
                }
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
            rel->symbol = fileSymbolTable->find(stoi(value))->name;
            section->relocationTable.push_back(rel);

            rel->offset += fileSymbolTable->find(sectionName)->offset;
        }
    }
    
    // SECTIONS
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
        std::getline(inputFile, line);

        int byteIndex = fileSymbolTable->find(section->name)->offset;

        while(std::getline(inputFile, line)) {
            if (line.empty())
                break;
            string _byte;
            std::istringstream iss(line);
            while (iss >> _byte) {
                int byte;
                std::stringstream ss;
                ss << std::hex << _byte;
                ss >> byte;
                Relocation* rel = section->checkByte(byteIndex);
                if (rel) {
                    Symbol* symbol = fileSymbolTable->find(rel->value);
                    if (!symbol->defined)
                        symbol = symbolTable->find(symbol->name);

                    bool negative = false;
                    if (section->isInstruction(byteIndex)) {
                        // Negative
                        if (section->isOffset(byteIndex) && byte & 0x90 != 0)
                            negative = true;
                        // Positive
                        else
                            byte = (((byte << 8) + symbol->offset) >> 8) & 0xFF;
                    }
                    else
                        byte = (byte + symbol->offset) & 0xFF;
                    if (!negative)
                        section->addByte(byte);
                    byteIndex++;

                    int byte2;
                    iss >> _byte;
                    std::stringstream ss;
                    ss << std::hex << _byte;
                    ss >> byte2;
                    if (section->isInstruction(byteIndex - 1)) {
                        if (negative) {
                            int8_t operand = (((int8_t) byte & 0xFF00) << 8) | ((int8_t) byte2 & 0xFF);
                            operand += symbol->offset;
                            section->addByte(operand >> 8 & 0xFF);
                            byte = operand & 0xFF;
                        } 
                        else
                            byte = (byte2 + symbol->offset) & 0xFF;
                    }
                    else 
                        byte = (((byte2 << 8) + symbol->offset) >> 8) & 0xFF;
                    
                }
                section->addByte(byte);
                byteIndex++;
            }
        }
    }    
}


void Linker::link(std::ofstream& outputFile, bool isHex, std::map<int, string> sectionLocations) {
    symbolTable->updateOrder();
    Section::updateRelocationOrdinal(sections, symbolTable);

    if (isHex) {
        if (!symbolTable->isDefined()) {
		    cerr << "ERROR! Symbols not defined" << endl;
		    exit(3);
	    }

        // Fixed sections
        std::map<int, string>::iterator k;
        int currentLocation = 0;
        for (k = sectionLocations.begin(); k != sectionLocations.end(); k++) {
            Symbol *section = symbolTable->find(k->second);
            if (!section) {
                cerr << "ERROR! Section not found: " << k->second << endl;
                exit(3);
            }
            if (k->first < currentLocation) {
                cerr << "ERROR! Sections are overlapping" << endl;
                exit(3);
            }
            currentLocation = k->first + section->size;
            section->offset = k->first;
            section->fixed = true;

            std::map<string, Symbol*>::iterator j;
            for (j = symbolTable->table.begin(); j != symbolTable->table.end(); j++) {
                if (j->second->section == section->name)
                    j->second->offset += section->offset;
            }
        }

        // Other sections
        std::map<string, Symbol*>::iterator i;
        for (i = symbolTable->sectionTable.begin(); i != symbolTable->sectionTable.end(); i++) {
            if (!i->second->fixed) {
                i->second->offset += currentLocation;
                currentLocation += i->second->size;
                
                std::map<string, Symbol*>::iterator j;
                for (j = symbolTable->table.begin(); j != symbolTable->table.end(); j++) {
                    if (j->second->section == i->second->name)
                        j->second->offset += i->second->offset;
                }
            }
        }
        std::map<int, Section*> _sections;
        for (i = symbolTable->sectionTable.begin(); i != symbolTable->sectionTable.end(); i++)
            _sections[i->second->offset] = sections.find(i->second->name)->second;
        
        int startAddr = (sectionLocations.empty()) ? 0 : sectionLocations.begin()->first;
        Section::updateOffsets(sections, symbolTable);
        Section::printHex(outputFile, _sections, startAddr, symbolTable);
    }
    else {
        symbolTable->printTable(outputFile);
        Section::printRelocationTable(outputFile, sections);
        Section::printSections(outputFile, sections); 
    }
}
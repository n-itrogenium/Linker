#include "section.h"
#include <fstream>
#include <iomanip>
#include <iostream>

using std::setw;
using std::setfill;
using std::endl;

Section::Section(std::string name) {
	this->name = name;
	this->size = 0;
	bytes = std::vector<int8_t>();
}

void Section::addByte(int8_t newByte) {
	bytes.push_back(newByte);
}

Relocation* Section::checkByte(int byteIndex) {
	for (int i = 0; i < relocationTable.size(); i++) {
		if (relocationTable[i]->offset == byteIndex) 
			return relocationTable[i];
	}
	return nullptr;
}

bool Section::isInstruction(int byteIndex) {
	if (byteIndex < 3 || byteIndex > (size - 2))
		return false;
	uint8_t InstrDescr = bytes[byteIndex - 3];
	uint8_t AddrMode = bytes[byteIndex - 1];
	switch (InstrDescr) {
		case 0x30:				// call
		case 0x50: 				// jmp
		case 0x51: 				// jeq
		case 0x52: 				// jne
		case 0x53: 				// jgt
		case 0xA0: 				// ldr
		case 0xB0: {			// str
			switch (AddrMode) {
				case 0x00:		// immed
				case 0x03:		// regindpom
				case 0x04:		// memdir
				case 0x05:		// regdirpom
					return true;
				default: return false;
			}
			break;
		}
		default: return false;
	}
}

bool Section::isOffset(int byteIndex) {
	if (byteIndex < 3 || byteIndex > (size - 2))
		return false;
	uint8_t AddrMode = bytes[byteIndex - 1];
	if (AddrMode == 0x03 ||		// regindpom
		AddrMode == 0x05)		// regdirpom
		return true;
	return false;
}

void Section::updateRelocationOrdinal(std::map<string, Section*> sections, SymbolTable *symbolTable) {
	std::map<string, Section*>::iterator i;
	for (i = sections.begin(); i != sections.end(); i++) {
		Section* section = i->second;
		if (!section->relocationTable.empty()) {
			for (int j = 0; j < section->relocationTable.size(); j++) {
				Symbol* symbol = symbolTable->find(section->relocationTable[j]->symbol);
				if ((symbol->name == symbol->section) || !symbol->defined)
					section->relocationTable[j]->value = symbol->ordinal;
				else
					section->relocationTable[j]->value = symbolTable->find(symbol->section)->ordinal;
			}
		}
	}
}

void Section::updateOffsets(std::map<string, Section*> sections, SymbolTable *symbolTable) {
	std::map<string, Section*>::iterator i;
	for (i = sections.begin(); i != sections.end(); i++) {
		Section* section = i->second;
		if (!section->relocationTable.empty()) {
			for (int j = 0; j < section->relocationTable.size(); j++) {
				int offset = symbolTable->find(section->relocationTable[j]->value)->offset;
				int sectionOffset = symbolTable->find(section->name)->offset;
				Symbol* symbol = symbolTable->find(section->relocationTable[j]->value);
				int byteIndex = section->relocationTable[j]->offset;
				int pc_rel = (section->relocationTable[j]->type == PC_REL) ? (byteIndex + sectionOffset) : 0;
				if (section->isInstruction(byteIndex)) {
					int operand = ((section->bytes[byteIndex] << 8) | (section->bytes[byteIndex + 1] & 0xFF)) & 0xFFFF;
					operand = operand - pc_rel + offset;
					section->bytes[byteIndex] = (operand >> 8) & 0xFF;
					section->bytes[byteIndex + 1] = operand & 0xFF;
				} else {
					int operand = ((section->bytes[byteIndex + 1] << 8) | (section->bytes[byteIndex] & 0xFF)) & 0xFFFF;
					operand += offset;
					section->bytes[byteIndex] = operand & 0xFF;
					section->bytes[byteIndex + 1] = (operand >> 8) & 0xFF;
				}
			}
		}
	}
}

void Section::printHex(std::ofstream& outfile, std::map<int, Section*> sections, int startAddr, SymbolTable* symbolTable) {
	Section *merge = new Section("merge");
	int currentAddr = startAddr;
	std::map<int, Section*>::iterator i;
	for (i = sections.begin(); i != sections.end(); i++) {
		Section* section = i->second;
		int sectionOffset = symbolTable->find(section->name)->offset;
		while (sectionOffset > currentAddr) {
			merge->addByte(0);
			currentAddr++;
		}
		for (int j = 0; j < section->bytes.size(); j++) {
			merge->addByte(section->bytes[j]);
			currentAddr++;
		}
	}
	merge->size = merge->bytes.size();

	std::stringstream sstream;
	currentAddr = startAddr;
	outfile << setw(4) << setfill('0') << std::hex << startAddr << ": ";
	for (int j = 0; j < merge->size; j++) {
		outfile << std::hex << setw(2) << setfill('0') << ((int) merge->bytes[j] & 0xFF);
		if ((j+1) % 8 == 0)
			outfile << endl << setw(4) << setfill('0') << std::hex << (currentAddr + 1) << ": ";
		else
			outfile << " ";
		currentAddr++;
	}
	outfile << endl;
}

void Section::printRelocationTable(std::ofstream& outfile, std::map<string, Section*> sections) {
	outfile << "===========================RELOCATION TABLE===========================" << endl;

	std::map<string, Section*>::iterator i;
	for (i = sections.begin(); i != sections.end(); i++) {
		Section* section = i->second;

		if (!section->relocationTable.empty()) {
			outfile << "Section: " << section->name << endl;
			outfile << setw(10) << setfill(' ') << "Offset";
			outfile << setw(12) << setfill(' ') << "Rel. type";
			outfile << setw(10) << setfill(' ') << "Ordinal";
			outfile << endl << "--------------------------------" << endl;

			for (int j = 0; j < section->relocationTable.size(); j++) {
				string relType = (section->relocationTable[j]->type == ABS) ? "ABS" : "PC_REL";
				outfile << setw(10) << setfill(' ') << section->relocationTable[j]->offset;
				outfile << setw(12) << setfill(' ') << relType;
				outfile << setw(10) << setfill(' ') << section->relocationTable[j]->value << endl;
			}

			outfile << endl << endl << endl;
		}
	}
}

void Section::printSections(std::ostream& outfile, std::map<string, Section*> sections) {
	outfile << "===============================SECTIONS===============================" << endl;
	std::map<string, Section*>::iterator i;
	for (i = sections.begin(); i != sections.end(); i++) {
		Section* section = i->second;

		if (!section->bytes.empty()) {
			outfile << "Section: " << section->name << "	[" << std::dec << section->size << " bytes]" << endl;
			outfile << "-----------------------------------------------" << endl;
			std::stringstream sstream;
			for (int j = 0; j < section->size; j++) {
				outfile << std::hex << setw(2) << setfill('0') << ((int) section->bytes[j] & 0xFF);
				if ((j+1) % 16 == 0)
					outfile << endl;
				else
				outfile << " ";
			}
			outfile << endl << endl;
		}
	}
}



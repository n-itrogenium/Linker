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

int Section::checkByte(int8_t byte) {
	for (int i = 0; i < relocationTable.size(); i++) {
		if (relocationTable[i]->offset == byte) 
			return relocationTable[i]->value;
	}
	return 0;
}

void Section::printHex(std::ofstream& outfile, std::map<string, Section*> sections, int startAddr) {
	Section *merge = new Section("merge");
	std::map<string, Section*>::iterator i;
	for (i = sections.begin(); i != sections.end(); i++) {
		Section* section = i->second;
		for (int j = 0; j < section->bytes.size(); j++) {
			merge->addByte(section->bytes[j]);
		}
	}
	merge->size = merge->bytes.size();

	std::stringstream sstream;
	int currentAddr = startAddr;
	outfile << "0000: ";
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



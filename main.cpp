#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <regex>
#include <map>
#include "linker.h"

using std::string;

int main(int argc, char* argv[]) {
    bool enableOptions = true;
    std::vector<string> inputFiles = std::vector<string>();
    std::map<int, string> sectionLocations = std::map<int, string>();
    string outputFile = "linker_output";
    bool linkable = false;
    bool hex = false;
    bool outputRead = false;

	for (int i = 1; i < argc; i++) {

        if (argv[i][0] != '-') {
            if (outputRead)
                inputFiles.push_back(argv[i]);
            else
                outputRead = true;
        } 
        else if (enableOptions) {

            if ((string) argv[i] == "-o") {
                outputFile = argv[i+1];
                enableOptions = false;
            }
            else if ((string) argv[i] == "-hex") {
                if (linkable) {
                    std::cerr << "ERROR! Invalid arguments: " << argv[i] << std::endl;
		            exit(1);
                }
                hex = true;
            }
            else if ((string) argv[i] == "-linkable") {
                if (hex) {
                    std::cerr << "ERROR! Invalid arguments: " << argv[i] << std::endl;
		            exit(1);
                }
                linkable = true;
            } 
            else {
                string option(argv[i]);
                if (option.substr(0,7) != "-place=" || option.substr(7) == "" || option[option.length() - 7] != '@') {
                    std::cerr << "ERROR! Invalid arguments: " << argv[i] << std::endl;
		            exit(1);
                }
                option = option.substr(7); // option = sekcija@0x0000
                string section = option.substr(0, option.length() - 7);
                string address = option.substr(option.length() - 6);

                if(!std::regex_match(address, std::regex("^0x[0-9A-Fa-f]{1,4}$"))) {
                    std::cerr << "ERROR! Invalid arguments: " << argv[i] << std::endl;
		            exit(1);
                }
                else {
                    int location;
                    std::stringstream ss;
				    ss << std::hex << address;
				    ss >> location;
                    sectionLocations[location] = section;
                }
            }

        }
        else {
            std::cerr << "ERROR! Invalid arguments: " << argv[i] << std::endl;
		    exit(1);
        }

    }

    if (inputFiles.empty() || enableOptions || (!linkable && !hex)) {
        std::cerr << "ERROR! Invalid arguments" << std::endl;
		exit(1);
    }

    std::ofstream outfile(outputFile);
    if (!outfile.is_open()) {
		    std::cerr << "ERROR! Cannot open file: " << outputFile << std::endl;
		    exit(2);
	}
    for (int i = 0; i < inputFiles.size(); i++) {
        std::ifstream infile(inputFiles[i]);
        if (!infile.is_open()) {
		    std::cerr << "ERROR! Cannot open file: " << inputFiles[i] << std::endl;
		    exit(2);
	    }
        Linker::processFile(infile);
        infile.close();
    }

    Linker::link(outfile, hex, sectionLocations);
    outfile.close();

	std::cout << "Happy ending!" << std::endl;

	return 0;
}
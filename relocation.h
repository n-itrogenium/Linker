#ifndef _RELOCATION_H_
#define _RELOCATION_H_

enum RelocationType { ABS, PC_REL };

class Relocation {
public:
	int offset;
	RelocationType type;
	int value;
};

#endif


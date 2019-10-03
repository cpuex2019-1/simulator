// global.h
//

#ifndef	GLOBAL_H
#define	GLOBAL_H

typedef unsigned char   sim_byte;
typedef	unsigned short  sim_hword;
typedef	unsigned int    sim_word;
typedef unsigned int    sim_addr;


const int regNum = 32;
const int regBits = 32;
const int hwordBits = 16;
const int wordBits = 32;
const int memorySize = 67108864; // 2^26

#endif
// end of global.h

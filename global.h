// global.h
//

#ifndef GLOBAL_H
#define GLOBAL_H

typedef unsigned char sim_byte;
typedef unsigned short sim_hword;
typedef unsigned int sim_word;
typedef unsigned int sim_addr;

enum Log { FATAL, ERROR, WARN, INFO, DEBUG, TRACE };
const int regNum = 32;
const int regBits = 32;
const int hwordBits = 16;
const int wordBits = 32;
const int memorySize = 2 * 1024 * 1024; // 2MB

#endif
// end of global.h

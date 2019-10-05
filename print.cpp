// print.cpp:
//

#include <stdio.h>
void print_binary(int data) {
    unsigned int v = (unsigned int)data;
    unsigned int mask = 1 << 31;
    do
        putchar(mask & v ? '1' : '0');
    while (mask >>= 1);
}
void print_prompt() { printf("\n >> "); }
void print_usage() {
    printf(" How to use mipsim:\n \
    \ts | step | \\n\t: run step by step\n \
    \ta | all\t\t: run all\n \
    \tr | reg\t\t: print register\n \
    \tm | memo\t: print memory from start to end\n \
    \tp | program\t: print program and line number\n \
    \tb | break\t: set breakpoint\n \
    \tl | log\t\t: change log level\n \
    \texit\t\t: exit program\n");
}

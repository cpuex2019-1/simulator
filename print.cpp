// print.cpp:
//

#include "print.h"
#include "register.h"
#include <cstdio>

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

void print_reg(int reg_num, reg *regs) {
    int reg_data = regs[reg_num].data;
    printf(" $%2d\tint:%9d\thex(16):%8x\tbinary:", reg_num, reg_data,
           ((unsigned int)reg_data));
    print_binary(reg_data);
    printf("\n");
}

void print_freg(int freg_num, freg *fregs) {
    IntAndFloat freg_data = fregs[freg_num].data;
    printf(" $f%2d\tfloat:%9f\thex(16):%8x\tbinary:", freg_num, freg_data.f,
           ((unsigned int)freg_data.i));
    print_binary(freg_data.i);
    printf("\n");
}

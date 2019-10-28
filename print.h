// print.h:
//
#ifndef PRINT_H
#define PRINT_H
#include "register.h"

void print_binary(int data);
void print_binary_with_space(int data);
void print_prompt();
void print_usage();
void print_reg(int reg_num, reg *one_reg);
void print_freg(int freg_num, freg *one_freg);

#endif

// controller.h:
// execute instructions
#ifndef EXECUTER_H
#define EXECUTER_H

#include <string>
#include "loader.h"
#include "memory.h"
#include "register.h"

class controller {
    memory * memo;
    loader * ld;
    reg *regs;
    // current line number being processed
    int line_num;
    void exec_code(string opecode, string res);
    sim_addr get_addr_by_base_plus_offset(string base_plus_offset);
    int get_reg_num(string reg_str);
    int get_immediate(string immediate_str);
public:
    //constructor
    controller(loader* l, memory* m, reg *r);

    bool exec_step();

};


#endif

// controller.h:
// execute instructions
#ifndef EXECUTER_H
#define EXECUTER_H

#include "global.h"
#include "loader.h"
#include "memory.h"
#include "register.h"
#include <string>

enum Status { ACTIVE, BREAK, END };

class controller {
    memory *memo;
    loader *ld;
    reg *regs;
    Log *log_level;

    void exec_code(vector<int> line_vec);
    sim_addr get_addr_by_base_plus_offset(string base_plus_offset);
    int get_reg_num(string reg_str);
    int get_immediate(string immediate_str);

  public:
    // current line number being processed
    int line_num;

    // constructor
    controller(loader *l, memory *m, reg *r, Log *l_level);

    Status exec_step(int break_point);
};

#endif

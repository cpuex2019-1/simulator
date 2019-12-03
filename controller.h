// controller.h:
// execute instructions
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "global.h"
#include "loader.h"
#include "memory.h"
#include "register.h"
#include <algorithm>
#include <fstream>
#include <map>
#include <string>
enum Status { ACTIVE, BREAK, END };

class controller {
    memory *memo;
    loader *ld;
    reg *regs;
    freg *fregs;
    string filename;
    FILE *outputfile; // 出力
    // FILE *inputfile;  // 入力
    ifstream ifs; // 入力
    int sp_max;
    int hp_max;
    map<int, long long int> jump_times;
    map<int, long long int> inst_times;
    void record_jump(int jump_line_num);
    void exec_code(unsigned int one_code);
    sim_addr get_addr_by_base_plus_offset(string base_plus_offset);
    int get_reg_num(string reg_str);
    int get_immediate(string immediate_str);

  public:
    // current line number being processed
    int line_num;
    Log log_level = DEBUG;

    // constructor
    controller(const char *fname, loader *l, memory *m, reg *r, freg *fr);
    // destructor
    ~controller();

    void init();
    Status exec_step(int break_point);

    void print_statistic_to_file();
    void print_inst_times();
};

#endif

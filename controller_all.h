// controller_all.h:
// execute instructions
#ifndef CONTROLLER_ALL_H
#define CONTROLLER_ALL_H

#include "global.h"
#include <fstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

enum Status { ACTIVE, END };

union IntAndFloat {
    int i;
    float f;
};

class controller {

    const char *file_name;
    string filename;
    FILE *outputfile; // 出力
    ifstream ifs;     // 入力

    // for register
    int regs[32];
    IntAndFloat fregs[32];

    // for print
    void print_binary_with_space(unsigned int v);
    void print_binary(int data);

    // for simulate
    void exec_code(unsigned int one_code);

    // for load
    int line_num;
    int load_line_num;
    int end_line_num;
    bool output_exist;
    bool input_exist;
    map<string, int> label_map;
    vector<unsigned int> machine_code;
    vector<string> raw_program;
    void load_line_label(string line);
    void load_line(string line);
    int load_file();
    unsigned int format_code(vector<string>);
    int get_reg_by_base_plus_offset(string base_plus_offset);
    int get_offset_by_base_plus_offset(string base_plus_offset);
    int get_reg_num(string reg_str);
    int get_freg_num(string reg_str);
    int get_logic_immediate(string immediate_str);
    int get_arith_immediate(string immediate_str);
    int get_line_num_by_label(string label);
    unsigned int get_machine_code_by_line_num(int l_num);
    string get_raw_program_by_line_num(int l_num);

    // for memory
    sim_word table[memorySize / 4];
    sim_byte read_byte(sim_addr addr);
    sim_word read_word(sim_addr addr);
    void write_byte(sim_addr addr, sim_byte byte_data);
    void write_word(sim_addr addr, sim_word word_data);
    void print_word_by_addr(sim_addr s_addr, sim_addr e_addr);

  public:
    // constructor
    controller(const char *fname);
    // destructor
    ~controller();

    Status exec_step(int break_point);

    Log log_level = DEBUG;

    // print
    void print_label_map();
    void print_program_map();
    void print_raw_program();
};

#endif

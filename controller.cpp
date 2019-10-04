// controller.cpp:
// execute instructions

#include "controller.h"
#include <iostream>
#include <regex>
#include <string>
using namespace std;

controller::controller(loader *l, memory *m, reg r[], bool *verb) {
    ld = l;
    memo = m;
    regs = r;
    verbose = verb;
    line_num = 1; // (line_num-1)*4 is addr for instruction?
    regs[0].data = 0;
    regs[29].data = memorySize - 4; // init sp;
}
sim_addr controller::get_addr_by_base_plus_offset(string base_plus_offset) {
    regex sep("([+-]?)([0-9]+)\\(\\$(\\d+)\\)");
    sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(),
                               sep, {1, 2, 3});

    string sign = iter->str();
    iter++;
    string offset_str = iter->str();
    int offset = 0;
    try {
        offset = stoi(offset_str); // convert string to int
    } catch (const std::invalid_argument &e) {
        cout << "[" << offset_str << "]: "
             << "invalid argument" << endl;
        exit(1);
    }
    iter++;
    string base_str = iter->str();
    int reg_num = 0;
    try {
        reg_num = stoi(base_str);
    } catch (const std::invalid_argument &e) {
        cout << "[" << offset_str << "]: "
             << "invalid argument" << endl;
        exit(1);
    }

    int base_addr =
        regs[reg_num].data; // sim_addr has the same type as sim_word

    if (sign == "-") {
        return (sim_addr)(base_addr - offset);
    } else {
        return (sim_addr)(base_addr + offset);
    }
}

int controller::get_reg_num(string reg_str) {
    regex sep("\\$(\\d+)");
    sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);
    unsigned int reg_num =
        stoi(iter->str()); // convert string to int to unsigned int
    return reg_num;
}

int controller::get_immediate(string immediate_str) {
    regex sep("([+-]?)([0-9]+)");
    sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(), sep,
                               {1, 2});
    string sign = iter->str();
    iter++;
    int immediate = stoi(iter->str()); // convert string to int to unsigned int

    if (sign == "-") {
        return -immediate;
    } else {
        return immediate;
    }
}

bool controller::exec_step() {

    vector<string> line_vec = ld->get_program_by_line_num(line_num);

    cout << "\n" << line_num << ":\t";
    for (auto itr_str = line_vec.begin(); itr_str != line_vec.end();
         ++itr_str) {
        cout << *itr_str << "\t";
    }
    cout << endl;

    exec_code(line_vec);

    cout << endl;

    if (line_num >= ld->end_line_num) {
        return false;
    }

    return true;
}

void controller::exec_code(vector<string> line_vec) {
    auto iter = line_vec.begin();
    string opecode = *iter;
    iter++;

    if (opecode == "add") { // ADD rd <- rs + rt
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data + regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " + rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "addi") { // ADDI rd <- rs + immediate
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        iter++;
        int immediate = get_immediate(*iter);

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " + immediate:" << immediate << endl;
            regs[rd].data = regs[rs].data + immediate;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "sub") { // SUB rd <- rs - rt
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data - regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " - rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "mul") { // MUL rd <- rs * rt
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data * regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " * rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "div") { // DIV rd <- rs / rt
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data / regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " / rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "mod") { // MOD rd <- rs % rt
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;
        regs[rd].data = regs[rs].data % regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " % rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "slt") { // slt Rd = if Rs < Rt then 1 else 0
        cout << "invalid opecode" << endl;
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;

        if (regs[rs].data < regs[rt].data) {
            regs[rd].data = 1;
        } else {
            regs[rd].data = 0;
        }
        if (*verbose == true) {
            cout << "rd($" << rd << ")"
                 << " <- if rs($" << rs << "):" << rs_data << " < rt($" << rt
                 << "):" << rt_data << " then 1 else 0" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }
        line_num++;

    } else if (opecode == "and") { // AND rd <- rs & rt
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data & regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " & rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "andi") { // ADNI rd <- rs & immediate
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        iter++;
        int immediate = get_immediate(*iter);

        regs[rd].data = regs[rs].data & immediate;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " & immediate:" << immediate << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "or") { // OR rd <- rs | rt
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data | regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " | rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "ori") { // ORI rd <- rs & immediate
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        iter++;
        int immediate = get_immediate(*iter);

        regs[rd].data = regs[rs].data | immediate;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " | immediate:" << immediate << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "nor") { // NOR rd <- ~(rs | rt)
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;
        regs[rd].data = ~(rs_data | regs[rt].data);

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- ~(rs($" << rs << "):" << rs_data << " | rt($" << rt
                 << "):" << rt_data << ")" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "xor") { // XOR rd <- rs ^ rt
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(*iter);
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data ^ regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " ^ rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "xori") { // XORI rd <- rs & immediate
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        iter++;
        int immediate = get_immediate(*iter);

        regs[rd].data = regs[rs].data ^ immediate;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " ^ immediate:" << immediate << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "sra") { // SRA rd <- rs >> sb (arithmetic)
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        int sb = get_immediate(*iter);

        regs[rd].data = regs[rs].data >> sb;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " >> sb:" << sb
                 << " (arithmetic)" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "srl") { // SRL rd <- rs >> sb (logical)
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        int sb = get_immediate(*iter);

        regs[rd].data = (int)((unsigned int)regs[rs].data >> (unsigned int)sb);

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " >> sb:" << sb
                 << " (logical)" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "sll") { // SLL rd <- rs << sb (logical)
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        int rs_data = regs[rs].data;
        int sb = get_immediate(*iter);

        regs[rd].data = (int)((unsigned int)regs[rs].data << (unsigned int)sb);

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " << sb:" << sb
                 << " (logical)" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "lw") { // LW rd, offset(base)
        int rd = get_reg_num(*iter);
        iter++;
        int addr = get_addr_by_base_plus_offset(*iter);
        regs[rd].data = memo->read_word(addr);
        if (*verbose == true) {
            cout << "rd($" << rd << ") <-"
                 << "\tmemo[" << addr << "]" << endl;
            cout << "reg[" << rd << "]:" << regs[rd].data << endl;
        }
        line_num++;

    } else if (opecode == "lb") { // lb rd, offset(base)
        int rd = get_reg_num(*iter);
        iter++;
        int addr = get_addr_by_base_plus_offset(*iter);
        unsigned char data = memo->read_byte(addr);
        regs[rd].data = (regs[rd].data & 0xffffff00) | (unsigned int)data;
        if (*verbose == true) {
            cout << "rd($" << rd << ") <-"
                 << "\tmemo[" << addr << "]" << endl;
            cout << "reg[" << rd << "]:" << regs[rd].data << endl;
        }
        line_num++;

    } else if (opecode == "sw") {
        int rd = get_reg_num(*iter);
        iter++;
        int addr = get_addr_by_base_plus_offset(*iter);
        memo->write_word(addr, regs[rd].data);
        if (*verbose == true) {
            cout << "rd($" << rd << "):" << regs[rd].data << "\taddr:" << addr
                 << endl;
            cout << "memory[" << addr << "]:" << memo->read_word(addr) << endl;
        }
        line_num++;

    } else if (opecode == "sb") { // sb rd, offset(base)
        int rd = get_reg_num(*iter);
        iter++;
        int addr = get_addr_by_base_plus_offset(*iter);
        memo->write_byte(addr,
                         (unsigned char)((regs[rd].data << 8 * 3) >> 8 * 3));
        if (*verbose == true) {
            cout << "rd($" << rd << "):" << regs[rd].data << "\taddr:" << addr
                 << endl;
            cout << "memory[" << addr << "]:" << memo->read_word(addr) << endl;
        }
        line_num++;

    } else if (opecode == "mov" || opecode == "move") { // mov rd <- rs
        int rd = get_reg_num(*iter);
        iter++;
        int rs = get_reg_num(*iter);
        regs[rd].data = regs[rs].data;
        if (*verbose == true) {
            cout << "rd($" << rd << ")"
                 << " <- rs($" << rs << "):" << regs[rs].data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data;
        }
        line_num++;

    } else if (opecode == "bc") { // BC label(pc+offset<<2)
        string label_str = *iter;
        line_num = ld->get_line_num_by_label(label_str);

    } else if (opecode == "beq") { // BEQ rs rt label(pc+offset<<2)
        int rs = get_reg_num(*iter);
        iter++;
        int rt = get_reg_num(*iter);
        iter++;
        if (regs[rs].data == regs[rt].data) {
            string label_str = *iter;
            line_num = ld->get_line_num_by_label(label_str);
        } else {
            line_num++;
        }
        if (*verbose == true) {
            cout << "rs($" << rs << "):" << regs[rs].data << "  rt($" << rt
                 << "):" << regs[rt].data << endl;
            cout << "next line:" << line_num;
        }

    } else if (opecode == "bne") { // BNE rs rt label(pc+offset<<2)
        int rs = get_reg_num(*iter);
        iter++;
        int rt = get_reg_num(*iter);
        iter++;
        if (regs[rs].data != regs[rt].data) {
            string label_str = *iter;
            line_num = ld->get_line_num_by_label(label_str);
        } else {
            line_num++;
        }
        if (*verbose == true) {
            cout << "rs($" << rs << "):" << regs[rs].data << "  rt($" << rt
                 << "):" << regs[rt].data << endl;
            cout << "next line:" << line_num;
        }

    } else if (opecode == "j") { // J label
        string label_str = *iter;
        line_num = ld->get_line_num_by_label(label_str);

    } else if (opecode == "jr") { // JR rs
        int rs = get_reg_num(*iter);
        line_num =
            (regs[rs].data / 4) + 1; // convert program addr to line number;

    } else if (opecode ==
               "jal") { // JAL label (next instruction addr is line_num*4)
        regs[31].data = line_num * 4;
        string label_str = *iter;
        line_num = ld->get_line_num_by_label(label_str);

    } else if (opecode == "jalr") { // JALR rd, rs or JALR rs(rd=$31)
        int rs = get_reg_num(*iter);
        iter++;
        int rd = get_reg_num(*iter);
        regs[rd].data = line_num * 4;
        line_num = (regs[rs].data / 4 + 1);
    } else if (opecode == "nop") { // nop
        line_num++;

    } else if (opecode == "out") { // output 未対応
        line_num++;

    } else {
        if (opecode != "") {
            cout << "invalid instruction" << endl;
            exit(1);
        }
        line_num++;
    }
}

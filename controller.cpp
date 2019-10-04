// controller.cpp:
// execute instructions

#include "controller.h"
#include "asm.h"
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

bool controller::exec_step() {

    vector<int> line_vec = ld->get_program_by_line_num(line_num);

    /*
        // raw_programを出力
        string one_raw_program = ld->get_raw_program_by_line_num(line_num);

        cout << "\n" << line_num << ":\t" << one_raw_program << "\t(\t";
        for (auto itr_str = line_vec.begin(); itr_str != line_vec.end();
             ++itr_str) {
            cout << *itr_str << "\t";
        }
        cout << ")" << endl;
    */
    exec_code(line_vec);

    // cout << endl;

    if (line_num >= ld->end_line_num) {
        return false;
    }

    return true;
}

void controller::exec_code(vector<int> line_vec) {
    auto iter = line_vec.begin();
    int opecode = *iter;
    iter++;

    if (opecode == ADD) { // ADD rd <- rs + rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rs_data = regs[rs].data;
        int rt = *iter;
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data + regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " + rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == ADDI) { // ADDI rd <- rs + immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        iter++;
        int immediate = *iter;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " + immediate:" << immediate << endl;
        }
        regs[rd].data = regs[rs].data + immediate;
        if (*verbose == true) {
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }
        line_num++;

    } else if (opecode == SUB) { // SUB rd <- rs - rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rs_data = regs[rs].data;
        int rt = *iter;
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data - regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " - rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == MUL) { // MUL rd <- rs * rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rs_data = regs[rs].data;
        int rt = *iter;
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data * regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " * rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == DIV) { // DIV rd <- rs / rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rs_data = regs[rs].data;
        int rt = *iter;
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data / regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " / rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == MOD) { // MOD rd <- rs % rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rs_data = regs[rs].data;
        int rt = *iter;
        int rt_data = regs[rt].data;
        regs[rd].data = regs[rs].data % regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " % rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == SLT) { // SLT Rd = if Rs < Rt then 1 else 0
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        iter++;
        int rt = *iter;
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

    } else if (opecode == AND) { // AND rd <- rs & rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        iter++;
        int rt = *iter;
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data & regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " & rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == ANDI) { // ANDI rd <- rs & immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        iter++;
        int immediate = *iter;

        regs[rd].data = regs[rs].data & immediate;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " & immediate:" << immediate << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == OR) { // OR rd <- rs | rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        iter++;
        int rt = *iter;
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data | regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " | rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == ORI) { // ORI rd <- rs & immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        iter++;
        int immediate = *iter;

        regs[rd].data = regs[rs].data | immediate;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " | immediate:" << immediate << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == NOR) { // NOR rd <- ~(rs | rt)
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        iter++;
        int rt = *iter;
        int rt_data = regs[rt].data;
        regs[rd].data = ~(rs_data | regs[rt].data);

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- ~(rs($" << rs << "):" << rs_data << " | rt($" << rt
                 << "):" << rt_data << ")" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == XOR) { // XOR rd <- rs ^ rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        iter++;
        int rt = *iter;
        int rt_data = regs[rt].data;
        regs[rd].data = rs_data ^ regs[rt].data;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " ^ rt($" << rt
                 << "):" << rt_data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == XORI) { // XORI rd <- rs & immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        iter++;
        int immediate = *iter;

        regs[rd].data = regs[rs].data ^ immediate;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " ^ immediate:" << immediate << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == SRA) { // SRA rd <- rs >> sb (arithmetic)
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        int sb = *iter;

        regs[rd].data = regs[rs].data >> sb;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " >> sb:" << sb
                 << " (arithmetic)" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == SRL) { // SRL rd <- rs >> sb (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        int sb = *iter;

        regs[rd].data = (int)((unsigned int)regs[rs].data >> (unsigned int)sb);

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " >> sb:" << sb
                 << " (logical)" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == SLL) { // SLL rd <- rs << sb (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        int rs_data = regs[rs].data;
        int sb = *iter;

        regs[rd].data = (int)((unsigned int)regs[rs].data << (unsigned int)sb);

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " << sb:" << sb
                 << " (logical)" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == LW) { // LW rd, offset(base)
        int rd = *iter;
        iter++;
        int reg = *iter;
        iter++;
        int offset = *iter;
        int addr = regs[reg].data + offset;
        regs[rd].data = memo->read_word(addr);
        if (*verbose == true) {
            cout << "rd($" << rd << ") <-"
                 << "\tmemo[" << addr << "]" << endl;
            cout << "reg[" << rd << "]:" << regs[rd].data << endl;
        }
        line_num++;

    } else if (opecode == LB) { // LB rd, offset(base)
        int rd = *iter;
        iter++;
        int reg = *iter;
        iter++;
        int offset = *iter;
        int addr = regs[reg].data + offset;
        unsigned char data = memo->read_byte(addr);
        regs[rd].data = (regs[rd].data & 0xffffff00) | (unsigned int)data;
        if (*verbose == true) {
            cout << "rd($" << rd << ") <-"
                 << "\tmemo[" << addr << "]" << endl;
            cout << "reg[" << rd << "]:" << regs[rd].data << endl;
        }
        line_num++;

    } else if (opecode == SW) {
        int rd = *iter;
        iter++;
        int reg = *iter;
        iter++;
        int offset = *iter;
        int addr = regs[reg].data + offset;
        memo->write_word(addr, regs[rd].data);
        if (*verbose == true) {
            cout << "rd($" << rd << "):" << regs[rd].data << "\taddr:" << addr
                 << endl;
            cout << "memory[" << addr << "]:" << memo->read_word(addr) << endl;
        }
        line_num++;

    } else if (opecode == SB) { // SB rd, offset(base)
        int rd = *iter;
        iter++;
        int reg = *iter;
        iter++;
        int offset = *iter;
        int addr = regs[reg].data + offset;
        memo->write_byte(addr,
                         (unsigned char)((regs[rd].data << 8 * 3) >> 8 * 3));
        if (*verbose == true) {
            cout << "rd($" << rd << "):" << regs[rd].data << "\taddr:" << addr
                 << endl;
            cout << "memory[" << addr << "]:" << memo->read_word(addr) << endl;
        }
        line_num++;

    } else if (opecode == MOV) { // MOV rd <- rs
        int rd = *iter;
        iter++;
        int rs = *iter;
        regs[rd].data = regs[rs].data;
        if (*verbose == true) {
            cout << "rd($" << rd << ")"
                 << " <- rs($" << rs << "):" << regs[rs].data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data;
        }
        line_num++;

    } else if (opecode == BC) { // BC label(pc+offset<<2)
        line_num = *iter;

    } else if (opecode == BEQ) { // BEQ rs rt label(pc+offset<<2)
        int rs = *iter;
        iter++;
        int rt = *iter;
        iter++;
        if (regs[rs].data == regs[rt].data) {
            line_num = *iter;
        } else {
            line_num++;
        }
        if (*verbose == true) {
            cout << "rs($" << rs << "):" << regs[rs].data << "  rt($" << rt
                 << "):" << regs[rt].data << endl;
            cout << "next line:" << line_num;
        }

    } else if (opecode == BNE) { // BNE rs rt label(pc+offset<<2)
        int rs = *iter;
        iter++;
        int rt = *iter;
        iter++;
        if (regs[rs].data != regs[rt].data) {
            line_num = *iter;
        } else {
            line_num++;
        }
        if (*verbose == true) {
            cout << "rs($" << rs << "):" << regs[rs].data << "  rt($" << rt
                 << "):" << regs[rt].data << endl;
            cout << "next line:" << line_num;
        }

    } else if (opecode == J) { // J label
        line_num = *iter;

    } else if (opecode == JR) { // JR rs
        int rs = *iter;
        line_num =
            (regs[rs].data / 4) + 1; // convert program addr to line number;

    } else if (opecode ==
               JAL) { // JAL label (next instruction addr is line_num*4)
        regs[31].data = line_num * 4;
        line_num = *iter;

    } else if (opecode == JALR) { // JALR rd, rs
        int rs = *iter;
        iter++;
        int rd = *iter;
        regs[rd].data = line_num * 4;
        line_num = (regs[rs].data / 4 + 1);
    } else if (opecode == NOP) { // nop
        line_num++;

    } else if (opecode == OUT) { // output 未対応
        line_num++;

    } else {
        line_num++;
    }
}

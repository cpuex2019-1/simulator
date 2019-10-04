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

bool controller::exec_step() {

    string line_str = ld->get_program_by_line_num(line_num);

    // split opecode and residual and remove redundant spases
    regex sep("[\\t ]+");
    sregex_token_iterator iter(line_str.begin(), line_str.end(), sep, -1);
    sregex_token_iterator end;
    string opecode_str = iter->str();
    iter++;
    string res_str = "";
    for (; iter != end; iter++) {
        res_str = res_str + iter->str();
    }

    cout << "line:" << line_num << "\topecode: "
         << "[" << opecode_str << "]"
         << "\tres: "
         << "[" << res_str << "]" << endl;

    exec_code(opecode_str, res_str);

    cout << endl;

    if (line_num >= ld->end_line_num) {
        return false;
    }

    return true;
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
        // for debug
        // cout <<"str:"<< base_plus_offset << "\tbase addr:" << base_addr  <<
        // "\toffset:" << sign <<  offset << "\tresult:" << base_addr - offset
        // << endl;
        return (sim_addr)(base_addr - offset);
    } else {
        // for debug
        // cout <<"str:"<< base_plus_offset << "\tbase addr:" << base_addr  <<
        // "\toffset:" << sign <<  offset << "\tresult:" << base_addr + offset
        // << endl;
        return (sim_addr)(base_addr + offset);
    }
}

int controller::get_reg_num(string reg_str) {
    regex sep("\\$(\\d+)");
    sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);

    // for debug
    // cout <<"from reg_str:"<< reg_str << " to reg num:" << iter->str() <<
    // endl;

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
        // for debug
        // cout <<"from immediate_str:"<< immediate_str << " to immediate:" << -
        // immediate << endl;
        return -immediate;
    } else {
        // for debug
        // cout <<"from immediate_str:"<< immediate_str << " to immediate:" <<
        // immediate << endl;
        return immediate;
    }
}

void controller::exec_code(string opecode, string res) {
    // split res by ","
    regex sep(",");
    sregex_token_iterator iter(res.begin(), res.end(), sep, -1);
    sregex_token_iterator end;

    if (opecode == "add") { // ADD rd <- rs + rt
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        iter++;
        int immediate = get_immediate(iter->str());

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " + immediate:" << immediate << endl;
            regs[rd].data = regs[rs].data + immediate;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "sub") { // SUB rd <- rs - rt
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        iter++;
        int rs_data = regs[rs].data;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        iter++;
        int immediate = get_immediate(iter->str());

        regs[rd].data = regs[rs].data & immediate;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " & immediate:" << immediate << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "or") { // OR rd <- rs | rt
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        iter++;
        int immediate = get_immediate(iter->str());

        regs[rd].data = regs[rs].data | immediate;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " | immediate:" << immediate << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "nor") { // NOR rd <- ~(rs | rt)
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        iter++;
        int rt = get_reg_num(iter->str());
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
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        iter++;
        int immediate = get_immediate(iter->str());

        regs[rd].data = regs[rs].data ^ immediate;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data
                 << " ^ immediate:" << immediate << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "sra") { // SRA rd <- rs >> sb (arithmetic)
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        int sb = get_immediate(iter->str());

        regs[rd].data = regs[rs].data >> sb;

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " >> sb:" << sb
                 << " (arithmetic)" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "srl") { // SRL rd <- rs >> sb (logical)
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        int sb = get_immediate(iter->str());

        regs[rd].data = (int)((unsigned int)regs[rs].data >> (unsigned int)sb);

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " >> sb:" << sb
                 << " (logical)" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "sll") { // SLL rd <- rs << sb (logical)
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        int rs_data = regs[rs].data;
        int sb = get_immediate(iter->str());

        regs[rd].data = (int)((unsigned int)regs[rs].data << (unsigned int)sb);

        if (*verbose == true) {
            cout << "rd($" << rd << "):"
                 << " <- rs($" << rs << "):" << rs_data << " << sb:" << sb
                 << " (logical)" << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    } else if (opecode == "lw") { // LW rd, offset(base)
        int rd = get_reg_num(iter->str());
        iter++;
        int addr = get_addr_by_base_plus_offset(iter->str());
        regs[rd].data = memo->read_word(addr);
        if (*verbose == true) {
            cout << "rd($" << rd << ") <-"
                 << "\tmemo[" << addr << "]" << endl;
            cout << "reg[" << rd << "]:" << regs[rd].data << endl;
        }
        line_num++;

    } else if (opecode == "lb") { // lb rd, offset(base)
        int rd = get_reg_num(iter->str());
        iter++;
        int addr = get_addr_by_base_plus_offset(iter->str());
        unsigned char data = memo->read_byte(addr);
        regs[rd].data = (regs[rd].data & 0xffffff00) | (unsigned int)data;
        if (*verbose == true) {
            cout << "rd($" << rd << ") <-"
                 << "\tmemo[" << addr << "]" << endl;
            cout << "reg[" << rd << "]:" << regs[rd].data << endl;
        }
        line_num++;

    } else if (opecode == "sw") {
        int rd = get_reg_num(iter->str());
        iter++;
        int addr = get_addr_by_base_plus_offset(iter->str());
        memo->write_word(addr, regs[rd].data);
        if (*verbose == true) {
            cout << "rd($" << rd << "):" << regs[rd].data << "\taddr:" << addr
                 << endl;
            cout << "memory[" << addr << "]:" << memo->read_word(addr) << endl;
        }
        line_num++;

    } else if (opecode == "sb") { // sb rd, offset(base)
        int rd = get_reg_num(iter->str());
        iter++;
        int addr = get_addr_by_base_plus_offset(iter->str());
        memo->write_byte(addr,
                         (unsigned char)((regs[rd].data << 8 * 3) >> 8 * 3));
        if (*verbose == true) {
            cout << "rd($" << rd << "):" << regs[rd].data << "\taddr:" << addr
                 << endl;
            cout << "memory[" << addr << "]:" << memo->read_word(addr) << endl;
        }
        line_num++;

    } else if (opecode == "mov" || opecode == "move") { // mov rd <- rs
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        regs[rd].data = regs[rs].data;
        if (*verbose == true) {
            cout << "rd($" << rd << ")"
                 << " <- rs($" << rs << "):" << regs[rs].data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data;
        }
        line_num++;

    } else if (opecode == "bc") { // BC label(pc+offset<<2)
        string label_str = iter->str();
        line_num = ld->get_line_num_by_label(label_str);

    } else if (opecode == "beq") { // BEQ rs rt label(pc+offset<<2)
        int rs = get_reg_num(iter->str());
        iter++;
        int rt = get_reg_num(iter->str());
        iter++;
        if (regs[rs].data == regs[rt].data) {
            string label_str = iter->str();
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
        int rs = get_reg_num(iter->str());
        iter++;
        int rt = get_reg_num(iter->str());
        iter++;
        if (regs[rs].data != regs[rt].data) {
            string label_str = iter->str();
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
        string label_str = iter->str();
        line_num = ld->get_line_num_by_label(label_str);

    } else if (opecode == "jr") { // JR rs
        int rs = get_reg_num(iter->str());
        line_num =
            (regs[rs].data / 4) + 1; // convert program addr to line number;

    } else if (opecode ==
               "jal") { // JAL label (next instruction addr is line_num*4)
        regs[31].data = line_num * 4;
        string label_str = iter->str();
        line_num = ld->get_line_num_by_label(label_str);

    } else if (opecode == "jalr") { // JALR rd, rs or JALR rs(rd=$31)
        int rs = get_reg_num(iter->str());
        iter++;
        if (iter != end) { // JALR rd, rs
            int rd = get_reg_num(iter->str());
            regs[rd].data = line_num * 4;
            line_num = (regs[rs].data / 4 + 1);
        } else {
            regs[31].data = line_num * 4;
            line_num = (regs[rs].data / 4 + 1);
        }
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

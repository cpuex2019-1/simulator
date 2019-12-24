// controller_all.cpp:
// execute instructions

#include "controller_all.h"
#include "asm.h"
#include "global.h"
#include <bitset>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

string label_str_tmp;

controller::controller(const char *fname) {
    // for load
    file_name = fname;
    load_line_num = 0;
    line_num = 0; // reset line number
    ex_count = 0;
    end_line_num = 1000000;

    output_exist = false;
    input_exist = false;
    end_line_num = load_file();

    // for memory
    for (sim_addr i = 0; i < memorySize / 4; i++) {
        table[i] = 0x0;
    }

    // start simulate
    line_num = 0;
    regs[0] = 0;
    regs[29] = 0; // init sp;

    // for output
    if (output_exist) {
        string outputfile_name = fname;
        outputfile_name.pop_back(); // 最後のsを削除
        outputfile_name = outputfile_name + "ppm";
        outputfile = fopen(outputfile_name.c_str(), "w");
        if (outputfile == NULL) { // オープンに失敗した場合
            printf("cannot open output file: %s\n", outputfile_name.c_str());
            exit(1);
        }
    }

    if (input_exist) {
        string inputfile_name = fname;
        inputfile_name.pop_back(); // 最後のsを削除
        inputfile_name = inputfile_name + "txt";
        ifs.open(inputfile_name);
        if (!ifs) { // オープンに失敗した場合
            printf("cannot open input file: %s\n", inputfile_name.c_str());
            // exit(1);
        }
    }
}

// destructor
controller::~controller() {
    fclose(outputfile);
    ifs.close();
}

// for print
void controller::print_binary(int data) {
    unsigned int v = (unsigned int)data;
    unsigned int mask = 1 << 31;
    do
        putchar(mask & v ? '1' : '0');
    while (mask >>= 1);
}
void controller::print_binary_with_space(unsigned int v) {
    unsigned int mask = 1 << 31;
    do {
        if (0x2108420 & mask) { // 00000010000100001000010000100000
            putchar(' ');
        }
        putchar(mask & v ? '1' : '0');
    } while (mask >>= 1);
}

// for memory
sim_byte controller::read_byte(sim_addr addr) {
    int offset = addr % 4;
    return (sim_byte)((table[addr / 4] >> offset * 8) & 0xff);
}
sim_word controller::read_word(sim_addr addr) { return table[addr / 4]; }
void controller::write_byte(sim_addr addr, sim_byte byte_data) {
    int offset = addr % 4;
    table[addr / 4] = (sim_word)((sim_word)byte_data << offset * 8);
}
void controller::write_word(sim_addr addr, sim_word word_data) {
    table[addr / 4] = word_data;
}
// print word from s_addr to e_addr
void controller::print_word_by_addr(sim_addr s_addr, sim_addr e_addr) {
    if (s_addr < memorySize && s_addr % 4 == 0 && e_addr + 4 <= memorySize &&
        e_addr % 4 == 0 && s_addr <= e_addr) {
        for (sim_addr addr = s_addr; addr <= e_addr; addr += 4) {
            sim_word word = read_word(addr);
            printf("%9d:\t%9d\t%8x\t", addr, table[addr / 4], table[addr / 4]);
            print_binary(word);
            printf("\n");
        }
    } else {
        if (log_level >= ERROR) {
            printf("ERROR\tinvalid address: [%d] to [%d]\n", s_addr, e_addr);
            printf("ERROR\tPlease input Multiples of 4\n");
        }
    }
}

// for simulate
Status controller::exec_step(int break_point) {
    unsigned int one_code = get_machine_code_by_line_num(line_num);
    ex_count++;
    exec_code(one_code);
    if (line_num >= end_line_num) {
        return END;
    }
    return ACTIVE;
}

void controller::exec_code(unsigned int one_code) {

    // unsigned int op_mask = 0xFE000000;          //上位6bit(<< 26)
    unsigned int rd_mask = 0x03E00000;          // 5bit(<< 21)
    unsigned int rs_mask = 0x001F0000;          // 5bit(<< 16)
    unsigned int rt_mask = 0x0000F800;          // 5bit(<< 11)
    unsigned int shamt_mask = 0x000007C0;       // 5bit(<< 6)
    unsigned int funct_mask = 0x0000003F;       // 6bit(<< 0)
    unsigned int addr_or_imm_mask = 0x0000FFFF; // 16bit
    unsigned int address_mask = 0x03FFFFFF;     // 26bit

    unsigned int opecode = one_code >> 26;
    unsigned int rd;
    unsigned int rs;
    unsigned int rt;
    unsigned int shamt = (one_code & shamt_mask) >> 6;
    unsigned int funct = one_code & funct_mask;
    int immediate = 0;
    int sb;
    int reg;
    int offset;
    int addr;
    unsigned int mask;
    int base;
    int label_line;

    switch (opecode) {
    case 0: {
        switch (funct) {
        // nop or SLLI
        case 0: { // SLLI rd <- rs << sb (logical)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            sb = (one_code & shamt_mask) >> 6;
            regs[rd] = (int)((unsigned int)regs[rs] << (unsigned int)sb);
            line_num++;
            break;
        }

        // SRLI
        case 2: { // SRLI rd <- rs >> sb (logical)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            sb = (one_code & shamt_mask) >> 6;
            regs[rd] = (int)((unsigned int)regs[rs] >> (unsigned int)sb);
            line_num++;
            break;
        }

        // SRAI
        case 3: { // SRAI rd <- rs >> sb (arithmetic)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            sb = (one_code & shamt_mask) >> 6;
            regs[rd] = regs[rs] >> sb;
            line_num++;
            break;
        }

        // SLL
        case 4: { // SLL rd <- rs << rt (logical)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            // 下位5ビットの取り出し
            mask = 0x1F;
            regs[rd] = (int)((unsigned int)regs[rs]
                             << (mask & (unsigned int)regs[rt]));
            line_num++;
            break;
        }

        // ITOF
        case 12: { // ITOF *rd <- itof(rs)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            fregs[rd].f = (float)regs[rs];
            line_num++;
            break;
        }

        // SRL
        case 6: { // SRL rd <- rs >> rt (logical)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            // 下位5ビットの取り出し
            mask = 0x1F;
            regs[rd] = (int)((unsigned int)regs[rs] >>
                             (mask & (unsigned int)regs[rt]));
            line_num++;
            break;
        }

        // SRA
        case 7: { // SRA rd <- rs >> rt (arithmetic)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            // 下位5ビットの取り出し
            mask = 0x1F;
            regs[rd] = regs[rs] >> (mask & regs[rt]);
            line_num++;
            break;
        }

        // JR or JALR
        case 9: { // JALR rd, rs
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            if (rd != 0) { // 0レジスタには書き込みしない
                regs[rd] = line_num * 4 + 4;
            }
            line_num = regs[rs] / 4;
            break;
        }

        // MUL
        case 24: { // MUL rd <- rs * rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            regs[rd] = regs[rs] * regs[rt];
            line_num++;
            break;
        }

        // DIV or MOD
        case 26: {
            if (shamt == 0x2) {
                rt = (one_code & rt_mask) >> 11;
                if (rt == 10) { // DIV10 rd <- rs / 10
                    rd = (one_code & rd_mask) >> 21;
                    rs = (one_code & rs_mask) >> 16;
                    regs[rd] = regs[rs] / 10;
                    line_num++;
                    break;
                }

            } else if (shamt == 0x3) { // MOD rd <- rs % rt
                rd = (one_code & rd_mask) >> 21;
                rs = (one_code & rs_mask) >> 16;
                rt = (one_code & rt_mask) >> 11;
                regs[rd] = regs[rs] % regs[rt];
                line_num++;
                break;
            }
        }

        // ADD or MOV
        case 32: {
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            regs[rd] = regs[rs] + regs[rt];
            line_num++;
            break;
        }

        // SUB
        case 34: { // SUB rd <- rs - rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            regs[rd] = regs[rs] - regs[rt];
            line_num++;
            break;
        }

        // AND
        case 36: { // AND rd <- rs & rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            regs[rd] = regs[rs] & regs[rt];
            line_num++;
            break;
        }

        // OR
        case 37: { // OR rd <- rs | rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            regs[rd] = regs[rs] | regs[rt];
            line_num++;
            break;
        }

        case 38: { // XOR rd <- rs ^ rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            regs[rd] = regs[rs] ^ regs[rt];
            line_num++;
            break;
        }

        case 39: { // NOR rd <- ~(rs | rt)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            regs[rd] = ~(regs[rs] | regs[rt]);
            line_num++;
            break;
        }
        case 42: { // SLT Rd = if Rs < Rt then 1 else 0
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            if (regs[rs] < regs[rt]) {
                regs[rd] = 1;
            } else {
                regs[rd] = 0;
            }
            line_num++;
            break;
        }
        }
        break;
    }

    case 0x11: { // opecode == 010001
        switch (funct) {

        // FADD and FMOV
        case 0: { // FADD rd <- rs +. rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            fregs[rd].f = fregs[rs].f + fregs[rt].f;
            line_num++;
            break;
        }

        case 1: { // FSUB rd <- rs -. rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            fregs[rd].f = fregs[rs].f - fregs[rt].f;
            line_num++;
            break;
        }

        case 2: { // FMUL rd <- rs *. rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            fregs[rd].f = fregs[rs].f * fregs[rt].f;
            line_num++;
            break;
        }
        case 3: { // FDIV rd <- rs /. rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            fregs[rd].f = fregs[rs].f / fregs[rt].f;
            line_num++;
            break;
        }
        case 9: { // FNEG rd <- -rs
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            fregs[rd].f = -fregs[rs].f;
            line_num++;
            break;
        }

        case 10: { // FABS rd <- abs(rs)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            fregs[rd].f = abs(fregs[rs].f);
            line_num++;
            break;
        }

        case 11: { // FLOOR rd <- -rs
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            fregs[rd].f = floor(fregs[rs].f);
            line_num++;
            break;
        }

        case 12: { // FTOI rd <- ftoi(rs)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            regs[rd] = (int)round(fregs[rs].f);
            line_num++;
            break;
        }

        case 4: { // SQRT rd <- sqrt(rs)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            fregs[rd].f = sqrt(fregs[rs].f);
            line_num++;
            break;
        }

        case 5: { // SIN rd <- sin(rs)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            fregs[rd].f = sin(fregs[rs].f);
            line_num++;
            break;
        }

        case 6: { // COS rd <- cos(rs)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            fregs[rd].f = cos(fregs[rs].f);
            line_num++;
            break;
        }

        case 7: { // ATAN rd <- atan(rs)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            fregs[rd].f = atan(fregs[rs].f);
            line_num++;
            break;
        }

        case 8: { // SLTF Rd[0] = if Rs < Rt then 1 else 0
            // * rd is a general register
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;
            if (fregs[rs].f < fregs[rt].f) {
                regs[rd] = 1;
            } else {
                regs[rd] = 0;
            }
            line_num++;
            break;
        }

        case 63: { // MOVF rd <- rs
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            fregs[rd].f = fregs[rs].f;
            line_num++;
            break;
        }
        }

        break;
    }

    case 0x2: { // J label
        label_line = (one_code & address_mask);
        line_num = label_line;
        break;
    }

    case 0x3: { // JAL label (next addr is line_num*4)
        label_line = (one_code & address_mask);
        regs[31] = line_num * 4 + 4;
        line_num = label_line;
        break;
    }

    case 4: { // BEQ rs rt label(pc+offset<<2)
        rs = (one_code & rd_mask) >> 21;
        rt = (one_code & rs_mask) >> 16;
        label_line = (one_code & addr_or_imm_mask);
        if ((label_line & 0x8000) == 0x8000) { //符号拡張
            label_line = 0xffff0000 | label_line;
        }

        if (regs[rs] == regs[rt]) {
            line_num = line_num + label_line;
        } else {
            line_num++;
        }

        break;
    }

    case 5: { // BNE rs rt label(pc+offset<<2)
        rs = (one_code & rd_mask) >> 21;
        rt = (one_code & rs_mask) >> 16;
        label_line = (one_code & addr_or_imm_mask);
        if ((label_line & 0x8000) == 0x8000) { //符号拡張
            label_line = 0xffff0000 | label_line;
        }
        if (regs[rs] != regs[rt]) {
            line_num = line_num + label_line;
        } else {
            line_num++;
        }
        break;
    }

    case 6: { // BLE rs rt label(pc+offset<<2)
        rs = (one_code & rd_mask) >> 21;
        rt = (one_code & rs_mask) >> 16;
        label_line = (one_code & addr_or_imm_mask);
        if ((label_line & 0x8000) == 0x8000) { //符号拡張
            label_line = 0xffff0000 | label_line;
        }
        if (regs[rs] <= regs[rt]) {
            line_num = line_num + label_line;
        } else {
            line_num++;
        }
        break;
    }

    case 7: { // BGE rs rt label(pc+offset<<2)
        rs = (one_code & rd_mask) >> 21;
        rt = (one_code & rs_mask) >> 16;
        label_line = (one_code & addr_or_imm_mask);
        if ((label_line & 0x8000) == 0x8000) { //符号拡張
            label_line = 0xffff0000 | label_line;
        }
        if (regs[rs] >= regs[rt]) {
            line_num = line_num + label_line;
        } else {
            line_num++;
        }
        break;
    }

    case 8: { // ADDI rd <- rs + immediate
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
        if ((immediate & 0x8000) == 0x8000) { //符号拡張
            immediate = 0xffff0000 | immediate;
        }
        regs[rd] = regs[rs] + immediate;
        line_num++;
        break;
    }

    case 12: { // ANDI rd <- rs & immediate
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
        regs[rd] = regs[rs] & (immediate & 0xffff);
        line_num++;
        break;
    }

    case 13: { // ORI rd <- rs & immediate
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
        regs[rd] = regs[rs] | (immediate & 0xffff);
        line_num++;
        break;
    }

    case 14: { // XORI rd <- rs & immediate
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
        regs[rd] = regs[rs] ^ immediate;
        regs[rd] = regs[rs] ^ (immediate & 0xffff);
        line_num++;
        break;
    }

    case 32: { // LB rd, offset(base)
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[reg] + offset;
        if (!(0 <= addr && addr < memorySize)) {
            string one_raw_program = get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        unsigned char data = read_byte(addr);
        regs[rd] = (regs[rd] & 0xffffff00) | (unsigned int)data;
        line_num++;
        break;
    }

    case 35: { // LW rd, offset(base)
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[reg] + offset;
        if (!(0 <= addr && addr < memorySize && addr % 4 == 0)) {
            string one_raw_program = get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        regs[rd] = read_word(addr);
        line_num++;
        break;
    }

    case 40: { // SB rd, offset(base)
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[reg] + offset;
        if (!(0 <= addr && addr < memorySize)) {
            string one_raw_program = get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        write_byte(addr, (unsigned char)((regs[rd] << 8 * 3) >> 8 * 3));
        line_num++;
        break;
    }

    case 43: { // SW
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[reg] + offset;
        if (!(0 <= addr && addr < memorySize && addr % 4 == 0)) {
            string one_raw_program = get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        write_word(addr, regs[rd]);
        line_num++;
        break;
    }

    case 49: { // LF rd, offset(base)
        rd = (one_code & rd_mask) >> 21;
        base = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[base] + offset;
        if (!(0 <= addr && addr < memorySize && addr % 4 == 0)) {
            string one_raw_program = get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        fregs[rd].i = read_word(addr);
        line_num++;
        break;
    }

    case 50: { // BC label(pc+offset<<2)
        offset = (one_code & address_mask);
        if ((offset & 0x2000000) == 0x2000000) { //符号拡張
            offset = 0xfc000000 | immediate;
        }
        line_num = line_num + offset;
        break;
    }

    case 57: { // SF
        rd = (one_code & rd_mask) >> 21;
        base = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[base] + offset;
        if (!(0 <= addr && addr < memorySize && addr % 4 == 0)) {
            string one_raw_program = get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        write_word(addr, fregs[rd].i);
        line_num++;
        break;
    }

    // INB, IN, OUT, OUTB
    case 63: {
        switch (shamt) {

        case 0: {
            switch (funct) {

            case 0: { // INB rd
                rd = (one_code & rd_mask) >> 21;
                char str;
                if (!ifs.get(str)) { // オープンに失敗した場合
                    printf("input file was not opened\n");
                    // exit(1);
                } else {
                    if (ifs.eof()) {
                        string one_raw_program =
                            get_raw_program_by_line_num(line_num);
                        printf("FATAL\n\tread EOF! program address:%d\n\t%s\n",
                               line_num * 4, one_raw_program.c_str());
                        exit(1);
                    }
                    regs[rd] = (int)((unsigned char)str);
                }
                line_num++;
                break;
            }

            case 1: { // outb rs
                rs = (one_code & rs_mask) >> 16;
                char lower8 = (char)(((unsigned int)regs[rs]) & 0xff);
                fprintf(outputfile, "%c", lower8);
                line_num++;
                break;
            }
            }

            break;
        }

        case 3: {
            switch (funct) {

            case 0: {
                // IN rd
                rd = (one_code & rd_mask) >> 21;
                int tmp;
                if (!ifs) { // オープンに失敗した場合
                    printf("input file was not opened\n");
                    // exit(1);
                } else {
                    if (!(ifs >> tmp)) {
                        string one_raw_program =
                            get_raw_program_by_line_num(line_num);
                        printf("FATAL\n\tread EOF! program address:%d\n\t%s\n",
                               line_num * 4, one_raw_program.c_str());
                        exit(1);
                    }
                    regs[rd] = tmp;
                }
                line_num++;
                break;
            }

            case 1: { // out rs
                rs = (one_code & rs_mask) >> 16;
                fprintf(outputfile, "%d", regs[rs]);
                line_num++;
                break;
            }

            case 2: { // INF rd
                rd = (one_code & rd_mask) >> 21;
                IntAndFloat tmp;
                if (!ifs) { // オープンに失敗した場合
                    printf("input file was not opened\n");
                    // exit(1);
                } else {
                    if (!(ifs >> tmp.f)) {
                        string one_raw_program =
                            get_raw_program_by_line_num(line_num);
                        printf("FATAL\n\tread EOF! program address:%d\n\t%s\n",
                               line_num * 4, one_raw_program.c_str());
                        exit(1);
                    }

                    fregs[rd].f = tmp.f;
                }
                line_num++;
                break;
            }

            case 3: { // OUTF rs
                rs = (one_code & rs_mask) >> 16;
                fprintf(outputfile, "%f", fregs[rs].f);
                line_num++;
                break;
            }
            }

            break;
        }
        }

        break;
    }

    default: {
        printf("FATAL invalid instructions:");
        print_binary_with_space(one_code);
        printf("\n");
    }
    }
}

// for load
int controller::get_reg_by_base_plus_offset(string base_plus_offset) {
    regex sep("^([+-]?)(0|[1-9][0-9]*)\\(\\$(3[0-1]|[1-2][0-9]|[0-9])\\)$");
    sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(),
                               sep, 3);
    sregex_token_iterator end;
    if (iter == end) {
        if (log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid base plus offset[%s]\n",
                   load_line_num, base_plus_offset.c_str());
        }
        exit(1);
    } else {
        string base_str = iter->str();
        int reg_num = 0;
        try {
            reg_num = stoi(base_str);
            return reg_num;
        } catch (std::out_of_range &e) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid base plus offset: "
                       "[%s](out_of_range)\n",
                       load_line_num, base_plus_offset.c_str());
            }
            exit(1);
        } catch (...) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid base plus offset: [%s]\n",
                       load_line_num, base_plus_offset.c_str());
            }
            exit(1);
        }
    }
}
int controller::get_offset_by_base_plus_offset(string base_plus_offset) {
    regex sep("^([+-]?)(0|[1-9][0-9]*)\\(\\$(3[0-1]|[1-2][0-9]|[0-9])\\)$");
    sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(),
                               sep, {1, 2});
    sregex_token_iterator end;
    if (iter == end) {
        if (log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid base plus offset[%s]\n",
                   load_line_num, base_plus_offset.c_str());
        }
        exit(1);
    } else {
        string sign = iter->str();
        iter++;
        string offset_str = iter->str();
        int offset = 0;
        try {
            offset = stoi(offset_str); // convert string to int
            if (sign == "-") {
                return -offset;
            } else {
                return offset;
            }
        } catch (std::out_of_range &e) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid base plus offset: "
                       "[%s](out_of_range)\n",
                       load_line_num, base_plus_offset.c_str());
            }
            exit(1);
        } catch (...) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid base plus offset: [%s]\n",
                       load_line_num, base_plus_offset.c_str());
            }
            exit(1);
        }
    }
}

int controller::get_reg_num(string reg_str) {
    regex sep("^\\$(3[0-1]|[1-2][0-9]|[0-9])$");
    sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);
    sregex_token_iterator end;
    if (iter == end) {
        if (log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid register[%s]\n", load_line_num,
                   reg_str.c_str());
        }
        exit(1);
    } else {
        try {
            int reg_num = stoi(iter->str()); // convert string to int
            return reg_num;
        } catch (std::out_of_range &e) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid register: "
                       "[%s](out_of_range)\n",
                       load_line_num, reg_str.c_str());
            }
            exit(1);
        } catch (...) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid register: [%s]\n",
                       load_line_num, reg_str.c_str());
            }
            exit(1);
        }
    }
}

int controller::get_freg_num(string reg_str) {
    regex sep("^\\$f(3[0-1]|[1-2][0-9]|[0-9])$");
    sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);
    sregex_token_iterator end;
    if (iter == end) {
        if (log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid register[%s]\n", load_line_num,
                   reg_str.c_str());
        }
        exit(1);
    } else {
        try {
            int reg_num = stoi(iter->str()); // convert string to int
            return reg_num;
        } catch (std::out_of_range &e) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid register: "
                       "[%s](out_of_range)\n",
                       load_line_num, reg_str.c_str());
            }
            exit(1);
        } catch (...) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid register: [%s]\n",
                       load_line_num, reg_str.c_str());
            }
            exit(1);
        }
    }
}

int controller::get_arith_immediate(string init_immediate_str) {
    // check immediate
    string immediate_str = init_immediate_str;
    regex sep("^([+-]?)(0|[1-9][0-9]*)$"); //([+-]?)([0-9]+)
    sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(), sep,
                               {1, 2});
    sregex_token_iterator end;
    if (iter != end) {
        string sign = iter->str();
        iter++;
        if (iter == end) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                       immediate_str.c_str());
            }
            exit(1);
        } else {
            try {
                int immediate =
                    stoi(iter->str()); // convert string to int to unsigned int
                if (sign == "-") {
                    if (immediate <= 32768) { // -2^15まで
                        return -immediate;
                    } else {
                        if (log_level >= FATAL) {
                            printf("FATAL\tline:%d\tinvalid immediate: "
                                   "[%s](under -2^15)\n",
                                   load_line_num, immediate_str.c_str());
                        }
                        exit(1);
                    }
                } else {
                    if (immediate <= 32767) { // 2^15-1まで
                        return immediate;
                    } else {
                        if (log_level >= FATAL) {
                            printf("FATAL\tline:%d\tinvalid immediate: "
                                   "[%s](over 2^15-1)\n",
                                   load_line_num, immediate_str.c_str());
                        }
                        exit(1);
                    }
                }
            } catch (std::out_of_range &e) {
                if (log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate: "
                           "[%s](out_of_range)\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            } catch (...) {
                if (log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate: [%s]\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            }
        }
    } else { // ha(label) or lo(label)
        immediate_str = init_immediate_str;
        regex halo_regex("^(ha|lo)\\(([A-Za-z][\\w\\.]*)\\)$");
        sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(),
                                   halo_regex, {1, 2});
        sregex_token_iterator end;
        if (iter != end) {

            string halo = iter->str();
            iter++;
            if (iter == end) {
                if (log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate[%s]\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            } else {
                string label_str =
                    iter->str(); // convert string to int to unsigned int
                int label_num = get_line_num_by_label(label_str);
                unsigned int label_addr = ((unsigned int)label_num) * 4;

                if (halo == "ha") {
                    return label_addr >> 16;
                } else if (halo == "lo") {
                    return (label_addr << 16) >> 16;
                } else {
                    if (log_level >= FATAL) {
                        printf("FATAL\tline:%d\tinvalid immediate: [%s]\n",
                               load_line_num, immediate_str.c_str());
                    }
                    exit(1);
                }
            }
        } else {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                       immediate_str.c_str());
            }
            exit(1);
        }
    }
}

int controller::get_logic_immediate(string init_immediate_str) {
    // check immediate
    string immediate_str = init_immediate_str;
    regex sep("^([+-]?)(0|[1-9][0-9]*)$"); //([+-]?)([0-9]+)
    sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(), sep,
                               {1, 2});
    sregex_token_iterator end;
    if (iter != end) {
        string sign = iter->str();
        iter++;
        if (iter == end) {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                       immediate_str.c_str());
            }
            exit(1);
        } else {
            try {
                int immediate =
                    stoi(iter->str()); // convert string to int to unsigned int

                if (sign == "-") {
                    if (log_level >= FATAL) {
                        printf("FATAL\tline:%d\tinvalid immediate: "
                               "[%s](under 0)\n",
                               load_line_num, immediate_str.c_str());
                    }
                    exit(1);
                } else {
                    if (immediate <= 65535) { // 2^16-1まで
                        return immediate;
                    } else {
                        if (log_level >= FATAL) {
                            printf("FATAL\tline:%d\tinvalid immediate: "
                                   "[%s](over 2^16-1)\n",
                                   load_line_num, immediate_str.c_str());
                        }
                        exit(1);
                    }
                }

            } catch (std::out_of_range &e) {
                if (log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate: "
                           "[%s](out_of_range)\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            } catch (...) {
                if (log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate: [%s]\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            }
        }
    } else { // ha(label) or lo(label)
        immediate_str = init_immediate_str;
        regex halo_regex("^(ha|lo)\\(([A-Za-z][\\w\\.]*)\\)$");
        sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(),
                                   halo_regex, {1, 2});
        sregex_token_iterator end;
        if (iter != end) {

            string halo = iter->str();
            iter++;
            if (iter == end) {
                if (log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate[%s]\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            } else {
                string label_str =
                    iter->str(); // convert string to int to unsigned int
                int label_num = get_line_num_by_label(label_str);
                unsigned int label_addr = ((unsigned int)label_num) * 4;

                if (halo == "ha") {
                    return label_addr >> 16;
                } else if (halo == "lo") {
                    return (label_addr << 16) >> 16;
                } else {
                    if (log_level >= FATAL) {
                        printf("FATAL\tline:%d\tinvalid immediate: [%s]\n",
                               load_line_num, immediate_str.c_str());
                    }
                    exit(1);
                }
            }
        } else {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                       immediate_str.c_str());
            }
            exit(1);
        }
    }
}

int controller::load_file() {
    // load label
    ifstream input;
    input.open(file_name);
    if (!input) {
        if (log_level >= FATAL) {
            printf("can't open file: %s\n", file_name);
        }
        exit(1);
    }
    string linebuf;
    while (!input.eof()) {
        load_line_num++;
        getline(input, linebuf);
        load_line_label(linebuf);
    }
    if (input.is_open()) {
        input.close();
    }
    int end_line = line_num;
    // load program
    line_num = 0;
    load_line_num = 0;
    input.open(file_name);
    if (!input) {
        if (log_level >= FATAL) {
            printf("can't open file: %s\n", file_name);
        }
        exit(1);
    }
    while (!input.eof()) {

        load_line_num++;
        getline(input, linebuf);
        load_line(linebuf);
    }
    if (input.is_open()) {
        input.close();
    }

    return end_line;
}

void controller::load_line_label(string line) {
    // delete comment
    regex comment_pattern("#.*$"); // #から末尾まで
    sregex_token_iterator iter1(line.begin(), line.end(), comment_pattern, -1);
    sregex_token_iterator end1;
    string line_not_comment = "";
    for (; iter1 != end1; iter1++) {
        line_not_comment = line_not_comment + iter1->str();
    }

    // get label
    regex label_pattern("^[\\t ]*(?:([A-Za-z][\\w\\.]*)[:])?[\\t ]*");
    sregex_token_iterator iter2(line_not_comment.begin(),
                                line_not_comment.end(), label_pattern,
                                {1, -1}); // group1: label, 残り: コード
    sregex_token_iterator end;
    string label_str = iter2->str();
    iter2++;
    string res_str = "";
    for (; iter2 != end; iter2++) {
        res_str = res_str + iter2->str();
    }

    // remove redundant spases
    vector<string> code;
    regex sep1("[\\t ]+");
    sregex_token_iterator iter3(res_str.begin(), res_str.end(), sep1, -1);
    string res = "";
    for (; iter3 != end; iter3++) {
        res = res + iter3->str();
    }

    if (label_str != "") {
        label_map.insert(std::make_pair(label_str, line_num));
    }
    if (res != "") { // 命令がある場合
        line_num++;  // increment before processing the line
    }
}

// load line
void controller::load_line(string line) {

    // delete comment
    regex comment_pattern("#.*$"); // #から末尾まで
    sregex_token_iterator iter1(line.begin(), line.end(), comment_pattern, -1);
    sregex_token_iterator end1;
    string line_not_comment = "";
    for (; iter1 != end1; iter1++) {
        line_not_comment = line_not_comment + iter1->str();
    }

    // get label
    regex label_pattern("^[\\t ]*(?:([A-Za-z][\\w\\.]*)[:])?[\\t ]*");
    sregex_token_iterator iter2(line_not_comment.begin(),
                                line_not_comment.end(), label_pattern,
                                {1, -1}); // group1: label, 残り: コード
    sregex_token_iterator end;
    string label_str = iter2->str();
    iter2++;
    string res_str = "";
    for (; iter2 != end; iter2++) {
        res_str = res_str + iter2->str();
    }
    if (label_str != "") {
        label_str_tmp = label_str;
    }
    // split opecode and residual and remove redundant spases
    vector<string> code;
    regex sep1("[\\t ]+");
    sregex_token_iterator iter3(res_str.begin(), res_str.end(), sep1, -1);
    string opecode_str = iter3->str();
    code.push_back(opecode_str);
    iter3++;
    string res = "";
    for (; iter3 != end; iter3++) {
        res = res + iter3->str();
    }

    // process operand (split res by ",")
    regex sep2(",");
    sregex_token_iterator iter4(res.begin(), res.end(), sep2, -1);
    for (; iter4 != end; iter4++) {
        code.push_back(iter4->str());
    }

    if (code[0] != "") {

        if (label_str_tmp != "") {
            raw_program.push_back(label_str_tmp + ":\t" + opecode_str + "\t" +
                                  res);
        } else {
            raw_program.push_back("\t\t" + opecode_str + "\t" + res);
        }

        unsigned int machine_code_line = format_code(code);
        machine_code.push_back(machine_code_line);
        label_str_tmp = "";
        line_num++; // increment before processing the line
    }
}

unsigned int controller::format_code(vector<string> code) {
    auto iter = code.begin();
    string opecode = *iter;
    iter++;
    unsigned int result;

    if (opecode == "add") { // ADD rd <- rs + rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x20;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "addi") { // ADDI rd <- rs + immediate
        unsigned int op_bit = 0x8 << 26;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int immediate_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int immediate = get_arith_immediate(*iter);
                immediate_bit = (unsigned int)immediate & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | immediate_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sub") { // SUB rd <- rs - rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x22;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "mul") { // MUL rd <- rs * rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x18;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "div10") { // DIV10 rd <- rs / 10
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0xa << 11;
        unsigned int shamt_bit = (0x2 << 6);
        unsigned int funct_bit = 0x1A;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "mod") { // MOD rd <- rs % rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x1A;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "slt") { // SLT Rd = if Rs < Rt then 1 else 0
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x2A;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "and") { // AND rd <- rs & rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x24;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "andi") { // ADNI rd <- rs & immediate
        unsigned int op_bit = 0xC << 26;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int immediate_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int immediate = get_logic_immediate(*iter);
                immediate_bit = (unsigned int)immediate & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | immediate_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "or") { // OR rd <- rs | rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x25;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "ori") { // ORI rd <- rs & immediate
        unsigned int op_bit = 0xD << 26;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int immediate_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int immediate = get_logic_immediate(*iter);
                immediate_bit = (unsigned int)immediate & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | immediate_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "nor") { // NOR rd <- ~(rs | rt)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x27;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "xor") { // XOR rd <- rs ^ rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x26;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "xori") { // XORI rd <- rs & immediate
        unsigned int op_bit = 0xE << 26;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int immediate_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int immediate = get_logic_immediate(*iter);
                immediate_bit = (unsigned int)immediate & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | immediate_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "srai") { // SRAI rd <- rs >> sb (arithmetic)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x3;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int sb = get_logic_immediate(*iter); // ＊要変更
                shamt_bit = ((unsigned int)sb << 6);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "srli") { // SRLI rd <- rs >> sb (logical)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x2;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int sb = get_logic_immediate(*iter); // ＊要変更
                shamt_bit = ((unsigned int)sb << 6);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "slli") { // SLLI rd <- rs << sb (logical)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x0;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int sb = get_logic_immediate(*iter); // ＊要変更
                shamt_bit = ((unsigned int)sb << 6);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sra") { // SRA rd <- rs >> rt (arithmetic)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x7;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "srl") { // SRL rd <- rs >> rt (logical)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x6;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sll") { // SLLI rd <- rs << rt (logical)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x4;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "itof") { // itof rd <- itof(rs)
        // rd is a float registor
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0xc;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fadd") { // FADD rd <- rs +. rt
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x0;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fsub") { // FSUB rd <- rs -. rt
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x1;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fmul") { // FMUL rd <- rs *. rt
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x2;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fdiv") { // FDIV rd <- rs /. rt
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x3;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fneg") { // FNEG rd <- -rs
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x9;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fabs") { // FABS rd <- abs(rs)
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0xa;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "floor") { // FLOOR rd <- floor(rs)
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0xb;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "ftoi") { // FTOI rd <- ftoi(rs)
        // rd is a general register
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0xc;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sqrt") { // SQRT rd <- sqrt(rs)
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x4;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sin") { // SIN rd <- sin(rs)
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x5;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "cos") { // COS rd <- cos(rs)
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x6;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "atan") { // ATAN rd <- atan(rs)
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x7;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sltf") { // SLTF Rd = if Rs < Rt then 1 else 0
        // *rd is a general register
        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x8;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "lw") { // LW rd, offset(base)
        unsigned int op_bit = (0x23 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int base_bit;
        unsigned int offset_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                base_bit = ((unsigned int)reg << 16);
                int offset = get_offset_by_base_plus_offset(*iter);
                offset_bit = (unsigned int)offset & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | base_bit | offset_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "lb") { // LB rd, offset(base)
        unsigned int op_bit = (0x20 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int base_bit;
        unsigned int offset_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                base_bit = ((unsigned int)reg << 16);
                int offset = get_offset_by_base_plus_offset(*iter);
                offset_bit = (unsigned int)offset & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | base_bit | offset_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sw") {
        unsigned int op_bit = (0x2B << 26);
        unsigned int rd_bit = 0x0;
        unsigned int base_bit;
        unsigned int offset_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                base_bit = ((unsigned int)reg << 16);
                int offset = get_offset_by_base_plus_offset(*iter);
                offset_bit = (unsigned int)offset & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | base_bit | offset_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sb") { // sb rt, offset(base)
        unsigned int op_bit = (0x28 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int base_bit;
        unsigned int offset_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rt = get_reg_num(*iter);
                rd_bit = ((unsigned int)rt << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                base_bit = ((unsigned int)reg << 16);
                int offset = get_offset_by_base_plus_offset(*iter);
                offset_bit = (unsigned int)offset & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | base_bit | offset_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "mov" || opecode == "move") { // mov rd <- rs
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x20;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "lf") { // lf rd, offset(base)
        unsigned int op_bit = (0x31 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int base_bit;
        unsigned int offset_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                base_bit = ((unsigned int)reg << 16);
                int offset = get_offset_by_base_plus_offset(*iter);
                offset_bit = (unsigned int)offset & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | base_bit | offset_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sf") { // SF rt, offset(base)
        unsigned int op_bit = (0x39 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int base_bit;
        unsigned int offset_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rt = get_freg_num(*iter);
                rd_bit = ((unsigned int)rt << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                base_bit = ((unsigned int)reg << 16);
                int offset = get_offset_by_base_plus_offset(*iter);
                offset_bit = (unsigned int)offset & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }

            result = op_bit | rd_bit | base_bit | offset_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "movf") { // MOVF rd <- rs
        unsigned int op_bit = (0x11 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x3f;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "bc") { // BC label(pc+offset<<2)
        unsigned int op_bit = (0x32 << 26);
        unsigned int offset_bit; // 下位16bit のみ
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                offset_bit = (label_num - line_num) & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
            result = op_bit | offset_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "beq") { // BEQ rs rt label(pc+offset<<2)
        unsigned int op_bit = (0x4 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int offset_bit; // 下位16bit のみ
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rd_bit = ((unsigned int)rs << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rt = get_reg_num(*iter);
                rs_bit = ((unsigned int)rt << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                offset_bit = (label_num - line_num) & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | offset_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "bne") { // BNE rs rt label(pc+offset<<2)
        unsigned int op_bit = (0x5 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int offset_bit; // 下位16bit のみ
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rd_bit = ((unsigned int)rs << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rt = get_reg_num(*iter);
                rs_bit = ((unsigned int)rt << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                offset_bit = (label_num - line_num) & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | offset_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
    } else if (opecode == "ble") { // BLE rs rt label(pc+offset<<2)
        unsigned int op_bit = (0x6 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int offset_bit; // 下位16bit のみ
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rd_bit = ((unsigned int)rs << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rt = get_reg_num(*iter);
                rs_bit = ((unsigned int)rt << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                offset_bit = (label_num - line_num) & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | offset_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
    } else if (opecode == "bge") { // BGE rs rt label(pc+offset<<2)
        unsigned int op_bit = (0x7 << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int offset_bit; // 下位16bit のみ
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rd_bit = ((unsigned int)rs << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rt = get_reg_num(*iter);
                rs_bit = ((unsigned int)rt << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                offset_bit = (label_num - line_num) & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            result = op_bit | rd_bit | rs_bit | offset_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "j") { // J label
        unsigned int op_bit = (0x2 << 26);
        unsigned int addr_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                addr_bit = label_num;
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }

            result = op_bit | addr_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "jr") { // JR rs
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x9;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode ==
               "jal") { // JAL label (next instruction addr is line_num*4)
        unsigned int op_bit = (0x3 << 26);
        unsigned int addr_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                addr_bit = label_num;
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
            result = op_bit | addr_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "jalr") { // JALR rd, rs
        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x9;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "inb") { // INB rd
        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x0;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        input_exist = true;

    } else if (opecode == "in") { // IN rd
        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x0;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        input_exist = true;

    } else if (opecode == "outb") { // OUTB rs
        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x0 << 6);
        unsigned int funct_bit = 0x1;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

        output_exist = true;

    } else if (opecode == "out") { // OUT rs
        unsigned int op_bit = ((unsigned int)0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x1;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        output_exist = true;

    } else if (opecode == "inf") { // INF rd
        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x2;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        input_exist = true;

    } else if (opecode == "outf") { // OUTF rs
        unsigned int op_bit = ((unsigned int)0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x3;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
            result = op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        output_exist = true;

    } else if (opecode == "nop") { // nop
        result = 0x0;

    } else {
        if (opecode != "") {
            if (log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid opecode:%s\n", load_line_num,
                       opecode.c_str());
            }
            exit(1);
        }
        result = 0x0;
    }

    if (log_level >= TRACE) {
        printf("\nDEBUG\tprogram address:%d", line_num * 4);
        string one_raw_program = get_raw_program_by_line_num(line_num);
        printf("\t%s\n", one_raw_program.c_str());
        printf("hex(16):%08x\tbinary:", result);
        print_binary_with_space(result);
        printf("\n");
    }
    return result;
}

// public

int controller::get_line_num_by_label(string label) {
    auto it = label_map.find(label);
    if (it == label_map.end()) {
        if (log_level >= FATAL) {
            printf("FATAL\tline:%d\tnot found label: %s\n", line_num,
                   label.c_str());
        }
        exit(1);
    } else {
        return label_map[label];
    }
}
/*
vector<int> controller::get_program_by_label(string label) {
    int line_num_of_label = get_line_num_by_label(label);
    return program_map[line_num_of_label];
}
*/

unsigned int controller::get_machine_code_by_line_num(int l_num) {
    return machine_code[l_num];
}

string controller::get_raw_program_by_line_num(int l_num) {
    return raw_program[l_num];
}

void controller::print_label_map() {
    printf("label map\n");
    for (auto itr = label_map.begin(); itr != label_map.end(); ++itr) {
        printf("\t%s :\t%d\n", itr->first.c_str(), (itr->second) * 4);
    }
}

void controller::print_program_map() {
    printf("program map\n");
    int line = 0;
    for (auto itr = machine_code.begin(); itr != machine_code.end(); ++itr) {
        printf("%d\t:", line);
        print_binary_with_space(*itr);
        printf("\n");
    }
}

void controller::print_raw_program() {
    printf("instruction memory\n");
    int line = 0;
    for (auto itr = raw_program.begin(); itr != raw_program.end(); ++itr) {
        printf("%8d:\t%s\n", line * 4, itr->c_str());
        line++;
    }
}

// controller.cpp:
// execute instructions

#include "controller.h"
#include "asm.h"
#include "global.h"
#include "print.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <set>
#include <string>
using namespace std;

long long count_zero_plus_one = 0;
long long bne_true_plus = 0;
long long bne_true_minus = 0;
long long bne_false = 0;

controller::controller(const char *fname, loader *l, memory *m, reg r[],
                       freg fr[]) {
    ld = l;
    memo = m;
    regs = r;
    fregs = fr;

    line_num = 0;
    filename = fname;
    ex_count = 0;

    regs[0].data = 0;
    regs[29].data = 0; // init sp;

    sp_max = 0;
    hp_max = 0;

    // for output
    if (ld->output_exist) {
        string outputfile_name = filename;
        outputfile_name.pop_back(); // 最後のsを削除
        outputfile_name = outputfile_name + "ppm";
        outputfile = fopen(outputfile_name.c_str(), "w");
        if (outputfile == NULL) { // オープンに失敗した場合
            printf("cannot open output file: %s\n", outputfile_name.c_str());
            exit(1);
        }
    }

    if (ld->input_exist) {
        string inputfile_name = filename;
        inputfile_name.pop_back(); // 最後のsを削除
        inputfile_name = inputfile_name + "txt";
        ifs.open(inputfile_name);
        if (!ifs) { // オープンに失敗した場合
            printf("cannot open input file: %s\n", inputfile_name.c_str());
            // exit(1);
        }
    }
    for (int i = ADD_OR_MOV; i <= SLLI_OR_NOP; i++) {
        inst_times.insert(make_pair(i, 0));
    }
}

// destructor
controller::~controller() {
    fclose(outputfile);
    ifs.close();
}

void controller::init() {
    line_num = 0;
    ex_count = 0;
    count_zero_plus_one = 0;
    bne_true_plus = 0;
    bne_true_minus = 0;
    bne_false = 0;

    fclose(outputfile);
    ifs.close();
    // for output
    if (ld->output_exist) {
        string outputfile_name = filename;
        outputfile_name.pop_back(); // 最後のsを削除
        outputfile_name = outputfile_name + "ppm";
        outputfile = fopen(outputfile_name.c_str(), "w");
        if (outputfile == NULL) { // オープンに失敗した場合
            printf("cannot open output file: %s\n", outputfile_name.c_str());
            exit(1);
        }
    }

    if (ld->input_exist) {
        string inputfile_name = filename;
        inputfile_name.pop_back(); // 最後のsを削除
        inputfile_name = inputfile_name + "txt";
        ifs.open(inputfile_name);
        if (!ifs) { // オープンに失敗した場合
            printf("cannot open input file: %s\n", inputfile_name.c_str());
            // exit(1);
        }
    }
}

Status controller::exec_step(int break_point) {

    sp_max = max(sp_max, regs[3].data);
    hp_max = max(hp_max, regs[4].data);

    unsigned int one_code = ld->get_machine_code_by_line_num(line_num);

    if (log_level >= INFO) {
        // raw_programを出力
        string one_raw_program = ld->get_raw_program_by_line_num(line_num);
        printf("INFO\t%d:\t%s", line_num * 4, one_raw_program.c_str());
        if (log_level >= TRACE) {
            printf("\t(TRACE\t");
            print_binary_with_space(one_code);
            printf(")");
        }
        printf("\n");
    }
    ex_count++;
    exec_code(one_code);

    if (line_num == break_point) {
        return BREAK;
    } else if (line_num >= ld->end_line_num) {
        return END;
    }

    return ACTIVE;
}

void controller::exec_code(unsigned int one_code) {

    // unsigned int op_mask = 0xFE000000;          //上位6bit(<< 26)
    unsigned int rd_mask = 0x03E00000;           // 5bit(<< 21)
    unsigned int rs_mask = 0x001F0000;           // 5bit(<< 16)
    unsigned int rt_mask = 0x0000F800;           // 5bit(<< 11)
    unsigned int addr_or_imm_mask = 0x0000FFFF;  // 16bit
    unsigned int imm_mask_for_slli = 0x0000007C; // 5bit<< 2
    unsigned int low_mask = 0x00000003;          // 2bit
    unsigned int address_mask = 0x03FFFFFF;      // 26bit

    unsigned int opecode = one_code >> 26;
    unsigned int rd;
    unsigned int rs;
    unsigned int rt;
    int immediate = 0;
    int sb;
    int reg;
    int offset;
    int addr;
    int base;
    int label_line;
    int low;

    switch (opecode) {

    case 0x1: { // LW rd, offset(base)
        inst_times[LW] += 1;
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[reg].data + offset;
        if (!(0 <= addr && addr < memorySize / 4)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (memo->check_memory && addr >= memo->heap_pointer &&
            !memo->is_int_stored(addr)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf(
                "FATAL\n\t%s\n\tprogram address:%d  read float memory[%d] by LW"
                "\n\t%lld instructions\n",
                one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- memory[%d]:%d\n", rd, addr,
                   memo->read_word(addr));
        }
        regs[rd].data = memo->read_word(addr);

        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case 0x11: { // addlw r1 r2 r3 : r1 <-mem[r2+r3]
        inst_times[ADDLW] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;
        addr = regs[rs].data + regs[rt].data;
        if (!(0 <= addr && addr < memorySize / 4)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (memo->check_memory && addr >= memo->heap_pointer &&
            !memo->is_int_stored(addr)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf(
                "FATAL\n\t%s\n\tprogram address:%d  read float memory[%d] by LW"
                "\n\t%lld instructions\n",
                one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\taddr:%d <- rs($%d):%d + rt($%d):%d\n", addr, rs,
                   regs[rs].data, rt, regs[rt].data);
            printf("\trd($%d) <- memory[%d]:%d\n", rd, addr,
                   memo->read_word(addr));
        }
        regs[rd].data = memo->read_word(addr);

        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case 0x31: { // addlf fr1 r2 r3 : fr1 <-mem[r2+r3]
        inst_times[ADDLF] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;
        addr = regs[rs].data + regs[rt].data;
        if (!(0 <= addr && addr < memorySize / 4)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (memo->check_memory && addr >= memo->heap_pointer &&
            !memo->is_int_stored(addr)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf(
                "FATAL\n\t%s\n\tprogram address:%d  read float memory[%d] by LW"
                "\n\t%lld instructions\n",
                one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\taddr:%d <- rs($%d):%d + rt($%d):%d\n", addr, rs,
                   regs[rs].data, rt, regs[rt].data);
            printf("\trd($f%d) <- memory[%d]:(hex:%8x)\n", rd, addr,
                   memo->read_word(addr));
        }
        fregs[rd].data.i = memo->read_word(addr);

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }

    case 0: { // SW
        inst_times[SW] += 1;
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[reg].data + offset;
        if (!(0 <= addr && addr < memorySize / 4)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid write address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tmemory[%d]:%d\n", addr, memo->read_word(addr));
            printf("\tmemory[%d] <- rd($%d):%d\n", addr, rd, regs[rd].data);
        }
        memo->write_word(addr, regs[rd].data, true);
        if (log_level >= DEBUG) {
            printf("\tmemory[%d]:%d\n", addr, memo->read_word(addr));
        }

        line_num++;
        break;
    }

    case 0x21: { // LF rd, offset(base)
        inst_times[LF] += 1;
        rd = (one_code & rd_mask) >> 21;
        base = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[base].data + offset;
        if (!(0 <= addr && addr < memorySize / 4)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (memo->check_memory && addr >= memo->heap_pointer &&
            !memo->is_float_stored(addr)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf(
                "FATAL\n\t%s\n\tprogram address:%d  read int memory[%d] by LF"
                "\n\t%lld instructions\n",
                one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- memory[%d]:(hex:%8x)\n", rd, addr,
                   memo->read_word(addr));
        }
        fregs[rd].data.i = memo->read_word(addr);

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }

    case 0x10: { // SF
        inst_times[SF] += 1;
        rd = (one_code & rd_mask) >> 21;
        base = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);
        if ((offset & 0x8000) == 0x8000) { //符号拡張
            offset = 0xffff0000 | offset;
        }
        addr = regs[base].data + offset;
        if (!(0 <= addr && addr < memorySize / 4)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid write address: "
                   "[%d]\n\t%lld instructions\n",
                   one_raw_program.c_str(), line_num * 4, addr, ex_count);
            exit(1);
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tmemory[%d]:(hex)%8x\n", addr, memo->read_word(addr));
            printf("\tmemory[%d] <- rd($f%d):%f(hex:%08x)\n", addr, rd,
                   fregs[rd].data.f, ((unsigned int)fregs[rd].data.i));
        }
        memo->write_word(addr, fregs[rd].data.i, false);
        if (log_level >= DEBUG) {
            printf("\tmemory[%d]:(hex)%8x\n", addr, memo->read_word(addr));
        }

        line_num++;
        break;
    }

    // ADD or MOV
    case 0x2: {
        inst_times[ADD_OR_MOV] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d + rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }

        regs[rd].data = regs[rs].data + regs[rt].data;

        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    // SUB
    case 0x3: { // SUB rd <- rs - rt
        inst_times[SUB] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d - rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data - regs[rt].data;
        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    // MUL
    case 0x4: { // MUL rd <- rs * rt
        inst_times[MUL] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d * rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data * regs[rt].data;
        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case 0x5: { // XOR rd <- rs ^ rt
        inst_times[XOR] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d ^ rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data ^ regs[rt].data;
        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case 0x8: { // ADDI rd <- rs + immediate
        inst_times[ADDI] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
        if ((immediate & 0x8000) == 0x8000) { //符号拡張
            immediate = 0xffff0000 | immediate;
        }
        if (rs == 0 && immediate == 1) {
            count_zero_plus_one++;
        }

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d + immediate:%d\n", rd, rs,
                   regs[rs].data, immediate);
        }

        regs[rd].data = regs[rs].data + immediate;

        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    // nop or SLLI or SRLI or SRAI
    case 0x9: {

        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        sb = (one_code & imm_mask_for_slli) >> 2;
        low = (one_code & low_mask);

        if (low == 0x0) { // SLLI rd <- rs << sb (logical)
            inst_times[SLLI_OR_NOP] += 1;
            if (log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf("\trd($%d) <- rs($%d):%d << sb:%d (logical)\n", rd, rs,
                       regs[rs].data, sb);
            }

            regs[rd].data =
                (int)((unsigned int)regs[rs].data << (unsigned int)sb);

            if (log_level >= DEBUG) {
                printf("\trd($%d):%d\n", rd, regs[rd].data);
            }

            line_num++;
            break;
        } else if (low == 0x1) { // SRLI rd <- rs >> sb (logical)
            inst_times[SRLI] += 1;
            if (log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf("\trd($%d) <- rs($%d):%d >> sb:%d (logical)\n", rd, rs,
                       regs[rs].data, sb);
            }

            regs[rd].data =
                (int)((unsigned int)regs[rs].data >> (unsigned int)sb);

            if (log_level >= DEBUG) {
                printf("\trd($%d):%d\n", rd, regs[rd].data);
            }

            line_num++;
            break;
        } else if (low == 0x3) { // SRAI rd <- rs >>> sb (arith)
            inst_times[SRAI] += 1;
            if (log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf("\trd($%d) <- rs($%d):%d >>> sb:%d (arith)\n", rd, rs,
                       regs[rs].data, sb);
            }

            regs[rd].data = regs[rs].data >> sb;

            if (log_level >= DEBUG) {
                printf("\trd($%d):%d\n", rd, regs[rd].data);
            }

            line_num++;
            break;
        }
    }

    case 0xA: { // ORI rd <- rs & immediate
        inst_times[ORI] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d | zero_extend(immediate):%d\n", rd,
                   rs, regs[rs].data, immediate);
        }
        regs[rd].data = regs[rs].data | (immediate & 0xffff);
        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    // FADD
    case 0x32: { // FADD rd <- rs +. rt
        inst_times[FADD] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- rs($f%d):%f(hex:%08x) +. "
                   "rt($f%d):%f(hex:%08x)\n",
                   rd, rs, fregs[rs].data.f, ((unsigned int)fregs[rs].data.i),
                   rt, fregs[rt].data.f, ((unsigned int)fregs[rt].data.i));
        }

        fregs[rd].data.f = fregs[rs].data.f + fregs[rt].data.f;

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }

    case 0x33: { // FSUB rd <- rs -. rt
        inst_times[FSUB] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- rs($f%d):%f(hex:%08x) -. "
                   "rt($f%d):%f(hex:%08x)\n",
                   rd, rs, fregs[rs].data.f, ((unsigned int)fregs[rs].data.i),
                   rt, fregs[rt].data.f, ((unsigned int)fregs[rt].data.i));
        }

        fregs[rd].data.f = fregs[rs].data.f - fregs[rt].data.f;

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }

    case 0x34: { // FMUL rd <- rs *. rt
        inst_times[FMUL] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- rs($f%d):%f(hex:%08x) *.  "
                   "rt($f%d):%f(hex:%08x)\n",
                   rd, rs, fregs[rs].data.f, ((unsigned int)fregs[rs].data.i),
                   rt, fregs[rt].data.f, ((unsigned int)fregs[rt].data.i));
        }

        fregs[rd].data.f = fregs[rs].data.f * fregs[rt].data.f;

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }
    case 0x35: { // FDIV rd <- rs /. rt
        inst_times[FDIV] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- rs($f%d):%f(hex:%08x) /. "
                   "rt($f%d):%f(hex:%08x)\n",
                   rd, rs, fregs[rs].data.f, ((unsigned int)fregs[rs].data.i),
                   rt, fregs[rt].data.f, ((unsigned int)fregs[rt].data.i));
        }

        fregs[rd].data.f = fregs[rs].data.f / fregs[rt].data.f;

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }
    case 0x36: { // FNEG rd <- -rs
        inst_times[FNEG] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- neg(rs($f%d):%f)\n", rd, rs,
                   fregs[rs].data.f);
        }
        fregs[rd].data.f = -fregs[rs].data.f;

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }

    case 0x37: { // FABS rd <- abs(rs)
        inst_times[FABS] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- abs(rs($f%d):%f)\n", rd, rs,
                   fregs[rs].data.f);
        }
        fregs[rd].data.f = abs(fregs[rs].data.f);

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }

    case 0x38: { // SQRT rd <- sqrt(rs)
        inst_times[SQRT] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- sqrt(rs($f%d):%f)\n", rd, rs,
                   fregs[rs].data.f);
        }
        fregs[rd].data.f = sqrt(fregs[rs].data.f);

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }

    case 0x39: { // FLOOR rd <- floor(rs)
        inst_times[FLOOR] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- FLOOR(rs($f%d):%f)\n", rd, rs,
                   fregs[rs].data.f);
        }
        fregs[rd].data.f = floor(fregs[rs].data.f);

        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }

    case 0x1A: { // FTOI rd <- ftoi(rs)
        inst_times[FTOI] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- ftoi(rs($f%d):%f)\n", rd, rs,
                   fregs[rs].data.f);
        }
        regs[rd].data = (int)round(fregs[rs].data.f);

        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    // ITOF
    case 0x2A: { // ITOF *rd <- itof(rs)
        inst_times[ITOF] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- itof(rs($%d):%d)\n", rd, rs, regs[rs].data);
        }
        fregs[rd].data.f = (float)(regs[rs].data);
        if (log_level >= DEBUG) {
            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }
        line_num++;
        break;
    }

    case 0x3A: { // MOVF rd <- rs
        inst_times[MOVF] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");

            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
            printf("\trd($f%d) <- rs($f%d):%f\n", rd, rs, fregs[rs].data.f);
        }
        fregs[rd].data.f = fregs[rs].data.f;
        if (log_level >= DEBUG) {

            printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                   ((unsigned int)fregs[rd].data.i));
        }

        line_num++;
        break;
    }

    case 0xB: { // J label
        inst_times[J] += 1;
        label_line = (one_code & address_mask);
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter:%d \n", line_num * 4);
            printf("\tprogram counter <- address:%d <<2 \n", label_line);
        }
        line_num = label_line;
        record_jump(line_num);
        if (log_level >= DEBUG) {
            printf("\tprogram counter:%d \n", line_num * 4);
        }
        break;
    }

    case 0xC: { // JAL label (next addr is line_num*4)
        inst_times[JAL] += 1;
        label_line = (one_code & address_mask);
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\t$31:%d\tprogram counter:%d\n", regs[31].data,
                   line_num * 4);
            printf("\t$31 <- pc+4\tprogram counter <- address:%d <<2\n",
                   label_line);
        }
        regs[31].data = line_num * 4 + 4;
        line_num = label_line;
        record_jump(line_num);
        if (log_level >= DEBUG) {
            printf("\t$31:%d\n", regs[31].data);
            printf("\tprogram counter:%d \n", line_num * 4);
        }
        break;
    }

    // JR or JALR
    case 0xD: { // JALR rd, rs
        inst_times[JR_OR_JALR] += 1;
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\tprogram counter:%d\n", rd, regs[rd].data,
                   line_num * 4);
            printf("\trd($%d) <- pc+4\tprogram counter <- rs($%d):%d\n", rd, rs,
                   regs[rs].data);
        }
        if (rd != 0) { // 0レジスタには書き込みしない
            regs[rd].data = line_num * 4 + 4;
        }
        line_num = regs[rs].data / 4;
        record_jump(line_num);
        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\tprogram counter:%d\trd($%d):%d\n", line_num * 4, rd,
                   regs[rd].data);
        }
        break;
    }

    case 0x7: { // SLT Rd[0] = if Rs < Rt then 1 else 0
        inst_times[SLT] += 1;
        // * rd is a general register
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d)[0] <- if (rs($%d):%d < "
                   "rt($%d):%d) then 1 "
                   "else 0\n",
                   rd, rs, regs[rs].data, rt, regs[rt].data);
        }
        if (regs[rs].data < regs[rt].data) {
            regs[rd].data = 1;
        } else {
            regs[rd].data = 0;
        }
        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case 0x12: { // SLTF Rd[0] = if Rs < Rt then 1 else 0
        inst_times[SLTF] += 1;
        // * rd is a general register
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        rt = (one_code & rt_mask) >> 11;

        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d)[0] <- if (rs($f%d):%f(hex:%08x) < "
                   "rt($f%d):%f(hex:%08x)) then 1 "
                   "else 0\n",
                   rd, rs, fregs[rs].data.f, ((unsigned int)fregs[rs].data.i),
                   rt, fregs[rt].data.f, ((unsigned int)fregs[rt].data.i));
        }
        if (fregs[rs].data.f < fregs[rt].data.f) {
            regs[rd].data = 1;
        } else {
            regs[rd].data = 0;
        }
        if (log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case 0x2C: { // BNE rs rt label(pc+offset<<2)
        inst_times[BNE] += 1;
        rs = (one_code & rd_mask) >> 21;
        rt = (one_code & rs_mask) >> 16;
        label_line = (one_code & addr_or_imm_mask);
        if ((label_line & 0x8000) == 0x8000) { //符号拡張
            label_line = 0xffff0000 | label_line;
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter <- if (rs($%d):%d != rt($%d):%d) then "
                   "pc+(offset:%d <<2) "
                   "else "
                   "pc+4\n",
                   rs, regs[rs].data, rt, regs[rt].data, label_line);
        }

        if (regs[rs].data != regs[rt].data) {
            if (label_line > 0) {
                bne_true_plus++;
            } else {
                bne_true_minus++;
            }

            line_num = line_num + label_line;
            record_jump(line_num);
        } else {
            bne_false++;
            line_num++;
        }
        if (log_level >= DEBUG) {
            printf("\tprogram counter:%d\n", line_num * 4);
        }
        break;
    }

    case 0x2E: { // BLE rs rt label(pc+offset<<2)
        inst_times[BLE_OR_BGE] += 1;
        rs = (one_code & rd_mask) >> 21;
        rt = (one_code & rs_mask) >> 16;
        label_line = (one_code & addr_or_imm_mask);
        if ((label_line & 0x8000) == 0x8000) { //符号拡張
            label_line = 0xffff0000 | label_line;
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter <- if (rs($%d):%d <= rt($%d):%d) then "
                   "pc+(offset:%d <<2) "
                   "else "
                   "pc+4\n",
                   rs, regs[rs].data, rt, regs[rt].data, label_line);
        }

        if (regs[rs].data <= regs[rt].data) {
            line_num = line_num + label_line;
            record_jump(line_num);
        } else {
            line_num++;
        }
        if (log_level >= DEBUG) {
            printf("\tprogram counter:%d\n", line_num * 4);
        }
        break;
    }

    case 0x1C: { // BEQF rs rt label(pc+offset<<2)
        inst_times[BEQF] += 1;
        rs = (one_code & rd_mask) >> 21;
        rt = (one_code & rs_mask) >> 16;
        label_line = (one_code & addr_or_imm_mask);
        if ((label_line & 0x8000) == 0x8000) { //符号拡張
            label_line = 0xffff0000 | label_line;
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter <- if (rs(f$%d):%f == rt(f$%d):%f) then "
                   "pc+(offset:%d <<2) "
                   "else "
                   "pc+4\n",
                   rs, fregs[rs].data.f, rt, fregs[rt].data.f, label_line);
        }

        if (fregs[rs].data.f == fregs[rt].data.f) {
            line_num = line_num + label_line;
            record_jump(line_num);
        } else {
            line_num++;
        }
        if (log_level >= DEBUG) {
            printf("\tprogram counter:%d\n", line_num * 4);
        }
        break;
    }

    case 0x1D: { // BLTF rs rt label(pc+offset<<2)
        inst_times[BLTF] += 1;
        rs = (one_code & rd_mask) >> 21;
        rt = (one_code & rs_mask) >> 16;
        label_line = (one_code & addr_or_imm_mask);
        if ((label_line & 0x8000) == 0x8000) { //符号拡張
            label_line = 0xffff0000 | label_line;
        }
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter <- if (rs(f$%d):%f < rt(f$%d):%f) then "
                   "pc+(offset:%d <<2) "
                   "else "
                   "pc+4\n",
                   rs, fregs[rs].data.f, rt, fregs[rt].data.f, label_line);
        }

        if (fregs[rs].data.f < fregs[rt].data.f) {
            line_num = line_num + label_line;
            record_jump(line_num);
        } else {
            line_num++;
        }
        if (log_level >= DEBUG) {
            printf("\tprogram counter:%d\n", line_num * 4);
        }
        break;
    }

        // INB, IN, OUT, OUTB
    case 0xE: { // outb rs
        inst_times[OUTB] += 1;
        rs = (one_code & rs_mask) >> 16;
        if (log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tOUTB rs($%d):(hex)%08x\n", rs, regs[rs].data);
        }
        char lower8 = (char)(((unsigned int)regs[rs].data) & 0xff);
        fprintf(outputfile, "%c", lower8);

        if (log_level >= DEBUG) {
            printf("\tout(char):%c\n", lower8);
        }
        line_num++;
        break;
    }

    case 0x3F: { // INF rd
        inst_times[INF] += 1;
        rd = (one_code & rd_mask) >> 21;
        IntAndFloat tmp;
        if (!ifs) { // オープンに失敗した場合
            printf("input file was not opened\n");
            // exit(1);
        } else {
            if (!(ifs >> tmp.f)) {
                string one_raw_program =
                    ld->get_raw_program_by_line_num(line_num);
                printf("FATAL\n\tread EOF! program address:%d\n\t%s\n",
                       line_num * 4, one_raw_program.c_str());
                exit(1);
            }

            if (log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\tINF rd($f%d):%f(hex:%08x) <- "
                       "get(float):%f(hex:%08x)\n",
                       rd, fregs[rd].data.f, ((unsigned int)fregs[rd].data.i),
                       tmp.f, ((unsigned int)tmp.i));
            }

            fregs[rd].data.f = tmp.f;
            if (log_level >= DEBUG) {
                printf("\trd($f%d):%f(hex:%08x)\n", rd, fregs[rd].data.f,
                       ((unsigned int)fregs[rd].data.i));
            }
        }
        line_num++;
        break;
    }
    case 0xF: { // IN rd
        inst_times[IN] += 1;
        rd = (one_code & rd_mask) >> 21;
        int tmp;
        if (!ifs) { // オープンに失敗した場合
            printf("input file was not opened\n");
            // exit(1);
        } else {
            if (!(ifs >> tmp)) {
                string one_raw_program =
                    ld->get_raw_program_by_line_num(line_num);
                printf("FATAL\n\tread EOF! program address:%d\n\t%s\n",
                       line_num * 4, one_raw_program.c_str());
                exit(1);
            }

            if (log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\tIN rd($%d):%d <- get(int):%d\n", rd, regs[rd].data,
                       tmp);
            }

            regs[rd].data = tmp;
            if (log_level >= DEBUG) {
                printf("\trd($%d):%d\n", rd, regs[rd].data);
            }
        }
        line_num++;
        break;
    }

    default: {
        if (log_level >= FATAL) {
            printf("FATAL invalid instructions:");
            print_binary_with_space(one_code);
            printf("\n");
        }
    }
    }
}

void controller::record_jump(int jump_line_num) {
    jump_times[jump_line_num] = jump_times[jump_line_num] + 1;
}

bool compare_by_b_lld(pair<int, long long int> a, pair<int, long long int> b) {
    return a.second > b.second;
    /*
    if (a.second != b.second) {
        return a.second > b.second;
    } else {
        return a.first > b.first;
    }*/
}

void controller::print_statistic_to_file() {
    // for output
    FILE *out_statistic;

    string outputfile_name = filename;
    outputfile_name.pop_back(); // 最後のsを削除
    outputfile_name = outputfile_name + "statistic.out";
    out_statistic = fopen(outputfile_name.c_str(), "w");
    if (out_statistic == NULL) { // オープンに失敗した場合
        printf("cannot open output file: %s\n", outputfile_name.c_str());
        exit(1);
    }

    fprintf(out_statistic, "max hp:%d\n", hp_max);
    fprintf(out_statistic, "max sp:%d\n", sp_max);
    fprintf(out_statistic, "$0 +1:%lld\n", count_zero_plus_one);
    fprintf(out_statistic, "bne true plus:%lld\n", bne_true_plus);
    fprintf(out_statistic, "bne true minus:%lld\n", bne_true_minus);
    fprintf(out_statistic, "bne false:%lld\n", bne_false);

    vector<pair<int, long long int>>
        jump_times_pairs;              // <labelの番号, labelのjamp回数>
    map<int, string> label_map_by_num; // <labelの番号,labelのstr>
    for (auto pair : ld->label_map) {
        label_map_by_num.insert(make_pair(pair.second, pair.first));
        jump_times_pairs.push_back(
            make_pair(pair.second, jump_times[pair.second]));
    }
    // jamp回数でソート
    sort(jump_times_pairs.begin(), jump_times_pairs.end(), compare_by_b_lld);

    fprintf(out_statistic, "jump times\n");
    set<int> s;
    for (auto pair : jump_times_pairs) {
        auto it = s.find(pair.first);
        if (it == s.end()) { // 見つからない
            fprintf(out_statistic, "\t%s :\t%lld\n",
                    label_map_by_num[pair.first].c_str(), pair.second);
            s.insert(pair.first);
        }
    }

    vector<pair<int, long long int>> inst_times_pairs;
    for (int i = LW; i <= IN; i++) {
        inst_times_pairs.push_back(make_pair(i, inst_times[i]));
    }
    sort(inst_times_pairs.begin(), inst_times_pairs.end(), compare_by_b_lld);

    fprintf(out_statistic, "inst times\n");

    for (int i = LW; i <= IN; i++) {
        auto pair = inst_times_pairs[i];
        string tmp;
        switch (pair.first) {
        case LW:
            tmp = "LW";
            break;
        case SW:
            tmp = "SW";
            break;
        case LF:
            tmp = "LF";
            break;
        case SF:
            tmp = "SF";
            break;
        case ADDLW:
            tmp = "ADDLW";
            break;
        case ADDLF:
            tmp = "ADDLF";
            break;
        case ADD_OR_MOV:
            tmp = "ADD_OR_MOV";
            break;
        case SUB:
            tmp = "SUB";
            break;
        case MUL:
            tmp = "MUL";
            break;
        case XOR:
            tmp = "XOR";
            break;
        case ADDI:
            tmp = "ADDI";
            break;
        case SLLI_OR_NOP:
            tmp = "SLLI_OR_NOP";
            break;
        case SRLI:
            tmp = "SRLI";
            break;
        case SRAI:
            tmp = "SRAI";
            break;
        case ORI:
            tmp = "ORI";
            break;
        case FADD:
            tmp = "FADD";
            break;
        case FSUB:
            tmp = "FSUB";
            break;
        case FMUL:
            tmp = "FMUL";
            break;
        case FDIV:
            tmp = "FDIV";
            break;
        case FNEG:
            tmp = "FNEG";
            break;
        case FABS:
            tmp = "FABS";
            break;
        case SQRT:
            tmp = "SQRT";
            break;
        case FLOOR:
            tmp = "FLOOR ";
            break;
        case FTOI:
            tmp = "FTOI";
            break;
        case ITOF:
            tmp = "ITOF";
            break;
        case MOVF:
            tmp = "MOVF";
            break;
        case J:
            tmp = "J";
            break;
        case JAL:
            tmp = "JAL";
            break;
        case JR_OR_JALR:
            tmp = "JR_OR_JALR";
            break;
        case SLT:
            tmp = "SLT";
            break;
        case SLTF:
            tmp = "SLTF";
            break;
        case BNE:
            tmp = "BNE";
            break;
        case BLE_OR_BGE:
            tmp = "BLE_OR_BGE";
            break;
        case BEQF:
            tmp = "BEQF";
            break;
        case BLTF:
            tmp = "BLTF";
            break;
        case OUTB:
            tmp = "OUTB";
            break;
        case IN:
            tmp = "IN";
            break;
        case INF:
            tmp = "INF";
            break;
        default:
            tmp = "no";
            break;
        }
        fprintf(out_statistic, "\t%s :\t%lld\n", tmp.c_str(), pair.second);
    }

    fclose(out_statistic);
}

// controller.cpp:
// execute instructions

#include "controller.h"
#include "asm.h"
#include "global.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <string>
using namespace std;

controller::controller(const char *fname, loader *l, memory *m, reg r[],
                       freg fr[], Log *l_level) {
    ld = l;
    memo = m;
    regs = r;
    fregs = fr;
    log_level = l_level;

    line_num = 0;

    regs[0].data = 0;
    regs[29].data = memorySize - 4; // init sp;

    // for output
    if (ld->output_exist) {
        string outputfile_name = fname;
        outputfile_name.pop_back(); // 最後のsを削除
        outputfile_name = outputfile_name + "out";
        outputfile = fopen(outputfile_name.c_str(), "w");
        if (outputfile == NULL) { // オープンに失敗した場合
            printf("cannot open output file: %s\n", outputfile_name.c_str());
            exit(1);
        }
    }

    if (ld->input_exist) {
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

Status controller::exec_step(int break_point) {

    vector<int> line_vec = ld->get_program_by_line_num(line_num);

    if (*log_level >= INFO) {
        // raw_programを出力
        string one_raw_program = ld->get_raw_program_by_line_num(line_num);
        printf("INFO\t%d:\t%s", line_num * 4, one_raw_program.c_str());
        if (*log_level >= TRACE) {
            printf("\t(TRACE\t");
            for (auto itr_str = line_vec.begin(); itr_str != line_vec.end();
                 ++itr_str) {
                printf("%d\t", *itr_str);
            }
            printf(")");
        }

        printf("\n");
    }

    exec_code(line_vec);

    if (line_num == break_point) {
        return BREAK;
    } else if (line_num >= ld->end_line_num) {
        return END;
    }

    return ACTIVE;
}

void controller::exec_code(vector<int> line_vec) {
    auto iter = line_vec.begin();
    int opecode = *iter;
    int rd;
    int rs;
    int rt;
    int immediate;
    int sb;
    int reg;
    int offset;
    int addr;
    unsigned int mask;
    int base;
    int label_line;

    iter++;

    switch (opecode) {
    case ADD: {
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d + rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }

        regs[rd].data = regs[rs].data + regs[rt].data;

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case ADDI: // ADDI rd <- rs + immediate
    {
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        immediate = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d + immediate:%d\n", rd, rs,
                   regs[rs].data, immediate);
        }

        regs[rd].data = regs[rs].data + immediate;

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case SUB: { // SUB rd <- rs - rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d - rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data - regs[rt].data;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case MUL: { // MUL rd <- rs * rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d * rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data * regs[rt].data;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case DIV: { // DIV rd <- rs / rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d / rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data / regs[rt].data;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }
        line_num++;
        break;
    }
    case MOD: { // MOD rd <- rs % rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d MOD rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data % regs[rt].data;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }
        line_num++;
        break;
    }
    case SLT: { // SLT Rd = if Rs < Rt then 1 else 0
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- if (rs($%d):%d < rt($%d):%d) then 1 else 0\n",
                   rd, rs, regs[rs].data, rt, regs[rt].data);
        }
        if (regs[rs].data < regs[rt].data) {
            regs[rd].data = 1;
        } else {
            regs[rd].data = 0;
        }
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }
    case AND: { // AND rd <- rs & rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d & rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data & regs[rt].data;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }
        line_num++;
        break;
    }

    case ANDI: { // ANDI rd <- rs & immediate
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        immediate = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d & zero_extend(immediate):%d\n", rd,
                   rs, regs[rs].data, immediate);
        }

        regs[rd].data = regs[rs].data & (immediate & 0xffff);

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }
    case OR: { // OR rd <- rs | rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d | rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data | regs[rt].data;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case ORI: { // ORI rd <- rs & immediate
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        immediate = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d | zero_extend(immediate):%d\n", rd,
                   rs, regs[rs].data, immediate);
        }
        regs[rd].data = regs[rs].data | (immediate & 0xffff);
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }
    case NOR: { // NOR rd <- ~(rs | rt)
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- ~(rs($%d):%d | rt($%d):%d)\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = ~(regs[rs].data | regs[rt].data);
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case XOR: { // XOR rd <- rs ^ rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d ^ rt($%d):%d\n", rd, rs,
                   regs[rs].data, rt, regs[rt].data);
        }
        regs[rd].data = regs[rs].data ^ regs[rt].data;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case XORI: { // XORI rd <- rs & immediate
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        immediate = *iter;

        regs[rd].data = regs[rs].data ^ immediate;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d ^ zero_extend(immediate):%d\n", rd,
                   rs, regs[rs].data, immediate);
        }
        regs[rd].data = regs[rs].data ^ (immediate & 0xffff);
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case SRAI: { // SRAI rd <- rs >> sb (arithmetic)
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        sb = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d >> sb:%d (arithmetic)\n", rd, rs,
                   regs[rs].data, sb);
        }
        regs[rd].data = regs[rs].data >> sb;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case SRLI: { // SRLI rd <- rs >> sb (logical)
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        sb = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d >> sb:%d (logical)\n", rd, rs,
                   regs[rs].data, sb);
        }

        regs[rd].data = (int)((unsigned int)regs[rs].data >> (unsigned int)sb);

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case SLLI: { // SLLI rd <- rs << sb (logical)
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        sb = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d << sb:%d (logical)\n", rd, rs,
                   regs[rs].data, sb);
        }

        regs[rd].data = (int)((unsigned int)regs[rs].data << (unsigned int)sb);

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case SRA: { // SRA rd <- rs >> rt (arithmetic)
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d >> rt($%d):%d [4:0] (arithmetic)\n",
                   rd, rs, regs[rs].data, rt, regs[rt].data);
        }
        mask = 0x1F; // 下位5ビットの取り出し
        regs[rd].data = regs[rs].data >> (mask & regs[rt].data);
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }
    case SRL: { // SRL rd <- rs >> rt (logical)
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d >> rt($%d):%d [4:0] (logical)\n",
                   rd, rs, regs[rs].data, rt, regs[rt].data);
        }
        mask = 0x1F; // 下位5ビットの取り出し
        regs[rd].data = (int)((unsigned int)regs[rs].data >>
                              (mask & (unsigned int)regs[rt].data));

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }
    case SLL: { // SLL rd <- rs << rt (logical)
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d << rt($%d):%d [4:0] (logical)\n",
                   rd, rs, regs[rs].data, rt, regs[rt].data);
        }
        mask = 0x1F; // 下位5ビットの取り出し
        regs[rd].data = (int)((unsigned int)regs[rs].data
                              << (mask & (unsigned int)regs[rt].data));

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case FADD: { // FADD rd <- rs +. rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
            printf("\trd($f%d) <- rs($f%d):%f +. rt($f%d):%f\n", rd, rs,
                   fregs[rs].data.f, rt, fregs[rt].data.f);
        }

        fregs[rd].data.f = fregs[rs].data.f + fregs[rt].data.f;

        if (*log_level >= DEBUG) {
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
        }

        line_num++;
        break;
    }

    case FSUB: { // FSUB rd <- rs -. rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
            printf("\trd($f%d) <- rs($f%d):%f -. rt($f%d):%f\n", rd, rs,
                   fregs[rs].data.f, rt, fregs[rt].data.f);
        }

        fregs[rd].data.f = fregs[rs].data.f - fregs[rt].data.f;

        if (*log_level >= DEBUG) {
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
        }

        line_num++;
        break;
    }

    case FMUL: { // FMUL rd <- rs *. rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
            printf("\trd($f%d) <- rs($f%d):%f *. rt($f%d):%f\n", rd, rs,
                   fregs[rs].data.f, rt, fregs[rt].data.f);
        }

        fregs[rd].data.f = fregs[rs].data.f * fregs[rt].data.f;

        if (*log_level >= DEBUG) {
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
        }

        line_num++;
        break;
    }
    case FDIV: { // FDIV rd <- rs /. rt
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
            printf("\trd($f%d) <- rs($f%d):%f /. rt($f%d):%f\n", rd, rs,
                   fregs[rs].data.f, rt, fregs[rt].data.f);
        }

        fregs[rd].data.f = fregs[rs].data.f / fregs[rt].data.f;

        if (*log_level >= DEBUG) {
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
        }

        line_num++;
        break;
    }
    case FNEG: { // FNEG rd <- -rs
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
            printf("\trd($f%d) <- neg(rs($f%d):%f)\n", rd, rs,
                   fregs[rs].data.f);
        }
        fregs[rd].data.f = -fregs[rs].data.f;

        if (*log_level >= DEBUG) {
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
        }

        line_num++;
        break;
    }

    case SQRT: { // SQRT rd <- sqrt(rs)
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
            printf("\trd($f%d) <- sqrt(rs($f%d):%f)\n", rd, rs,
                   fregs[rs].data.f);
        }
        fregs[rd].data.f = sqrt(fregs[rs].data.f);

        if (*log_level >= DEBUG) {
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
        }

        line_num++;
        break;
    }

    case SLTF: { // SLTF Rd[0] = if Rs < Rt then 1 else 0
        // * rd is a general register
        rd = *iter;
        iter++;
        rs = *iter;
        iter++;
        rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($f%d)[0] <- if (rs($f%d):%f < rt($f%d):%f) then 1 "
                   "else 0\n",
                   rd, rs, fregs[rs].data.f, rt, fregs[rt].data.f);
        }
        if (fregs[rs].data.f < fregs[rt].data.f) {
            regs[rd].data = 1;
        } else {
            regs[rd].data = 0;
        }
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case LW: { // LW rd, offset(base)
        rd = *iter;
        iter++;
        reg = *iter;
        iter++;
        offset = *iter;

        addr = regs[reg].data + offset;
        if (!(0 <= addr && addr < memorySize && addr % 4 == 0)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n",
                   one_raw_program.c_str(), line_num * 4, addr);
            exit(1);
        }
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- memory[%d]:%d\n", rd, addr,
                   memo->read_word(addr));
        }
        regs[rd].data = memo->read_word(addr);

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }

    case LB: { // LB rd, offset(base)
        rd = *iter;
        iter++;
        reg = *iter;
        iter++;
        offset = *iter;

        addr = regs[reg].data + offset;
        if (!(0 <= addr && addr < memorySize)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n",
                   one_raw_program.c_str(), line_num * 4, addr);
            exit(1);
        }
        unsigned char data = memo->read_byte(addr);

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- memory[%d]:%hhu\n", rd, addr, data);
        }
        regs[rd].data = (regs[rd].data & 0xffffff00) | (unsigned int)data;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }
        line_num++;
        break;
    }
    case SW: {
        rd = *iter;
        iter++;
        reg = *iter;
        iter++;
        offset = *iter;

        addr = regs[reg].data + offset;
        if (!(0 <= addr && addr < memorySize && addr % 4 == 0)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n",
                   one_raw_program.c_str(), line_num * 4, addr);
            exit(1);
        }

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tmemory[%d]:%d\n", addr, memo->read_word(addr));
            printf("\tmemory[%d] <- rd($%d):%d\n", addr, rd, regs[rd].data);
        }
        memo->write_word(addr, regs[rd].data);
        if (*log_level >= DEBUG) {
            printf("\tmemory[%d]:%d\n", addr, memo->read_word(addr));
        }

        line_num++;
        break;
    }

    case SB: { // SB rd, offset(base)
        rd = *iter;
        iter++;
        reg = *iter;
        iter++;
        offset = *iter;

        addr = regs[reg].data + offset;
        if (!(0 <= addr && addr < memorySize)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n",
                   one_raw_program.c_str(), line_num * 4, addr);
            exit(1);
        }
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tmemory[%d]:%d\n", addr, memo->read_byte(addr));
            printf("\tmemory[%d] <- rd($%d):%d\n", addr, rd, regs[rd].data);
        }
        memo->write_byte(addr,
                         (unsigned char)((regs[rd].data << 8 * 3) >> 8 * 3));
        if (*log_level >= DEBUG) {
            printf("\tmemory[%d]:%d\n", addr, memo->read_byte(addr));
        }

        line_num++;
        break;
    }
    case MOV: { // MOV rd <- rs
        rd = *iter;
        iter++;
        rs = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d\n", rd, rs, regs[rs].data);
        }
        regs[rd].data = regs[rs].data;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;
        break;
    }
    case LF: { // LF rd, offset(base)
        rd = *iter;
        iter++;
        base = *iter;
        iter++;
        offset = *iter;

        addr = regs[base].data + offset;

        if (!(0 <= addr && addr < memorySize && addr % 4 == 0)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n",
                   one_raw_program.c_str(), line_num * 4, addr);
            exit(1);
        }

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
            printf("\trd($f%d) <- memory[%d]:(hex)%8x\n", rd, addr,
                   memo->read_word(addr));
        }
        fregs[rd].data.i = memo->read_word(addr);

        if (*log_level >= DEBUG) {
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
        }

        line_num++;
        break;
    }
    case SF: {
        rd = *iter;
        iter++;
        base = *iter;
        iter++;
        offset = *iter;

        addr = regs[base].data + offset;
        if (!(0 <= addr && addr < memorySize && addr % 4 == 0)) {
            string one_raw_program = ld->get_raw_program_by_line_num(line_num);
            printf("FATAL\n\t%s\n\tprogram address:%d  invalid read address: "
                   "[%d]\n",
                   one_raw_program.c_str(), line_num * 4, addr);
            exit(1);
        }

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tmemory[%d]:(hex)%8x\n", addr, memo->read_word(addr));
            printf("\tmemory[%d] <- rd($f%d):%f\n", addr, rd, fregs[rd].data.f);
        }
        memo->write_word(addr, fregs[rd].data.i);
        if (*log_level >= DEBUG) {
            printf("\tmemory[%d]:(hex)%8x\n", addr, memo->read_word(addr));
        }

        line_num++;
        break;
    }
    case MOVF: { // MOVF rd <- rs
        rd = *iter;
        iter++;
        rs = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
            printf("\trd($f%d) <- rs($f%d):%f\n", rd, rs, fregs[rs].data.f);
        }
        fregs[rd].data.f = fregs[rs].data.f;
        if (*log_level >= DEBUG) {
            printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
        }

        line_num++;
        break;
    }

    case BC: { // BC label(pc+offset<<2)
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter:%d \n", line_num * 4);
            printf("\tprogram counter <- pc:%d + (offset:%d <<2) \n",
                   line_num * 4, *iter);
        }
        line_num = line_num + *iter;
        if (*log_level >= DEBUG) {
            printf("\tprogram counter:%d \n", line_num * 4);
        }
        break;
    }

    case BEQ: { // BEQ rs rt label(pc+offset<<2)
        rs = *iter;
        iter++;
        rt = *iter;
        iter++;
        label_line = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter <- if (rs($%d):%d == rt($%d):%d) then "
                   "pc+(offset:%d <<2) "
                   "else "
                   "pc+4\n",
                   rs, regs[rs].data, rt, regs[rt].data, label_line);
        }

        if (regs[rs].data == regs[rt].data) {
            line_num = line_num + *iter;
        } else {
            line_num++;
        }

        if (*log_level >= DEBUG) {
            printf("\tprogram counter:%d\n", line_num * 4);
        }
        break;
    }

    case BNE: { // BNE rs rt label(pc+offset<<2)
        rs = *iter;
        iter++;
        rt = *iter;
        iter++;
        label_line = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter <- if (rs($%d):%d != rt($%d):%d) then "
                   "pc+(offset:%d <<2) "
                   "else "
                   "pc+4\n",
                   rs, regs[rs].data, rt, regs[rt].data, label_line);
        }

        if (regs[rs].data != regs[rt].data) {
            line_num = line_num + *iter;
        } else {
            line_num++;
        }
        if (*log_level >= DEBUG) {
            printf("\tprogram counter:%d\n", line_num * 4);
        }
        break;
    }

    case J: { // J label
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter:%d \n", line_num * 4);
            printf("\tprogram counter <- address:%d <<2 \n", *iter);
        }
        line_num = *iter;
        if (*log_level >= DEBUG) {
            printf("\tprogram counter:%d \n", line_num * 4);
        }
        break;
    }

    case JR: { // JR rs
        rs = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter:%d \n", line_num * 4);
            printf("\tprogram counter <- rs($%d):%d\n", rs, regs[rs].data);
        }
        line_num = regs[rs].data / 4; // convert program addr to line number;
        if (*log_level >= DEBUG) {
            printf("\tprogram counter:%d \n", line_num * 4);
        }
        break;
    }

    case JAL: { // JAL label (next addr is line_num*4)

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\t$31:%d\tprogram counter:%d\n", regs[31].data,
                   line_num * 4);
            printf("\t$31 <- pc+4\tprogram counter <- address:%d <<2\n", *iter);
        }
        regs[31].data = line_num * 4 + 4;
        line_num = *iter;
        if (*log_level >= DEBUG) {
            printf("\t$31:%d\n", regs[31].data);
            printf("\tprogram counter:%d \n", line_num * 4);
        }
        break;
    }

    case JALR: { // JALR rd, rs
        rd = *iter;
        iter++;
        rs = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\tprogram counter:%d\n", rd, regs[rd].data,
                   line_num * 4);
            printf("\trd($%d) <- pc+4\tprogram counter <- rs($%d):%d\n", rd, rs,
                   regs[rs].data);
        }
        regs[rd].data = line_num * 4 + 4;
        line_num = regs[rs].data / 4;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\tprogram counter:%d\trd($%d):%d\n", line_num * 4, rd,
                   regs[rd].data);
        }
        break;
    }

    case INB: { // INB rd
        rd = *iter;
        char str;
        if (!ifs.get(str)) { // オープンに失敗した場合
            printf("input file was not opened\n");
            // exit(1);
        } else {
            if (ifs.eof()) {
                string one_raw_program =
                    ld->get_raw_program_by_line_num(line_num);
                printf("FATAL\n\tread EOF! program address:%d\n\t%s\n",
                       line_num * 4, one_raw_program.c_str());
                exit(1);
            }

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):(hex)%08x <- get(char):%c\n", rd,
                       regs[rd].data, str);
            }

            regs[rd].data = (int)((unsigned char)str);
            if (*log_level >= DEBUG) {
                printf("\trd($%d):(hex)%08x\n", rd, regs[rd].data);
            }
        }

        line_num++;
        break;
    }

    case IN: { // IN rd
        rd = *iter;
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

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\tIN rd($%d):%d <- get(int):%d\n", rd, regs[rd].data,
                       tmp);
            }

            regs[rd].data = tmp;
            if (*log_level >= DEBUG) {
                printf("\trd($%d):%d\n", rd, regs[rd].data);
            }
        }
        line_num++;
        break;
    }
    case OUTB: { // outb rs
        rs = *iter;
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tOUTB rs($%d):(hex)%08x\n", rs, regs[rs].data);
        }
        char lower8 = (char)(((unsigned int)regs[rs].data) & 0xff);
        fprintf(outputfile, "%c", lower8);

        if (*log_level >= DEBUG) {
            printf("\tout(char):%c\n", lower8);
        }
        line_num++;
        break;
    }

    case OUT: { // out rs
        rs = *iter;
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tOUT rs($%d):%d\n", rs, regs[rs].data);
        }
        fprintf(outputfile, "%d", regs[rs].data);
        if (*log_level >= DEBUG) {
            printf("\tout(int):%d\n", regs[rs].data);
        }
        line_num++;
        break;
    }

    case INF: { // INF rd
        rd = *iter;
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

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\tINF rd($f%d):%f <- get(float):%f\n", rd,
                       fregs[rd].data.f, tmp.f);
            }

            fregs[rd].data.f = tmp.f;
            if (*log_level >= DEBUG) {
                printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
            }
        }
        line_num++;
        break;
    }
    case OUTF: { // OUTF rs
        rs = *iter;
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tOUTF rs($%d):%f\n", rs, fregs[rs].data.f);
        }
        fprintf(outputfile, "%f", fregs[rs].data.f);
        if (*log_level >= DEBUG) {
            printf("\tout(float):%f\n", fregs[rs].data.f);
        }
        line_num++;
        break;
    }

    case NOP: { // nop
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tNOP\n");
        }
        line_num++;
        break;
    }

    default: { line_num++; }
    }
}

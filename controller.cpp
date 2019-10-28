// controller.cpp:
// execute instructions

#include "controller.h"
#include "asm.h"
#include "global.h"
#include "print.h"
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
        outputfile_name = outputfile_name + "ppm";
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

    unsigned int one_code = ld->get_machine_code_by_line_num(line_num);

    if (*log_level >= INFO) {
        // raw_programを出力
        string one_raw_program = ld->get_raw_program_by_line_num(line_num);
        printf("INFO\t%d:\t%s", line_num * 4, one_raw_program.c_str());
        if (*log_level >= TRACE) {
            printf("\t(TRACE\t");
            print_binary_with_space(one_code);
            printf(")");
        }

        printf("\n");
    }

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

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf("\trd($%d) <- rs($%d):%d << sb:%d (logical)\n", rd, rs,
                       regs[rs].data, sb);
            }

            regs[rd].data =
                (int)((unsigned int)regs[rs].data << (unsigned int)sb);

            if (*log_level >= DEBUG) {
                printf("\trd($%d):%d\n", rd, regs[rd].data);
            }

            line_num++;
            break;
        }

        // SRLI
        case 2: { // SRLI rd <- rs >> sb (logical)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            sb = (one_code & shamt_mask) >> 6;

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf("\trd($%d) <- rs($%d):%d >> sb:%d (logical)\n", rd, rs,
                       regs[rs].data, sb);
            }

            regs[rd].data =
                (int)((unsigned int)regs[rs].data >> (unsigned int)sb);

            if (*log_level >= DEBUG) {
                printf("\trd($%d):%d\n", rd, regs[rd].data);
            }

            line_num++;
            break;
        }

        // SRAI
        case 3: { // SRAI rd <- rs >> sb (arithmetic)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            sb = (one_code & shamt_mask) >> 6;

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf("\trd($%d) <- rs($%d):%d >> sb:%d (arithmetic)\n", rd,
                       rs, regs[rs].data, sb);
            }
            regs[rd].data = regs[rs].data >> sb;
            if (*log_level >= DEBUG) {
                printf("\trd($%d):%d\n", rd, regs[rd].data);
            }

            line_num++;
            break;
        }

        // SLL
        case 4: { // SLL rd <- rs << rt (logical)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf(
                    "\trd($%d) <- rs($%d):%d << rt($%d):%d [4:0] (logical)\n",
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

        // SRL
        case 6: { // SRL rd <- rs >> rt (logical)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf(
                    "\trd($%d) <- rs($%d):%d >> rt($%d):%d [4:0] (logical)\n",
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

        // SRA
        case 7: { // SRA rd <- rs >> rt (arithmetic)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf("\trd($%d) <- rs($%d):%d >> rt($%d):%d [4:0] "
                       "(arithmetic)\n",
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

        // JR or JALR
        case 9: { // JALR rd, rs
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\tprogram counter:%d\n", rd, regs[rd].data,
                       line_num * 4);
                printf("\trd($%d) <- pc+4\tprogram counter <- rs($%d):%d\n", rd,
                       rs, regs[rs].data);
            }
            if (rd != 0) { // 0レジスタには書き込みしない
                regs[rd].data = line_num * 4 + 4;
            }
            line_num = regs[rs].data / 4;
            if (*log_level >= DEBUG) {
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf("\tprogram counter:%d\trd($%d):%d\n", line_num * 4, rd,
                       regs[rd].data);
            }
            break;
        }

        // MUL
        case 24: { // MUL rd <- rs * rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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

        // DIV or MOD
        case 26: {
            if (shamt == 0x2) { // DIV rd <- rs / rt

                rd = (one_code & rd_mask) >> 21;
                rs = (one_code & rs_mask) >> 16;
                rt = (one_code & rt_mask) >> 11;

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

            } else if (shamt == 0x3) { // MOD rd <- rs % rt

                rd = (one_code & rd_mask) >> 21;
                rs = (one_code & rs_mask) >> 16;
                rt = (one_code & rt_mask) >> 11;

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
        }

        // ADD or MOV
        case 32: {
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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

        // SUB
        case 34: { // SUB rd <- rs - rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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

        // AND
        case 36: { // AND rd <- rs & rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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

        // OR
        case 37: { // OR rd <- rs | rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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

        case 38: { // XOR rd <- rs ^ rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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

        case 39: { // NOR rd <- ~(rs | rt)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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
        case 42: { // SLT Rd = if Rs < Rt then 1 else 0
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

            if (*log_level >= DEBUG) {
                printf("DEBUG\n");
                printf("\trd($%d):%d\n", rd, regs[rd].data);
                printf(
                    "\trd($%d) <- if (rs($%d):%d < rt($%d):%d) then 1 else 0\n",
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

        case 1: { // FSUB rd <- rs -. rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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

        case 2: { // FMUL rd <- rs *. rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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
        case 3: { // FDIV rd <- rs /. rt
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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
        case 9: { // FNEG rd <- -rs
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;

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

        case 4: { // SQRT rd <- sqrt(rs)
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;

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

        case 8: { // SLTF Rd[0] = if Rs < Rt then 1 else 0
            // * rd is a general register
            rd = (one_code & rd_mask) >> 21;
            rs = (one_code & rs_mask) >> 16;
            rt = (one_code & rt_mask) >> 11;

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
            /*
            case 0:{// MOVF rd <- rs
                rd = (one_code & rd_mask) >> 21;
                rs = (one_code & rs_mask) >> 16;

                if (*log_level >= DEBUG) {
                    printf("DEBUG\n");
                    printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
                    printf("\trd($f%d) <- rs($f%d):%f\n", rd, rs,
            fregs[rs].data.f);
                }
                fregs[rd].data.f = fregs[rs].data.f;
                if (*log_level >= DEBUG) {
                    printf("\trd($f%d):%f\n", rd, fregs[rd].data.f);
                }

                line_num++;
                break;
            }
            */
        }
        break;
    }

    case 0x2: { // J label
        label_line = (one_code & address_mask);
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter:%d \n", line_num * 4);
            printf("\tprogram counter <- address:%d <<2 \n", label_line);
        }
        line_num = label_line;
        if (*log_level >= DEBUG) {
            printf("\tprogram counter:%d \n", line_num * 4);
        }
        break;
    }

    case 0x3: { // JAL label (next addr is line_num*4)
        label_line = (one_code & address_mask);
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\t$31:%d\tprogram counter:%d\n", regs[31].data,
                   line_num * 4);
            printf("\t$31 <- pc+4\tprogram counter <- address:%d <<2\n",
                   label_line);
        }
        regs[31].data = line_num * 4 + 4;
        line_num = label_line;
        if (*log_level >= DEBUG) {
            printf("\t$31:%d\n", regs[31].data);
            printf("\tprogram counter:%d \n", line_num * 4);
        }
        break;
    }

    case 4: { // BEQ rs rt label(pc+offset<<2)
        rs = (one_code & rd_mask) >> 21;
        rt = (one_code & rs_mask) >> 16;
        label_line = (one_code & addr_or_imm_mask);
        if ((label_line & 0x8000) == 0x8000) { //符号拡張
            label_line = 0xffff0000 | label_line;
        }
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter <- if (rs($%d):%d == rt($%d):%d) then "
                   "pc+(offset:%d <<2) "
                   "else "
                   "pc+4\n",
                   rs, regs[rs].data, rt, regs[rt].data, label_line);
        }

        if (regs[rs].data == regs[rt].data) {
            line_num = line_num + label_line;
        } else {
            line_num++;
        }

        if (*log_level >= DEBUG) {
            printf("\tprogram counter:%d\n", line_num * 4);
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
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter <- if (rs($%d):%d != rt($%d):%d) then "
                   "pc+(offset:%d <<2) "
                   "else "
                   "pc+4\n",
                   rs, regs[rs].data, rt, regs[rt].data, label_line);
        }

        if (regs[rs].data != regs[rt].data) {
            line_num = line_num + label_line;
        } else {
            line_num++;
        }
        if (*log_level >= DEBUG) {
            printf("\tprogram counter:%d\n", line_num * 4);
        }
        break;
    }

    case 8: // ADDI rd <- rs + immediate
    {
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
        if ((immediate & 0x8000) == 0x8000) { //符号拡張
            immediate = 0xffff0000 | immediate;
        }

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

    case 12: { // ANDI rd <- rs & immediate
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
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

    case 13: { // ORI rd <- rs & immediate
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
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
    case 14: { // XORI rd <- rs & immediate
        rd = (one_code & rd_mask) >> 21;
        rs = (one_code & rs_mask) >> 16;
        immediate = (one_code & addr_or_imm_mask);
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

    case 32: { // LB rd, offset(base)
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);

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

    case 35: { // LW rd, offset(base)
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);

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

    case 40: { // SB rd, offset(base)
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);

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

    case 43: { // SW
        rd = (one_code & rd_mask) >> 21;
        reg = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);

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

    case 49: { // LF rd, offset(base)
        rd = (one_code & rd_mask) >> 21;
        base = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);

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

    case 50: { // BC label(pc+offset<<2)
        offset = (one_code & address_mask);
        if ((offset & 0x2000000) == 0x2000000) { //符号拡張
            offset = 0xfc000000 | immediate;
        }
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tprogram counter:%d \n", line_num * 4);
            printf("\tprogram counter <- pc:%d + (offset:%d <<2) \n",
                   line_num * 4, offset);
        }
        line_num = line_num + offset;
        if (*log_level >= DEBUG) {
            printf("\tprogram counter:%d \n", line_num * 4);
        }
        break;
    }

    case 57: { // SF
        rd = (one_code & rd_mask) >> 21;
        base = (one_code & rs_mask) >> 16;
        offset = (one_code & addr_or_imm_mask);

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

            case 1: { // outb rs
                rs = (one_code & rs_mask) >> 16;
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
                            ld->get_raw_program_by_line_num(line_num);
                        printf("FATAL\n\tread EOF! program address:%d\n\t%s\n",
                               line_num * 4, one_raw_program.c_str());
                        exit(1);
                    }

                    if (*log_level >= DEBUG) {
                        printf("DEBUG\n");
                        printf("\tIN rd($%d):%d <- get(int):%d\n", rd,
                               regs[rd].data, tmp);
                    }

                    regs[rd].data = tmp;
                    if (*log_level >= DEBUG) {
                        printf("\trd($%d):%d\n", rd, regs[rd].data);
                    }
                }
                line_num++;
                break;
            }

            case 1: { // out rs
                rs = (one_code & rs_mask) >> 16;
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

            case 2: { // INF rd
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

            case 3: { // OUTF rs
                rs = (one_code & rs_mask) >> 16;
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
            }
            break;
        }
        }

        break;
    }

    default: {
        if (*log_level >= FATAL) {
            printf("FATAL invalid instructions:");
            print_binary_with_space(one_code);
            printf("\n");
        }
    }
    }
}

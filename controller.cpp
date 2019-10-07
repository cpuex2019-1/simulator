// controller.cpp:
// execute instructions

#include "controller.h"
#include "asm.h"
#include "global.h"
#include <regex>
#include <stdio.h>
#include <string>
using namespace std;

controller::controller(loader *l, memory *m, reg r[], Log *l_level) {
    ld = l;
    memo = m;
    regs = r;
    log_level = l_level;
    line_num = 1; // (line_num-1)*4 is addr for instruction?
    regs[0].data = 0;
    regs[29].data = memorySize - 4; // init sp;
}

Status controller::exec_step(int break_point) {

    vector<int> line_vec = ld->get_program_by_line_num(line_num);

    if (*log_level >= INFO) {
        // raw_programを出力
        string one_raw_program = ld->get_raw_program_by_line_num(line_num);
        printf("INFO\t%d:\t%s", line_num, one_raw_program.c_str());
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
    iter++;

    if (opecode == ADD) { // ADD rd <- rs + rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == ADDI) { // ADDI rd <- rs + immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int immediate = *iter;

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

    } else if (opecode == SUB) { // SUB rd <- rs - rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == MUL) { // MUL rd <- rs * rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == DIV) { // DIV rd <- rs / rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == MOD) { // MOD rd <- rs % rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == SLT) { // SLT Rd = if Rs < Rt then 1 else 0
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == AND) { // AND rd <- rs & rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == ANDI) { // ANDI rd <- rs & immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int immediate = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d & immediate:%d\n", rd, rs,
                   regs[rs].data, immediate);
        }

        regs[rd].data = regs[rs].data & immediate;

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;

    } else if (opecode == OR) { // OR rd <- rs | rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == ORI) { // ORI rd <- rs & immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int immediate = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d | immediate:%d\n", rd, rs,
                   regs[rs].data, immediate);
        }
        regs[rd].data = regs[rs].data | immediate;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;

    } else if (opecode == NOR) { // NOR rd <- ~(rs | rt)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == XOR) { // XOR rd <- rs ^ rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

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

    } else if (opecode == XORI) { // XORI rd <- rs & immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int immediate = *iter;

        regs[rd].data = regs[rs].data ^ immediate;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d ^ immediate:%d\n", rd, rs,
                   regs[rs].data, immediate);
        }
        regs[rd].data = regs[rs].data ^ immediate;
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;

    } else if (opecode == SRAI) { // SRAI rd <- rs >> sb (arithmetic)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int sb = *iter;

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

    } else if (opecode == SRLI) { // SRLI rd <- rs >> sb (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int sb = *iter;

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

    } else if (opecode == SLLI) { // SLLI rd <- rs << sb (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int sb = *iter;

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

    } else if (opecode == SRA) { // SRA rd <- rs >> rt (arithmetic)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d >> rt($%d):%d [4:0] (arithmetic)\n",
                   rd, rs, regs[rs].data, rt, regs[rt].data);
        }
        unsigned int mask = 0x1F; // 下位5ビットの取り出し
        regs[rd].data = regs[rs].data >> (mask & regs[rt].data);
        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;

    } else if (opecode == SRL) { // SRL rd <- rs >> rt (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d >> rt($%d):%d [4:0] (logical)\n",
                   rd, rs, regs[rs].data, rt, regs[rt].data);
        }
        unsigned int mask = 0x1F; // 下位5ビットの取り出し
        regs[rd].data = (int)((unsigned int)regs[rs].data >>
                              (mask & (unsigned int)regs[rt].data));

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;

    } else if (opecode == SLL) { // SLL rd <- rs << rt (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\trd($%d) <- rs($%d):%d << rt($%d):%d [4:0] (logical)\n",
                   rd, rs, regs[rs].data, rt, regs[rt].data);
        }
        unsigned int mask = 0x1F; // 下位5ビットの取り出し
        regs[rd].data = (int)((unsigned int)regs[rs].data
                              << (mask & (unsigned int)regs[rt].data));

        if (*log_level >= DEBUG) {
            printf("\trd($%d):%d\n", rd, regs[rd].data);
        }

        line_num++;

    } else if (opecode == LW) { // LW rd, offset(base)
        int rd = *iter;
        iter++;
        int reg = *iter;
        iter++;
        int offset = *iter;

        int addr = regs[reg].data + offset;

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

    } else if (opecode == LB) { // LB rd, offset(base)
        int rd = *iter;
        iter++;
        int reg = *iter;
        iter++;
        int offset = *iter;

        int addr = regs[reg].data + offset;
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

    } else if (opecode == SW) {
        int rd = *iter;
        iter++;
        int reg = *iter;
        iter++;
        int offset = *iter;

        int addr = regs[reg].data + offset;

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

    } else if (opecode == SB) { // SB rd, offset(base)
        int rd = *iter;
        iter++;
        int reg = *iter;
        iter++;
        int offset = *iter;

        int addr = regs[reg].data + offset;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tmemory[%d]:%d\n", addr, memo->read_byte(addr));
            printf("\tmemory[%d] <- rd($%d):%d\n", addr, rd, regs[reg].data);
        }
        memo->write_byte(addr,
                         (unsigned char)((regs[rd].data << 8 * 3) >> 8 * 3));
        if (*log_level >= DEBUG) {
            printf("\tmemory[%d]:%d\n", addr, memo->read_byte(addr));
        }

        line_num++;

    } else if (opecode == MOV) { // MOV rd <- rs
        int rd = *iter;
        iter++;
        int rs = *iter;

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

    } else if (opecode == BC) { // BC label(pc+offset<<2)
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tline_num:%d to %d\n", line_num, *iter);
        }
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

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tline_num <- if (rs($%d):%d == rt($%d):%d) then %d else "
                   "next_line\n",
                   rs, regs[rs].data, rt, regs[rt].data, *iter);
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

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tline_num <- if (rs($%d):%d != rt($%d):%d) then %d else "
                   "next_line\n",
                   rs, regs[rs].data, rt, regs[rt].data, *iter);
        }

    } else if (opecode == J) { // J label
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tline_num:%d to %d\n", line_num, *iter);
        }
        line_num = *iter;

    } else if (opecode == JR) { // JR rs
        int rs = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trs($%d):%d\n", rs, regs[rs].data);
            printf("\tline_num:%d to %d(rs/4+1)\n", line_num,
                   ((regs[rs].data / 4) + 1));
        }
        line_num =
            (regs[rs].data / 4) + 1; // convert program addr to line number;

    } else if (opecode == JAL) { // JAL label (next addr is line_num*4)

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\t$31:%d\n", regs[31].data);
            printf("\t$31 <- return addr:%d\n", line_num * 4);
        }
        regs[31].data = line_num * 4;
        if (*log_level >= DEBUG) {
            printf("\tline_num:%d to %d\n", line_num, *iter);
        }
        line_num = *iter;

    } else if (opecode == JALR) { // JALR rd, rs
        int rd = *iter;
        iter++;
        int rs = *iter;

        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\trd($%d):%d\n", rd, regs[rd].data);
            printf("\tline_num <- (rs($%d):%d)*4+1 \trd($%d) <- return addr: "
                   "%d\n ",
                   rs, regs[rs].data, rd, line_num * 4);
        }
        regs[rd].data = line_num * 4;
        if (*log_level >= DEBUG) {
            printf("\tline_num:%d to %d\trd($%d):%d\n", line_num,
                   (regs[rs].data / 4 + 1), rd, regs[rd].data);
        }
        line_num = (regs[rs].data / 4 + 1);

    } else if (opecode == NOP) { // nop
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tNOP\n");
        }
        line_num++;

    } else if (opecode == OUT) { // output 未対応
        if (*log_level >= DEBUG) {
            printf("DEBUG\n");
            printf("\tOUT\n");
        }
        line_num++;

    } else {
        line_num++;
    }
}

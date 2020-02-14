// memory.cpp:
// (Big endian)

#include "memory.h"
#include "global.h"
#include "print.h"
#include <bitset>
#include <stdio.h>
using namespace std;

// constructor
memory::memory() {
    check_memory = false;
    heap_pointer = 3000;
    for (sim_addr i = 0; i < memorySize / 4; i++) {
        table[i] = 0x0;
        used[i] = NON; // 0:未使用 1:int 2:float
    }
}

// memory::~memory() {}

sim_byte memory::read_byte(sim_addr addr) {
    int offset = addr % 4;
    return (sim_byte)((table[addr / 4] >> offset * 8) & 0xff);
}

sim_word memory::read_word(sim_addr addr) { return table[addr / 4]; }

void memory::write_byte(sim_addr addr, sim_byte byte_data) {
    int offset = addr % 4;
    table[addr / 4] = (sim_word)((sim_word)byte_data << offset * 8);
    used[addr / 4] = INT;
}

void memory::write_word(sim_addr addr, sim_word word_data, bool is_int) {
    table[addr / 4] = word_data;
    if (is_int) {
        used[addr / 4] = INT;
    } else {
        used[addr / 4] = FLOAT;
    }
}

bool memory::is_int_stored(sim_addr addr) { return used[addr / 4] != FLOAT; }
bool memory::is_float_stored(sim_addr addr) { return used[addr / 4] != INT; }

// print word from s_addr to e_addr
void memory::print_word_by_addr(sim_addr s_addr, sim_addr e_addr) {
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

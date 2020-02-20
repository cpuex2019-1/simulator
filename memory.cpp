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

sim_word memory::read_word(sim_addr addr) { return table[addr]; }

void memory::write_word(sim_addr addr, sim_word word_data, bool is_int) {
    table[addr] = word_data;
    if (is_int) {
        used[addr] = INT;
    } else {
        used[addr] = FLOAT;
    }
}

bool memory::is_int_stored(sim_addr addr) { return used[addr] != FLOAT; }
bool memory::is_float_stored(sim_addr addr) { return used[addr] != INT; }

// print word from s_addr to e_addr
void memory::print_word_by_addr(sim_addr s_addr, sim_addr e_addr) {
    if (s_addr < memorySize / 4 && e_addr <= memorySize / 4 &&
        s_addr <= e_addr) {
        for (sim_addr addr = s_addr; addr <= e_addr; addr++) {
            sim_word word = read_word(addr);
            printf("%9d:\t%9d\t%8x\t", addr, table[addr], table[addr]);
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

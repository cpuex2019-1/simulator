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
    for (sim_addr i = 0; i < memorySize; i++) {
        table[i] = 0x0;
    }
}

// memory::~memory() {}

sim_byte memory::read_byte(sim_addr addr) {
    if (0 <= addr && addr < memorySize) {
        return table[addr];
    } else {
        if (log_level >= FATAL) {
            printf("FATAL\t[Byte] invalid read address: [%d]\n", addr);
        }
        exit(1);
    }
}

sim_word memory::read_word(sim_addr addr) {
    sim_word word = 0x0;
    word = word | (unsigned int)table[addr] << 8 * 3 |
           (unsigned int)table[addr + 1] << 8 * 2 |
           (unsigned int)table[addr + 2] << 8 * 1 |
           (unsigned int)table[addr + 3];
    return word;
}

void memory::write_byte(sim_addr addr, sim_byte byte_data) {
    if (0 <= addr && addr < memorySize) {
        table[addr] = byte_data;
    } else {
        if (log_level >= FATAL) {
            printf("FATAL\t[Byte] invalid write address: [%d]\n", addr);
        }
        exit(1);
    }
}

void memory::write_word(sim_addr addr, sim_word word_data) {
    table[addr] = (unsigned char)(word_data >> 8 * 3);
    table[addr + 1] = (unsigned char)(word_data >> 8 * 2);
    table[addr + 2] = (unsigned char)(word_data >> 8 * 1);
    table[addr + 3] = (unsigned char)(word_data);
}

// print word from s_addr to e_addr
void memory::print_word_by_addr(sim_addr s_addr, sim_addr e_addr) {
    if (s_addr < memorySize && s_addr % 4 == 0 && e_addr + 4 <= memorySize &&
        e_addr % 4 == 0 && s_addr <= e_addr) {
        for (sim_addr addr = s_addr; addr <= e_addr; addr += 4) {
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

// memory.h:
//
#ifndef MEMORY_H
#define MEMORY_H

#include "global.h"

enum IANDF { NON, INT, FLOAT };

class memory {
    sim_word table[memorySize / 4 + 10];
    int used[memorySize / 4 + 10];

  public:
    Log log_level = DEBUG;
    bool check_memory;
    int heap_pointer;
    // constructor
    memory();
    // destructor
    //~memory();

    sim_word read_word(sim_addr addr);
    void write_word(sim_addr addr, sim_word word_data, bool is_int);

    bool is_int_stored(sim_addr addr);
    bool is_float_stored(sim_addr addr);

    // print_memory_word from s_addr to e_addr
    void print_word_by_addr(sim_addr s_addr, sim_addr e_addr);
};

#endif

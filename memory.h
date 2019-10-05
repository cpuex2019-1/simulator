// memory.h:
//
#ifndef MEMORY_H
#define MEMORY_H

#include "global.h"

class memory {
    sim_byte *table;
    Log *log_level;

  public:
    // constructor
    memory(Log *l_level);
    // destructor
    ~memory();

    sim_byte read_byte(sim_addr addr);
    sim_word read_word(sim_addr addr);
    void write_byte(sim_addr addr, sim_byte byte_data);
    void write_word(sim_addr addr, sim_word word_data);

    // print_memory_word from s_addr to e_addr
    void print_word_by_addr(sim_addr s_addr, sim_addr e_addr);
};

#endif

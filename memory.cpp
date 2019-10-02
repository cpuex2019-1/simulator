// memory.cpp:
//

#include <iostream>
#include "memory.h"
#include "global.h"
using namespace std;


//constructor
memory::memory(){
    table = new sim_byte[memorySize];
    for(sim_addr i=0; i<memorySize; i++){
        table[i] = 0x0;
    }
}

memory::~memory(){
    delete table;
}

sim_byte memory::read_byte(sim_addr addr){
    if(0 <= addr && addr < memorySize){
        return table[addr];
    }else{
        cout << "invalid address" << endl;
        exit(1);
    }
}

sim_word memory::read_word(sim_addr addr){
    if(addr < memorySize && addr%4==0){
        sim_word word = 0x0;
        word = word | (unsigned int)table[addr] << 8*3
                    | (unsigned int)table[addr+1] << 8*2
                    | (unsigned int)table[addr+2] << 8*1
                    | (unsigned int)table[addr+3] ;
        return word;
    }else{
        cout << "invalid address" << endl;
        exit(1);
    }
}

void memory::write_byte(sim_addr addr, sim_byte byte_data){
    if(addr < memorySize){
        table[addr] = byte_data;
    }else{
        cout << "invalid address" << endl;
        exit(1);
    }
}

void memory::write_word(sim_addr addr, sim_word word_data){
    if(addr < memorySize && addr%4==0){
        table[addr] = (unsigned char)(word_data >> 8*3);
        table[addr+1] = (unsigned char)(word_data >> 8*2);
        table[addr+2] = (unsigned char)(word_data >> 8*1);
        table[addr+3] = (unsigned char)(word_data);
    }else{
        cout << "invalid address" << endl;
        exit(1);
    }
}

// print word from s_addr to e_addr
void memory::print_word_by_addr(sim_addr s_addr,sim_addr e_addr){
    if(s_addr < memorySize && s_addr%4==0 && e_addr < memorySize && e_addr%4==0 && s_addr < e_addr){
        for(sim_addr addr=s_addr; addr!=e_addr; addr+=4){
            sim_word word = read_word(addr);
            cout << "address: " << addr << "\t" << word << endl;
        }
    }else{
        cout << "invalid address" << endl;
        exit(1);
    }
}

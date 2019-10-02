// controller.h:
// execute instructions
#ifndef EXECUTER_H
#define EXECUTER_H

#include "loader.h"
#include "memory.h"

class controller {
    memory * memo;
    loader * ld;
    // current line number being processed
    int line_num;
public:
    //constructor
    controller(loader* l, memory* m);

    bool exec_step();

};


#endif

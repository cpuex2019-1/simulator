// controller.cpp:
// execute instructions

#include <string>
#include <regex>
#include <iostream>
#include "loader.h"
#include "memory.h"
#include "controller.h"
using namespace std;


controller::controller(loader* l, memory* m){
    ld = l;
    memo = m;
    line_num = 1;
}

bool controller::exec_step(){
    string line_str = ld->get_program_by_line_num(line_num);
    regex sep("[\\t ]+");
    sregex_token_iterator iter(line_str.begin(), line_str.end(), sep, -1);
    sregex_token_iterator end;

    // debug
    cout << "line:" << line_num;
    for(; iter!=end; iter++) {
             cout << " ["<< iter->str() << "] ";
    }

    if(line_num == ld->end_line_num){
        return false;
    }

    line_num++;
    return true;
}

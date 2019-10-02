// controller.cpp:
// execute instructions

#include <string>
#include <regex>
#include <iostream>
#include "loader.h"
#include "memory.h"
#include "register.h"
#include "controller.h"
using namespace std;


controller::controller(loader* l, memory* m, reg r[]){
    ld = l;
    memo = m;
    regs = r;
    line_num = 1;
}

bool controller::exec_step(){
    // for debug
    /*
    regs[8].data = regs[8].data+1;
    cout << "$8:" << regs[8].data << endl;
    */

    string line_str = ld->get_program_by_line_num(line_num);

    // split opecode and residual and remove redundant spases
    regex sep("[\\t ]+");
    sregex_token_iterator iter(line_str.begin(), line_str.end(), sep, -1);
    sregex_token_iterator end;
    string opecode_str = iter->str();
    iter++;
    string res_str = "" ;

    //cout << "line:" << line_num <<  "\t"; // for debug
    for(; iter!=end; iter++) {
        //cout << " [" << iter->str() << "] "; // for debug
        res_str = res_str + iter->str();
    }
    //cout << endl; // for debug

    /* debug*/
     cout << "line:" << line_num <<  "\topecode: "<<"[" << opecode_str <<"]"<< "\tres: "<<"[" << res_str <<"]"<< endl;


    exec_code(opecode_str, res_str);

    if(line_num == ld->end_line_num){
        return false;
    }

    line_num++;
    return true;
}


void controller::exec_code(string opecode, string res){
    if(opecode=="add"){
    }else if(opecode=="addi"){
    }else if(opecode=="j"){
    }else if(opecode=="jr"){
    }else if(opecode=="jal"){
    }else if(opecode=="sw"){
    }else if(opecode=="lw"){
    }else{
    }
}

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
    regs[0].data = 0;
    regs[29].data = memorySize-4; // init sp;
}

bool controller::exec_step(){

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

     //cout << "line:" << line_num <<  "\t"; // for debug
     for(; iter!=end; iter++) {
         //cout << " [" << iter->str() << "] "; // for debug
         res_str = res_str + iter->str();
     }

    exec_code(opecode_str, res_str);

    if(line_num == ld->end_line_num){
        return false;
    }

    line_num++;
    return true;
}

sim_addr controller::get_addr_by_base_plus_offset(string base_plus_offset){
    regex sep("([+-]?)([0-9]+)\\(([%d]+)\\)");
    sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(), sep, {1,2,3});
    string sign = iter->str();
    iter++;
    string offset_str = iter->str();
    int offset = stoi(offset_str); // convert string to int
    iter++;
    string base_str =  iter->str();
    int reg_num = stoi(base_str);
    int base_addr = regs[reg_num].data; // sim_addr has the same type as sim_word

    if(sign=="-"){
        // for debug
        //cout <<"str:"<< base_plus_offset << " base addr:" << base_addr  << " offset:" << sign <<  base_addr << "result:" << base_addr - offset << endl;
        return (sim_addr)(base_addr - offset);
    }else{
        // for debug
        //cout <<"str:"<< base_plus_offset << " base addr:" << base_addr  << " offset:" << sign <<  base_addr << "result:" << base_addr + offset << endl;
        return (sim_addr)(base_addr + offset);
    }
}

int controller::get_reg_num(string reg_str){
    regex sep("\\$(\\d+)");
    sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);

    // for debug
    //cout <<"from reg_str:"<< reg_str << " to reg num:" << iter->str() << endl;

    unsigned int reg_num = stoi(iter->str()); // convert string to int to unsigned int
    return reg_num;
}


int controller::get_immediate(string immediate_str){
    regex sep("([+-]?)([0-9]+)");
    sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(), sep, {1,2});
    string sign = iter->str();
    iter++;
    int immediate = stoi(iter->str()); // convert string to int to unsigned int

    if(sign=="-"){
        // for debug
        // cout <<"from immediate_str:"<< immediate_str << " to immediate:" << - immediate << endl;
        return - immediate;
    }else{
        // for debug
        // cout <<"from immediate_str:"<< immediate_str << " to immediate:" << immediate << endl;
        return immediate;
    }
}


void controller::exec_code(string opecode, string res){
    // split res by ","
    regex sep(",");
    sregex_token_iterator iter(res.begin(), res.end(), sep, -1);
    sregex_token_iterator end;

    if(opecode=="add"){ // ADD rd <- rs + rt
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        iter++;
        int rt = get_reg_num(iter->str());
        regs[rd].data = regs[rs].data + regs[rt].data;
        /* for debug */
        cout << "rd($" << rd << "):" <<  " <- rs($" << rs << "):" << regs[rs].data << " + rt($" << rt << "):" << regs[rt].data << endl;
        cout << "rd($" << rd << "):" << regs[rd].data << endl;
    }else if(opecode=="addi"){ // ADDI rd <- rs + immediate
        int rd = get_reg_num(iter->str());
        iter++;

        int rs = get_reg_num(iter->str());
        iter++;
        int immediate = get_immediate(iter->str());
        /* for debug*/
        cout << "rd($" << rd << "):" <<  " <- rs($" << rs << "):" << regs[rs].data << " + immediate:" << immediate << endl;
        regs[rd].data = regs[rs].data + immediate;
        /* for debug*/
        cout << "rd($" << rd << "):" << regs[rd].data << endl;
    }else if(opecode=="j"){
        string label_str = iter->str();
        line_num = ld->get_line_num_by_label(label_str);
    }else if(opecode=="jr"){
    }else if(opecode=="jal"){
    }else if(opecode=="sw"){
    }else if(opecode=="lw"){
    }else{
    }
}

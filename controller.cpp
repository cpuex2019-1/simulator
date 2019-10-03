// controller.cpp:
// execute instructions

#include <string>
#include <regex>
#include <iostream>
#include "controller.h"
using namespace std;

bool debug_flag = true ;

controller::controller(loader* l, memory* m, reg r[]){
    ld = l;
    memo = m;
    regs = r;
    line_num = 1; // (line_num-1)*4 is addr for instruction?
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
    for(; iter!=end; iter++) {
        res_str = res_str + iter->str();
    }

    cout << "line:" << line_num <<  "\topecode: "<<"[" << opecode_str <<"]"<< "\tres: "<<"[" << res_str <<"]"<< endl;

    exec_code(opecode_str, res_str);

    cout << endl;

    if(line_num >= ld->end_line_num){
        return false;
    }


    return true;
}

sim_addr controller::get_addr_by_base_plus_offset(string base_plus_offset){
    regex sep("([+-]?)([0-9]+)\\(\\$(\\d+)\\)");
    sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(), sep, {1,2,3});

    string sign = iter->str();
    iter++;
    string offset_str = iter->str();
    int offset=0;
    try {
        offset = stoi(offset_str); // convert string to int
    }catch (const std::invalid_argument& e) {
        cout << "[" << offset_str << "]: " << "invalid argument" << endl;
        exit(1);
    }
    iter++;
    string base_str =  iter->str();
    int reg_num=0;
    try {
        reg_num = stoi(base_str);
    }catch (const std::invalid_argument& e) {
        cout << "[" << offset_str << "]: " << "invalid argument" << endl;
        exit(1);
    }


    int base_addr = regs[reg_num].data; // sim_addr has the same type as sim_word

    if(sign=="-"){
        // for debug
        //cout <<"str:"<< base_plus_offset << "\tbase addr:" << base_addr  << "\toffset:" << sign <<  offset << "\tresult:" << base_addr - offset << endl;
        return (sim_addr)(base_addr - offset);
    }else{
        // for debug
        //cout <<"str:"<< base_plus_offset << "\tbase addr:" << base_addr  << "\toffset:" << sign <<  offset << "\tresult:" << base_addr + offset << endl;
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
        int re_data = regs[rs].data;
        int rt = get_reg_num(iter->str());
        regs[rd].data = re_data + regs[rt].data;

        if (debug_flag==true){
            cout << "rd($" << rd << "):" <<  " <- rs($" << rs << "):" << re_data << " + rt($" << rt << "):" << regs[rt].data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    }else if(opecode=="addi"){ // ADDI rd <- rs + immediate
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        iter++;
        int immediate = get_immediate(iter->str());

        if (debug_flag==true){
            cout << "rd($" << rd << "):" <<  " <- rs($" << rs << "):" << regs[rs].data << " + immediate:" << immediate << endl;
            regs[rd].data = regs[rs].data + immediate;
            cout << "rd($" << rd << "):" << regs[rd].data << endl;
        }

        line_num++;

    }else if(opecode=="lw"){ // LW rd, offset(base)
        int rd = get_reg_num(iter->str());
        iter++;
        int addr = get_addr_by_base_plus_offset(iter->str());
        regs[rd].data = memo->read_word(addr);
        if (debug_flag==true){
            cout << "rd($" << rd << ") <-" <<  "\tmemo[" << addr << "]"<< endl;
            cout << "reg[" << rd << "]:" << regs[rd].data << endl;
        }
        line_num++;

    }else if(opecode=="sw"){
        int rd = get_reg_num(iter->str());
        iter++;
        int addr = get_addr_by_base_plus_offset(iter->str());
        memo->write_word(addr, regs[rd].data);
        if (debug_flag==true){
            cout << "rd($" << rd << "):" << regs[rd].data <<  "\taddr:" << addr << endl;
            cout << "memory[" << addr << "]:" << memo->read_word(addr) << endl;
        }
        line_num++;

    }else if( opecode=="mov" || opecode=="move"){ // mov rd <- rs
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        regs[rd].data = regs[rs].data;
        if (debug_flag==true){
            cout << "rd($" << rd << ")" << " <- rs($" << rs << "):" << regs[rs].data << endl;
            cout << "rd($" << rd << "):" << regs[rd].data;
        }
        line_num++;

    }else if(opecode=="beq"){ // BEQ rs rt label(pc+offset)
        int rs = get_reg_num(iter->str());
        iter++;
        int rt = get_reg_num(iter->str());
        iter++;
        if(regs[rs].data == regs[rt].data){
            string label_str = iter->str();
            line_num = ld->get_line_num_by_label(label_str);
        }else{
            line_num++;
        }
        if (debug_flag==true){
            cout << "rs($" << rs << "):" << regs[rs].data << "  rt($" << rt << "):" << regs[rt].data << endl;
            cout << "next line:" << line_num;
        }

    }else if(opecode=="j"){ // J label
        string label_str = iter->str();
        line_num = ld->get_line_num_by_label(label_str);

    }else if(opecode=="jr"){ // JR rs
        int rs = get_reg_num(iter->str());
        line_num = (regs[rs].data / 4) + 1; // convert program addr to line number;

    }else if(opecode=="jal"){ // JAL label (next instruction addr is line_num*4)
        regs[31].data = line_num*4;
        string label_str = iter->str();
        line_num = ld->get_line_num_by_label(label_str);

    }else if(opecode=="slt"){ // slt Rd = if Rs < Rt then 1 else 0
        cout << "invalid opecode" << endl;
        int rd = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        iter++;
        int rt = get_reg_num(iter->str());

        if (regs[rs].data < regs[rt].data){
            regs[rd].data = 1;
        }else{
            regs[rd].data = 0;
        }
        if (debug_flag==true){
            cout << "rd($" << rd << "):" << regs[rd].data << "  rs($" << rs << ")" << regs[rs].data << " < rt($" << rt << ")" << regs[rt].data << endl;
        }
        line_num++;

    }else if(opecode=="slti"){ // slti Rt = if Rs < Imm then 1 else 0 (いらない)
        cout << "invalid opecode" << endl;
        int rt = get_reg_num(iter->str());
        iter++;
        int rs = get_reg_num(iter->str());
        iter++;
        int immediate = get_immediate(iter->str());

        if (regs[rs].data < immediate){
            regs[rt].data = 1;
        }else{
            regs[rt].data = 0;
        }
        if (debug_flag==true){
            cout << "rt($" << rt << "):" << regs[rt].data << "  rs($" << rs << ")";
        }
        line_num++;
    }else if(opecode=="bgt"){ // BEQ rs rt label(pc+offset)  (いらない)
        cout << "invalid opecode" << endl;
        int rs = get_reg_num(iter->str());
        iter++;
        int rt = get_reg_num(iter->str());
        iter++;
        if(regs[rs].data > regs[rt].data){
            string label_str = iter->str();
            line_num = ld->get_line_num_by_label(label_str);
        }else{
            line_num++;
        }
        if (debug_flag==true){
            cout << "rs($" << rs << "):" << regs[rs].data << "  rt($" << rt << "):" << regs[rt].data << endl;
            cout << "next line:" << line_num;
        }
    }else{
        line_num++;
    }
}

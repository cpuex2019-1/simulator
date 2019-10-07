// loader.cpp:
//

#include "loader.h"
#include "asm.h"
#include "global.h"
#include <regex>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

// constructor
loader::loader(const char *fname, Log *l_level) {
    file_name = fname;
    log_level = l_level;
    line_num = 0; // reset line number
    end_line_num = load_file();
    vector<int> first;
    first.push_back(634);
    program_map.insert(program_map.begin(), first); // ignore number 0
    raw_program.insert(raw_program.begin(),
                       "program begin hear"); // ignore number 0
}

// destructor
/*
loader::~loader() {

}
*/
int loader::get_reg_by_base_plus_offset(string base_plus_offset) {
    regex sep("([+-]?)(0|[1-9][0-9]*)\\(\\$(3[0-1]|[1-2][0-9]|[0-9])\\)");
    sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(),
                               sep, 3);
    sregex_token_iterator end;
    if (iter == end) {
        if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid base plus offset[%s]\n", line_num,
                   base_plus_offset.c_str());
        }
        exit(1);
    } else {
        string base_str = iter->str();
        int reg_num = 0;
        try {
            reg_num = stoi(base_str);
            return reg_num;
        } catch (const std::invalid_argument &e) {
            if (*log_level >= FATAL) {
                printf("FATAL\tinvalid base plus offset: [%s]\n",
                       base_plus_offset.c_str());
            }
            exit(1);
        }
    }
}
int loader::get_offset_by_base_plus_offset(string base_plus_offset) {
    regex sep("([+-]?)(0|[1-9][0-9]*)\\(\\$(3[0-1]|[1-2][0-9]|[0-9])\\)");
    sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(),
                               sep, {1, 2});
    sregex_token_iterator end;
    if (iter == end) {
        if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid base plus offset[%s]\n", line_num,
                   base_plus_offset.c_str());
        }
        exit(1);
    } else {
        string sign = iter->str();
        iter++;
        string offset_str = iter->str();
        int offset = 0;
        try {
            offset = stoi(offset_str); // convert string to int
            if (sign == "-") {
                return -offset;
            } else {
                return offset;
            }
        } catch (const std::invalid_argument &e) {
            if (*log_level >= FATAL) {
                printf("FATAL\tinvalid base plus offset: [%s]\n",
                       base_plus_offset.c_str());
            }
            exit(1);
        }
    }
}

int loader::get_reg_num(string reg_str) {
    regex sep("\\$(3[0-1]|[1-2][0-9]|[0-9])");
    sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);
    sregex_token_iterator end;
    if (iter == end) {
        if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid register[%s]\n", line_num,
                   reg_str.c_str());
        }
        exit(1);
    } else {
        try {
            int reg_num = stoi(iter->str()); // convert string to int
            return reg_num;
        } catch (const std::invalid_argument &e) {
            if (*log_level >= FATAL) {
                printf("FATAL\tinvalid register: [%s]\n", iter->str().c_str());
            }
            exit(1);
        }
    }
}

int loader::get_immediate(string immediate_str) {
    regex sep("([+-]?)(0|[1-9][0-9]*)"); //([+-]?)([0-9]+)
    sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(), sep,
                               {1, 2});
    sregex_token_iterator end;
    if (iter == end) {
        if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid immediate[%s]\n", line_num,
                   immediate_str.c_str());
        }
        exit(1);
    } else {
        string sign = iter->str();
        iter++;
        if (iter == end) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid immediate[%s]\n", line_num,
                       immediate_str.c_str());
            }
            exit(1);
        } else {
            try {
                int immediate =
                    stoi(iter->str()); // convert string to int to unsigned int

                if (sign == "-") {
                    return -immediate;
                } else {
                    return immediate;
                }
            } catch (const std::invalid_argument &e) {
                if (*log_level >= FATAL) {
                    printf("FATAL\tinvalid immediate: %s[%s]\n", sign.c_str(),
                           iter->str().c_str());
                }
                exit(1);
            }
        }
    }
}

int loader::load_file() {
    // load label
    ifstream input;
    input.open(file_name);
    if (!input) {
        if (*log_level >= FATAL) {
            printf("can't open file: %s\n", file_name);
        }
        exit(1);
    }
    string linebuf;
    while (!input.eof()) {
        line_num++; // increment before processing the line
        getline(input, linebuf);
        load_line_label(linebuf);
    }
    if (input.is_open()) {
        input.close();
    }

    // load program
    line_num = 0;
    input.open(file_name);
    if (!input) {
        if (*log_level >= FATAL) {
            printf("can't open file: %s\n", file_name);
        }
        exit(1);
    }
    while (!input.eof()) {
        line_num++; // increment before processing the line
        getline(input, linebuf);
        load_line(linebuf);
    }
    if (input.is_open()) {
        input.close();
    }

    return line_num;
}

void loader::load_line_label(string line) {
    // delete comment
    regex comment_pattern("#.*$"); // #から末尾まで
    sregex_token_iterator iter1(line.begin(), line.end(), comment_pattern, -1);
    sregex_token_iterator end1;
    string line_not_comment = "";
    for (; iter1 != end1; iter1++) {
        line_not_comment = line_not_comment + iter1->str();
    }

    // get label
    regex label_pattern("^[\\t ]*(?:([A-Za-z][\\w\\.]*)[:])?[\\t ]*");
    sregex_token_iterator iter2(line_not_comment.begin(),
                                line_not_comment.end(), label_pattern,
                                {1, -1}); // group1: label, 残り: コード
    sregex_token_iterator end;
    string label_str = iter2->str();
    iter2++;
    string res_str = "";
    for (; iter2 != end; iter2++) {
        res_str = res_str + iter2->str();
    }

    if (label_str != "") {
        label_map.insert(std::make_pair(label_str, line_num));
    }
}

// load line
void loader::load_line(string line) {

    // delete comment
    regex comment_pattern("#.*$"); // #から末尾まで
    sregex_token_iterator iter1(line.begin(), line.end(), comment_pattern, -1);
    sregex_token_iterator end1;
    string line_not_comment = "";
    for (; iter1 != end1; iter1++) {
        line_not_comment = line_not_comment + iter1->str();
    }

    // get label
    regex label_pattern("^[\\t ]*(?:([A-Za-z][\\w\\.]*)[:])?[\\t ]*");
    sregex_token_iterator iter2(line_not_comment.begin(),
                                line_not_comment.end(), label_pattern,
                                {1, -1}); // group1: label, 残り: コード
    sregex_token_iterator end;
    iter2++;
    string res_str = "";
    for (; iter2 != end; iter2++) {
        res_str = res_str + iter2->str();
    }

    // split opecode and residual and remove redundant spases
    vector<string> code;
    regex sep1("[\\t ]+");
    sregex_token_iterator iter3(res_str.begin(), res_str.end(), sep1, -1);
    string opecode_str = iter3->str();
    code.push_back(opecode_str);
    iter3++;
    string res = "";
    for (; iter3 != end; iter3++) {
        res = res + iter3->str();
    }
    raw_program.push_back(opecode_str + "\t" + res);

    // process operand (split res by ",")
    regex sep2(",");
    sregex_token_iterator iter4(res.begin(), res.end(), sep2, -1);
    for (; iter4 != end; iter4++) {
        code.push_back(iter4->str());
    }

    vector<int> formatted_code = format_code(code);

    program_map.push_back(formatted_code);
}

vector<int> loader::format_code(vector<string> code) {
    auto iter = code.begin();
    string opecode = *iter;
    iter++;
    vector<int> result;

    if (opecode == "add") { // ADD rd <- rs + rt
        result.push_back(ADD);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "addi") { // ADDI rd <- rs + immediate
        result.push_back(ADDI);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int immediate = get_immediate(*iter);
        result.push_back(immediate);

    } else if (opecode == "sub") { // SUB rd <- rs - rt
        result.push_back(SUB);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "mul") { // MUL rd <- rs * rt
        result.push_back(MUL);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "div") { // DIV rd <- rs / rt
        result.push_back(DIV);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "mod") { // MOD rd <- rs % rt
        result.push_back(MOD);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "slt") { // SLT Rd = if Rs < Rt then 1 else 0
        result.push_back(SLT);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "and") { // AND rd <- rs & rt
        result.push_back(AND);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "andi") { // ADNI rd <- rs & immediate
        result.push_back(ANDI);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int immediate = get_immediate(*iter);
        result.push_back(immediate);

    } else if (opecode == "or") { // OR rd <- rs | rt
        result.push_back(OR);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "ori") { // ORI rd <- rs & immediate
        result.push_back(ORI);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int immediate = get_immediate(*iter);
        result.push_back(immediate);

    } else if (opecode == "nor") { // NOR rd <- ~(rs | rt)
        result.push_back(NOR);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "xor") { // XOR rd <- rs ^ rt
        result.push_back(XOR);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "xori") { // XORI rd <- rs & immediate
        result.push_back(XORI);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int immediate = get_immediate(*iter);
        result.push_back(immediate);

    } else if (opecode == "srai") { // SRAI rd <- rs >> sb (arithmetic)
        result.push_back(SRAI);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int sb = get_immediate(*iter);
        result.push_back(sb);

    } else if (opecode == "srli") { // SRLI rd <- rs >> sb (logical)
        result.push_back(SRL);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int sb = get_immediate(*iter);
        result.push_back(sb);

    } else if (opecode == "slli") { // SLLI rd <- rs << sb (logical)
        result.push_back(SLLI);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int sb = get_immediate(*iter);
        result.push_back(sb);

    } else if (opecode == "sra") { // SRA rd <- rs >> rt (arithmetic)
        result.push_back(SRA);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "srl") { // SRL rd <- rs >> rt (logical)
        result.push_back(SRL);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "sll") { // SLLI rd <- rs << rt (logical)
        result.push_back(SLL);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);

    } else if (opecode == "lw") { // LW rd, offset(base)
        result.push_back(LW);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);

    } else if (opecode == "lb") { // LB rd, offset(base)
        result.push_back(LB);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);

    } else if (opecode == "sw") {
        result.push_back(SW);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);

    } else if (opecode == "sb") { // sb rd, offset(base)
        result.push_back(SB);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);

    } else if (opecode == "mov" || opecode == "move") { // mov rd <- rs
        result.push_back(MOV);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;

    } else if (opecode == "bc") { // BC label(pc+offset<<2)
        result.push_back(BC);
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num);

    } else if (opecode == "beq") { // BEQ rs rt label(pc+offset<<2)
        result.push_back(BEQ);
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num);

    } else if (opecode == "bne") { // BNE rs rt label(pc+offset<<2)
        result.push_back(BNE);
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num);

    } else if (opecode == "j") { // J label
        result.push_back(J);
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num);

    } else if (opecode == "jr") { // JR rs
        result.push_back(JR);
        int rs = get_reg_num(*iter);
        result.push_back(rs);

    } else if (opecode ==
               "jal") { // JAL label (next instruction addr is line_num*4)
        result.push_back(JAL);
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num);

    } else if (opecode == "jalr") { // JALR rd, rs
        result.push_back(JALR);
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
    } else if (opecode == "nop") { // nop
        result.push_back(JALR);
    } else if (opecode == "out") { // output 未対応
        result.push_back(OUT);
    } else {
        if (opecode != "") {
            if (*log_level >= FATAL) {
                printf("FATAL\tinvalid opecode:%s (line:%d)", opecode.c_str(),
                       line_num);
            }
            exit(1);
        }
        result.push_back(NOP);
    }
    return result;
}

// public

int loader::get_line_num_by_label(string label) {
    auto it = label_map.find(label);
    if (it == label_map.end()) {
        if (*log_level >= FATAL) {
            printf("FATAL\tnot found label: %s", label.c_str());
        }
        exit(1);
    } else {
        return label_map[label];
    }
}
vector<int> loader::get_program_by_label(string label) {
    int line_num_of_label = get_line_num_by_label(label);
    return program_map[line_num_of_label];
}

vector<int> loader::get_program_by_line_num(int l_num) {
    return program_map[l_num];
}

string loader::get_raw_program_by_line_num(int l_num) {
    return raw_program[l_num];
}

void loader::print_label_map() {
    printf("label map\n");
    for (auto itr = label_map.begin(); itr != label_map.end(); ++itr) {
        printf("\t%s :\t%d\n", itr->first.c_str(), itr->second);
    }
}
void loader::print_program_map() {
    printf("program map\n");
    int line = 0;
    for (auto itr = program_map.begin(); itr != program_map.end(); ++itr) {
        if (line != 0) {
            for (auto itr_int = itr->begin(); itr_int != itr->end();
                 ++itr_int) {
                printf("%d\t", *itr_int);
            }
            printf("\n");
        }
        line++;
    }
}

void loader::print_raw_program() {
    printf("raw program\n");
    int line = 0;
    for (auto itr = raw_program.begin(); itr != raw_program.end(); ++itr) {
        if (line != 0) {
            printf("%8d:\t%s\n", line, itr->c_str());
        }
        line++;
    }
}

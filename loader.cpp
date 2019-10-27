// loader.cpp:
//

#include "loader.h"
#include "asm.h"
#include "global.h"
#include "print.h"
#include <regex>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

string label_str_tmp;

// constructor
loader::loader(const char *fname, Log *l_level) {
    file_name = fname;
    log_level = l_level;
    load_line_num = 0;
    line_num = 0; // reset line number

    output_exist = false;
    input_exist = false;
    end_line_num = load_file();
    assemble();
}

// destructor
/*
loader::~loader() {

}
*/
int loader::get_reg_by_base_plus_offset(string base_plus_offset) {
    regex sep("^([+-]?)(0|[1-9][0-9]*)\\(\\$(3[0-1]|[1-2][0-9]|[0-9])\\)$");
    sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(),
                               sep, 3);
    sregex_token_iterator end;
    if (iter == end) {
        if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid base plus offset[%s]\n",
                   load_line_num, base_plus_offset.c_str());
        }
        exit(1);
    } else {
        string base_str = iter->str();
        int reg_num = 0;
        try {
            reg_num = stoi(base_str);
            return reg_num;
        } catch (std::out_of_range &e) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid base plus offset: "
                       "[%s](out_of_range)\n",
                       load_line_num, base_plus_offset.c_str());
            }
            exit(1);
        } catch (...) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid base plus offset: [%s]\n",
                       load_line_num, base_plus_offset.c_str());
            }
            exit(1);
        }
    }
}
int loader::get_offset_by_base_plus_offset(string base_plus_offset) {
    regex sep("^([+-]?)(0|[1-9][0-9]*)\\(\\$(3[0-1]|[1-2][0-9]|[0-9])\\)$");
    sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(),
                               sep, {1, 2});
    sregex_token_iterator end;
    if (iter == end) {
        if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid base plus offset[%s]\n",
                   load_line_num, base_plus_offset.c_str());
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
        } catch (std::out_of_range &e) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid base plus offset: "
                       "[%s](out_of_range)\n",
                       load_line_num, base_plus_offset.c_str());
            }
            exit(1);
        } catch (...) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid base plus offset: [%s]\n",
                       load_line_num, base_plus_offset.c_str());
            }
            exit(1);
        }
    }
}

int loader::get_reg_num(string reg_str) {
    regex sep("^\\$(3[0-1]|[1-2][0-9]|[0-9])$");
    sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);
    sregex_token_iterator end;
    if (iter == end) {
        if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid register[%s]\n", load_line_num,
                   reg_str.c_str());
        }
        exit(1);
    } else {
        try {
            int reg_num = stoi(iter->str()); // convert string to int
            return reg_num;
        } catch (std::out_of_range &e) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid register: "
                       "[%s](out_of_range)\n",
                       load_line_num, reg_str.c_str());
            }
            exit(1);
        } catch (...) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid register: [%s]\n",
                       load_line_num, reg_str.c_str());
            }
            exit(1);
        }
    }
}

int loader::get_freg_num(string reg_str) {
    regex sep("^\\$f(3[0-1]|[1-2][0-9]|[0-9])$");
    sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);
    sregex_token_iterator end;
    if (iter == end) {
        if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid register[%s]\n", load_line_num,
                   reg_str.c_str());
        }
        exit(1);
    } else {
        try {
            int reg_num = stoi(iter->str()); // convert string to int
            return reg_num;
        } catch (std::out_of_range &e) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid register: "
                       "[%s](out_of_range)\n",
                       load_line_num, reg_str.c_str());
            }
            exit(1);
        } catch (...) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid register: [%s]\n",
                       load_line_num, reg_str.c_str());
            }
            exit(1);
        }
    }
}

int loader::get_arith_immediate(string init_immediate_str) {
    // check immediate
    string immediate_str = init_immediate_str;
    regex sep("^([+-]?)(0|[1-9][0-9]*)$"); //([+-]?)([0-9]+)
    sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(), sep,
                               {1, 2});
    sregex_token_iterator end;
    if (iter != end) {
        string sign = iter->str();
        iter++;
        if (iter == end) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                       immediate_str.c_str());
            }
            exit(1);
        } else {
            try {
                int immediate =
                    stoi(iter->str()); // convert string to int to unsigned int
                if (sign == "-") {
                    if (immediate <= 32768) { // -2^15まで
                        return -immediate;
                    } else {
                        if (*log_level >= FATAL) {
                            printf("FATAL\tline:%d\tinvalid immediate: "
                                   "[%s](under -2^15)\n",
                                   load_line_num, immediate_str.c_str());
                        }
                        exit(1);
                    }
                } else {
                    if (immediate <= 32767) { // 2^15-1まで
                        return immediate;
                    } else {
                        if (*log_level >= FATAL) {
                            printf("FATAL\tline:%d\tinvalid immediate: "
                                   "[%s](over 2^15-1)\n",
                                   load_line_num, immediate_str.c_str());
                        }
                        exit(1);
                    }
                }
            } catch (std::out_of_range &e) {
                if (*log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate: "
                           "[%s](out_of_range)\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            } catch (...) {
                if (*log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate: [%s]\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            }
        }
    } else { // ha(label) or lo(label)
        immediate_str = init_immediate_str;
        regex halo_regex("^(ha|lo)\\(([A-Za-z][\\w\\.]*)\\)$");
        sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(),
                                   halo_regex, {1, 2});
        sregex_token_iterator end;
        if (iter != end) {

            string halo = iter->str();
            iter++;
            if (iter == end) {
                if (*log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate[%s]\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            } else {
                string label_str =
                    iter->str(); // convert string to int to unsigned int
                int label_num = get_line_num_by_label(label_str);
                unsigned int label_addr = ((unsigned int)label_num) * 4;

                if (halo == "ha") {
                    return label_addr >> 16;
                } else if (halo == "lo") {
                    return (label_addr << 16) >> 16;
                } else {
                    if (*log_level >= FATAL) {
                        printf("FATAL\tline:%d\tinvalid immediate: [%s]\n",
                               load_line_num, immediate_str.c_str());
                    }
                    exit(1);
                }
            }
        } else {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                       immediate_str.c_str());
            }
            exit(1);
        }
    }
}

int loader::get_logic_immediate(string init_immediate_str) {
    // check immediate
    string immediate_str = init_immediate_str;
    regex sep("^([+-]?)(0|[1-9][0-9]*)$"); //([+-]?)([0-9]+)
    sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(), sep,
                               {1, 2});
    sregex_token_iterator end;
    if (iter != end) {
        string sign = iter->str();
        iter++;
        if (iter == end) {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                       immediate_str.c_str());
            }
            exit(1);
        } else {
            try {
                int immediate =
                    stoi(iter->str()); // convert string to int to unsigned int

                if (sign == "-") {
                    if (*log_level >= FATAL) {
                        printf("FATAL\tline:%d\tinvalid immediate: "
                               "[%s](under 0)\n",
                               load_line_num, immediate_str.c_str());
                    }
                    exit(1);
                } else {
                    if (immediate <= 65535) { // 2^16-1まで
                        return immediate;
                    } else {
                        if (*log_level >= FATAL) {
                            printf("FATAL\tline:%d\tinvalid immediate: "
                                   "[%s](over 2^16-1)\n",
                                   load_line_num, immediate_str.c_str());
                        }
                        exit(1);
                    }
                }

            } catch (std::out_of_range &e) {
                if (*log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate: "
                           "[%s](out_of_range)\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            } catch (...) {
                if (*log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate: [%s]\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            }
        }
    } else { // ha(label) or lo(label)
        immediate_str = init_immediate_str;
        regex halo_regex("^(ha|lo)\\(([A-Za-z][\\w\\.]*)\\)$");
        sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(),
                                   halo_regex, {1, 2});
        sregex_token_iterator end;
        if (iter != end) {

            string halo = iter->str();
            iter++;
            if (iter == end) {
                if (*log_level >= FATAL) {
                    printf("FATAL\tline:%d\tinvalid immediate[%s]\n",
                           load_line_num, immediate_str.c_str());
                }
                exit(1);
            } else {
                string label_str =
                    iter->str(); // convert string to int to unsigned int
                int label_num = get_line_num_by_label(label_str);
                unsigned int label_addr = ((unsigned int)label_num) * 4;

                if (halo == "ha") {
                    return label_addr >> 16;
                } else if (halo == "lo") {
                    return (label_addr << 16) >> 16;
                } else {
                    if (*log_level >= FATAL) {
                        printf("FATAL\tline:%d\tinvalid immediate: [%s]\n",
                               load_line_num, immediate_str.c_str());
                    }
                    exit(1);
                }
            }
        } else {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                       immediate_str.c_str());
            }
            exit(1);
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
        load_line_num++;
        getline(input, linebuf);
        load_line_label(linebuf);
    }
    if (input.is_open()) {
        input.close();
    }

    // load program
    line_num = 0;
    load_line_num = 0;
    input.open(file_name);
    if (!input) {
        if (*log_level >= FATAL) {
            printf("can't open file: %s\n", file_name);
        }
        exit(1);
    }
    while (!input.eof()) {

        load_line_num++;
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

    // remove redundant spases
    vector<string> code;
    regex sep1("[\\t ]+");
    sregex_token_iterator iter3(res_str.begin(), res_str.end(), sep1, -1);
    string res = "";
    for (; iter3 != end; iter3++) {
        res = res + iter3->str();
    }

    if (label_str != "") {
        label_map.insert(std::make_pair(label_str, line_num));
    }
    if (res != "") { // 命令がある場合
        line_num++;  // increment before processing the line
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
    string label_str = iter2->str();
    iter2++;
    string res_str = "";
    for (; iter2 != end; iter2++) {
        res_str = res_str + iter2->str();
    }
    if (label_str != "") {
        label_str_tmp = label_str;
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

    // process operand (split res by ",")
    regex sep2(",");
    sregex_token_iterator iter4(res.begin(), res.end(), sep2, -1);
    for (; iter4 != end; iter4++) {
        code.push_back(iter4->str());
    }

    if (code[0] != "") {

        if (label_str_tmp != "") {
            raw_program.push_back(label_str_tmp + ":\t" + opecode_str + "\t" +
                                  res);
        } else {
            raw_program.push_back("\t\t" + opecode_str + "\t" + res);
        }

        unsigned int machine_code_line = format_code(code);
        machine_code.push_back(machine_code_line);
        label_str_tmp = "";
        line_num++; // increment before processing the line
    }
}

unsigned int loader::format_code(vector<string> code) {
    auto iter = code.begin();
    string opecode = *iter;
    iter++;
    vector<int> result;

    if (opecode == "add") { // ADD rd <- rs + rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x20;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "addi") { // ADDI rd <- rs + immediate
        unsigned int op_bit = 0x8 << 26;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int immediate_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int immediate = get_arith_immediate(*iter);
                immediate_bit = (unsigned int)immediate & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result = op_bit | rd_bit | rs_bit | immediate_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sub") { // SUB rd <- rs - rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x22;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "mul") { // MUL rd <- rs * rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x18;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "div") { // DIV rd <- rs / rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = (0x2 << 6);
        unsigned int funct_bit = 0x1A;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "mod") { // MOD rd <- rs % rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x1A;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "slt") { // SLT Rd = if Rs < Rt then 1 else 0
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x2A;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "and") { // AND rd <- rs & rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x24;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "andi") { // ADNI rd <- rs & immediate
        unsigned int op_bit = 0xC << 26;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int immediate_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int immediate = get_logic_immediate(*iter);
                immediate_bit = (unsigned int)immediate & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result = op_bit | rd_bit | rs_bit | immediate_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "or") { // OR rd <- rs | rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x25;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "ori") { // ORI rd <- rs & immediate
        unsigned int op_bit = 0xD << 26;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int immediate_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int immediate = get_logic_immediate(*iter);
                immediate_bit = (unsigned int)immediate & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result = op_bit | rd_bit | rs_bit | immediate_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "nor") { // NOR rd <- ~(rs | rt)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x27;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "xor") { // XOR rd <- rs ^ rt
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x26;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "xori") { // XORI rd <- rs & immediate
        unsigned int op_bit = 0xE << 26;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int immediate_bit;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int immediate = get_logic_immediate(*iter);
                immediate_bit = (unsigned int)immediate & 0xffff;
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result = op_bit | rd_bit | rs_bit | immediate_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "srai") { // SRAI rd <- rs >> sb (arithmetic)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit;
        unsigned int funct_bit = 0x3;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int sb = get_logic_immediate(*iter); // ＊要変更
                shamt_bit = ((unsigned int)sb << 6);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "srli") { // SRLI rd <- rs >> sb (logical)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit;
        unsigned int funct_bit = 0x2;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int sb = get_logic_immediate(*iter); // ＊要変更
                shamt_bit = ((unsigned int)sb << 6);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "slli") { // SLLI rd <- rs << sb (logical)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit;
        unsigned int funct_bit = 0x0;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int sb = get_logic_immediate(*iter); // ＊要変更
                shamt_bit = ((unsigned int)sb << 6);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sra") { // SRA rd <- rs >> rt (arithmetic)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x7;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "srl") { // SRL rd <- rs >> rt (logical)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x6;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sll") { // SLLI rd <- rs << rt (logical)
        unsigned int op_bit = 0x0;
        unsigned int rd_bit;
        unsigned int rs_bit;
        unsigned int rt_bit;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x4;
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_reg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }

            unsigned int result =
                op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fadd") { // FADD rd <- rs +. rt
        result.push_back(FADD);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fsub") { // FSUB rd <- rs -. rt
        result.push_back(FSUB);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fmul") { // FMUL rd <- rs *. rt
        result.push_back(FMUL);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fdiv") { // FDIV rd <- rs /. rt
        result.push_back(FDIV);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "fneg") { // FNEG rd <- -rs
        result.push_back(FNEG);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sqrt") { // SQRT rd <- sqrt(rs)
        result.push_back(SQRT);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sltf") { // SLTF Rd = if Rs < Rt then 1 else 0
        // *rd is a general register
        result.push_back(SLTF);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "lw") { // LW rd, offset(base)
        result.push_back(LW);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                result.push_back(reg);
                int offset = get_offset_by_base_plus_offset(*iter);
                result.push_back(offset);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "lb") { // LB rd, offset(base)
        result.push_back(LB);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                result.push_back(reg);
                int offset = get_offset_by_base_plus_offset(*iter);
                result.push_back(offset);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sw") {
        result.push_back(SW);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                result.push_back(reg);
                int offset = get_offset_by_base_plus_offset(*iter);
                result.push_back(offset);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sb") { // sb rd, offset(base)
        result.push_back(SB);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                result.push_back(reg);
                int offset = get_offset_by_base_plus_offset(*iter);
                result.push_back(offset);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "mov" || opecode == "move") { // mov rd <- rs
        result.push_back(MOV);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "lf") { // lf rd, offset(base)
        result.push_back(LF);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                result.push_back(reg);
                int offset = get_offset_by_base_plus_offset(*iter);
                result.push_back(offset);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "sf") { // SF rt, offset(base)
        result.push_back(SF);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rt = get_freg_num(*iter);
                rt_bit = ((unsigned int)rt << 11);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int reg = get_reg_by_base_plus_offset(*iter);
                result.push_back(reg);
                int offset = get_offset_by_base_plus_offset(*iter);
                result.push_back(offset);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "movf") { // MOVF rd <- rs
        result.push_back(MOVF);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "bc") { // BC label(pc+offset<<2)
        result.push_back(BC);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                result.push_back(label_num - line_num);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "beq") { // BEQ rs rt label(pc+offset<<2)
        result.push_back(BEQ);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                result.push_back(label_num - line_num);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "bne") { // BNE rs rt label(pc+offset<<2)
        result.push_back(BNE);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter == code.end()) {
                throw 3;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                result.push_back(label_num - line_num);
                iter++;
            }
            if (iter != code.end()) {
                throw 4;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "j") { // J label
        result.push_back(J);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                result.push_back(label_num);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "jr") { // JR rs
        result.push_back(JR);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode ==
               "jal") { // JAL label (next instruction addr is line_num*4)
        result.push_back(JAL);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                string label_str = *iter;
                int label_num = get_line_num_by_label(label_str);
                result.push_back(label_num);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "jalr") { // JALR rd, rs
        result.push_back(JALR);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter == code.end()) {
                throw 2;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 3;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

    } else if (opecode == "inb") { // INB rd
        result.push_back(INB);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        input_exist = true;

    } else if (opecode == "in") { // IN rd
        result.push_back(IN);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_reg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        input_exist = true;

    } else if (opecode == "outb") { // OUTB rs
        result.push_back(OUTB);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }

        output_exist = true;

    } else if (opecode == "out") { // OUT rs

        result.push_back(OUT);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_reg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        output_exist = true;

    } else if (opecode == "inf") { // INF rd
        result.push_back(INF);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rd = get_freg_num(*iter);
                rd_bit = ((unsigned int)rd << 21);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        input_exist = true;

    } else if (opecode == "outf") { // OUTF rs
        result.push_back(OUTF);
        try {
            if (iter == code.end()) {
                throw 1;
            } else {
                int rs = get_freg_num(*iter);
                rs_bit = ((unsigned int)rs << 16);
                iter++;
            }
            if (iter != code.end()) {
                throw 2;
            }
        } catch (int arg_num) {
            printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
                   arg_num, get_raw_program_by_line_num(line_num).c_str());
            exit(1);
        }
        output_exist = true;

    } else if (opecode == "nop") { // nop
        result.push_back(JALR);

    } else {
        if (opecode != "") {
            if (*log_level >= FATAL) {
                printf("FATAL\tline:%d\tinvalid opecode:%s\n", load_line_num,
                       opecode.c_str());
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
            printf("FATAL\tline:%d\tnot found label: %s\n", line_num,
                   label.c_str());
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
        printf("\t%s :\t%d\n", itr->first.c_str(), (itr->second) * 4);
    }
}
void loader::print_program_map() {
    printf("program map\n");
    int line = 0;
    for (auto itr = program_map.begin(); itr != program_map.end(); ++itr) {
        for (auto itr_int = itr->begin(); itr_int != itr->end(); ++itr_int) {
            printf("%d\t", *itr_int);
        }
        printf("\n");

        line++;
    }
}

void loader::print_raw_program() {
    printf("instruction memory\n");
    int line = 0;
    for (auto itr = raw_program.begin(); itr != raw_program.end(); ++itr) {
        printf("%8d:\t%s\n", line * 4, itr->c_str());
        line++;
    }
}

void loader::assemble() {
    int line_num = 0;

    while (line_num < end_line_num) {
        vector<int> line_vec = get_program_by_line_num(line_num);
        if (*log_level >= DEBUG) { // raw_programを出力
            printf("\nDEBUG\tprogram address:%d", line_num * 4);
            string one_raw_program = get_raw_program_by_line_num(line_num);
            printf("\t%s\n", one_raw_program.c_str());
        }
        if (*log_level >= TRACE) { // programを出力
            printf("TRACE\t");
            for (auto itr_str = line_vec.begin(); itr_str != line_vec.end();
                 ++itr_str) {
                printf("%d\t", *itr_str);
            }
            printf("\n");
        }

        exec_code(line_vec);

        line_num++;
    }
}

void loader::exec_code(vector<int> line_vec) {

    auto iter = line_vec.begin();
    int opecode = *iter;
    iter++;

    if (opecode == ADD) { // ADD rd <- rs + rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x20;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == ADDI) { // ADDI rd <- rs + immediate

        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int immediate = *iter;

        unsigned int op_bit = 0x8 << 26;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int immediate_bit = (unsigned int)immediate & 0xffff;

        unsigned int code = op_bit | rd_bit | rs_bit | immediate_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SUB) { // SUB rd <- rs - rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x22;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == MUL) { // MUL rd <- rs * rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = (0x2 << 6);
        unsigned int funct_bit = 0x18;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == DIV) { // DIV rd <- rs / rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = (0x2 << 6);
        unsigned int funct_bit = 0x1A;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == MOD) { // MOD rd <- rs % rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x1A;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SLT) { // SLT Rd = if Rs < Rt then 1 else 0
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = (0x0 << 6);
        unsigned int funct_bit = 0x2A;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == AND) { // AND rd <- rs & rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = (0x0 << 6);
        unsigned int funct_bit = 0x24;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == ANDI) { // ANDI rd <- rs & immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int immediate = *iter;

        unsigned int op_bit = 0xC << 26;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int immediate_bit = (unsigned int)immediate & 0xffff;

        unsigned int code = op_bit | rd_bit | rs_bit | immediate_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == OR) { // OR rd <- rs | rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = (0x0 << 6);
        unsigned int funct_bit = 0x25;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == ORI) { // ORI rd <- rs & immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int immediate = *iter;

        unsigned int op_bit = 0xD << 26;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int immediate_bit = (unsigned int)immediate & 0xffff;

        unsigned int code = op_bit | rd_bit | rs_bit | immediate_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == NOR) { // NOR rd <- ~(rs | rt)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = (0x0 << 6);
        unsigned int funct_bit = 0x27;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == XOR) { // XOR rd <- rs ^ rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = (0x0 << 6);
        unsigned int funct_bit = 0x26;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == XORI) { // XORI rd <- rs & immediate
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int immediate = *iter;

        unsigned int op_bit = 0xE << 26;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int immediate_bit = (unsigned int)immediate & 0xffff;

        unsigned int code = op_bit | rd_bit | rs_bit | immediate_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SRAI) { // SRAI rd <- rs >> sb (arithmetic)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int sb = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = ((unsigned int)sb << 6);
        unsigned int funct_bit = 0x3;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SRLI) { // SRLI rd <- rs >> sb (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int sb = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = ((unsigned int)sb << 6);
        unsigned int funct_bit = 0x2;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SLLI) { // SLLI rd <- rs << sb (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int sb = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = ((unsigned int)sb << 6);
        unsigned int funct_bit = 0x0;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SRA) { // SRA rd <- rs >> rt (arithmetic)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x7;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SRL) { // SRL rd <- rs >> rt (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x6;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SLL) { // SLL rd <- rs << rt (logical)
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x4;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == FADD) { // FADD rd <- rs +. rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x0;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == FSUB) { // FSUB rd <- rs -. rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x1;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == FMUL) { // FMUL rd <- rs *. rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x2;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == FDIV) { // FDIV rd <- rs /. rt
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x3;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SQRT) { // SQRT rd <- sqrt(rs)
        int rd = *iter;
        iter++;
        int rs = *iter;

        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x4;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SLTF) { // SLTF
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;
        int rt = *iter;

        unsigned int op_bit = ((unsigned int)0x11 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = ((unsigned int)rt << 11);
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x8;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == LW) { // LW rd, offset(base)
        int rd = *iter;
        iter++;
        int base = *iter;
        iter++;
        int offset = *iter;

        unsigned int op_bit = (0x23 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int base_bit = ((unsigned int)base << 16);
        unsigned int offset_bit = (unsigned int)offset & 0xffff;

        unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == LB) { // LB rd, offset(base)
        int rd = *iter;
        iter++;
        int base = *iter;
        iter++;
        int offset = *iter;

        unsigned int op_bit = (0x20 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int base_bit = ((unsigned int)base << 16);
        unsigned int offset_bit = (unsigned int)offset & 0xffff;

        unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SW) {
        int rd = *iter;
        iter++;
        int base = *iter;
        iter++;
        int offset = *iter;

        unsigned int op_bit = (0x2B << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int base_bit = ((unsigned int)base << 16);
        unsigned int offset_bit = (unsigned int)offset & 0xffff;

        unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SB) { // SB rd, offset(base)
        int rd = *iter;
        iter++;
        int base = *iter;
        iter++;
        int offset = *iter;

        unsigned int op_bit = (0x28 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int base_bit = ((unsigned int)base << 16);
        unsigned int offset_bit = (unsigned int)offset & 0xffff;

        unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == MOV) { // MOV rd <- rs
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x0 << 6);
        unsigned int funct_bit = 0x20;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == LF) { // LF rd, offset(base)
        int rd = *iter;
        iter++;
        int base = *iter;
        iter++;
        int offset = *iter;

        unsigned int op_bit = (0x31 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int base_bit = ((unsigned int)base << 16);
        unsigned int offset_bit = (unsigned int)offset & 0xffff;

        unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == SF) { // SF rt, offset(base)
        int rt = *iter;
        iter++;
        int base = *iter;
        iter++;
        int offset = *iter;

        unsigned int op_bit = (0x39 << 26);
        unsigned int rd_bit = ((unsigned int)rt << 21);
        unsigned int base_bit = ((unsigned int)base << 16);
        unsigned int offset_bit = (unsigned int)offset & 0xffff;

        unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == MOVF) { // MOVF rd <- rs
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;

        unsigned int op_bit = (0x11 << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x0;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == BC) { // BC label(pc+offset<<2)
        int offset = *iter;

        unsigned int op_bit = (0x32 << 26);
        unsigned int offset_bit = offset & 0xffff; // 下位16bit のみ

        unsigned int code = op_bit | offset_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == BEQ) { // BEQ rs rt label(pc+offset<<2)
        int rs = *iter;
        iter++;
        int rt = *iter;
        iter++;
        int offset = *iter;

        unsigned int op_bit = (0x4 << 26);
        unsigned int rs_bit = ((unsigned int)rs << 21);
        unsigned int rt_bit = ((unsigned int)rt << 16);
        unsigned int offset_bit = offset & 0xffff; // 下位16bit のみ

        unsigned int code = op_bit | rs_bit | rt_bit | offset_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == BNE) { // BNE rs rt label(pc+offset<<2)
        int rs = *iter;
        iter++;
        int rt = *iter;
        iter++;
        int offset = *iter;

        unsigned int op_bit = (0x5 << 26);
        unsigned int rs_bit = ((unsigned int)rs << 21);
        unsigned int rt_bit = ((unsigned int)rt << 16);
        unsigned int offset_bit = offset & 0xffff; // 下位16bit のみ

        unsigned int code = op_bit | rs_bit | rt_bit | offset_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == J) { // J label
        int addr = *iter;

        unsigned int op_bit = (0x2 << 26);
        unsigned int addr_bit = addr;

        unsigned int code = op_bit | addr_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == INB) { // INB rd
        int rd = *iter;

        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x0;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == IN) { // IN rd
        int rd = *iter;

        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x0;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == OUTB) { // OUTB rs
        int rs = *iter;

        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x0 << 6);
        unsigned int funct_bit = 0x1;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == OUT) { // OUT rs
        int rs = *iter;

        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x1;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == INF) { // INF rd
        int rd = *iter;

        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = 0x0;
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (0x3 << 6);
        unsigned int funct_bit = 0x2;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == OUTB) { // OUTB rs
        int rs = *iter;

        unsigned int op_bit = (0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = (03 << 6);
        unsigned int funct_bit = 0x3;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == JR) { // JR rs
        int rs = *iter;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x9;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == JAL) { // JAL label (next addr is line_num*4)
        int addr = *iter;

        unsigned int op_bit = (0x3 << 26);
        unsigned int addr_bit = addr;

        unsigned int code = op_bit | addr_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == JALR) { // JALR rd, rs
        int rd = *iter;
        iter++;
        int rs = *iter;
        iter++;

        unsigned int op_bit = 0x0;
        unsigned int rd_bit = ((unsigned int)rd << 21);
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x9;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == NOP) { // nop
        unsigned int code = 0x0;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else if (opecode == OUT) { // output 未対応
        int rs = *iter;
        iter++;
        unsigned int op_bit = ((unsigned int)0x3F << 26);
        unsigned int rd_bit = 0x0;
        unsigned int rs_bit = ((unsigned int)rs << 16);
        unsigned int rt_bit = 0x0;
        unsigned int shamt_bit = 0x0;
        unsigned int funct_bit = 0x0;

        unsigned int code =
            op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

        if (*log_level >= DEBUG) {
            printf("hex(16):%08x\tbinary:", code);
            print_binary(code);
            printf("\n");
        }

        machine_code_map.push_back(code);

    } else {
        line_num++;
    }
}

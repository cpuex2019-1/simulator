// loader.cpp:
//

#include "loader.h"
#include <iostream>
#include <regex>
#include <string>
#include <vector>
using namespace std;

// constructor
loader::loader(const char *fname) {
    input.open(fname);
    if (!input) {
        cerr << "can't open file " << fname << endl;
        exit(1);
    }
    line_num = 0; // reset line number
    end_line_num = load_file();
    vector<string> first;
    first.insert(first.begin(), "program begin hear");
    program_map.insert(program_map.begin(), first); // ignore number 0
}

// destructor
loader::~loader() {
    if (input.is_open())
        input.close();
}

int loader::load_file() {
    string linebuf;
    while (!input.eof()) {
        line_num++; // increment before processing the line
        getline(input, linebuf);
        load_line(linebuf);
    }
    return line_num;
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
    regex label_pattern("^[\\t ]*(?:([A-Za-z][\\w.]*)[:])?[\\t ]*");
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

    program_map.push_back(code);
}

// public

int loader::get_line_num_by_label(string label) {
    auto it = label_map.find(label);
    if (it == label_map.end()) {
        cout << "not found label: " << label << endl;
        exit(1);
    } else {
        return label_map[label];
    }
}
vector<string> loader::get_program_by_label(string label) {
    int line_num_of_label = get_line_num_by_label(label);
    return program_map[line_num_of_label];
}

vector<string> loader::get_program_by_line_num(int l_num) {
    return program_map[l_num];
}

void loader::print_label_map() {
    cout << "label_map\n";
    for (auto itr = label_map.begin(); itr != label_map.end(); ++itr) {
        cout << "\t" << itr->first << ":\t" << itr->second << "\n";
    }
}
void loader::print_program_map() {
    cout << "program_map\n";
    int line = 0;
    for (auto itr = program_map.begin(); itr != program_map.end(); ++itr) {
        if (line != 0) {
            cout << "\t" << line << ":\t";
            for (auto itr_str = itr->begin(); itr_str != itr->end();
                 ++itr_str) {
                cout << *itr_str << "\t";
            }
            cout << endl;
        }
        line++;
    }
}

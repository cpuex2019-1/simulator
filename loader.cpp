// loader.cpp:
//

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include "loader.h"
using namespace std;



// constructor
loader::loader(const char *fname){
    input.open(fname);
    if (!input) {
        cerr << "can't open file " << fname << endl;
        exit(1);
    }
    line_num = 0;	// reset line number
    load_file();
}

// destructor
loader::~loader()
{
    if (input.is_open())
	input.close();
}

void loader::load_file(){
    string linebuf;
    while(!input.eof()){
        line_num++;		// increment before processing the line
        getline(input, linebuf);
        load_line(linebuf);
    }
}

// load line
void loader::load_line(string line){
    regex label_pattern("^[\\t ]*(?:([A-Za-z][\\w.]*)[:])?");
    sregex_token_iterator iter(line.begin(), line.end(), label_pattern, {1,-1}); // group1: label, 残り: コード
    sregex_token_iterator end;
    string label_str = iter->str();
    iter++;
    string res_str = "" ;
    for(; iter!=end; iter++) {
             res_str = res_str + iter->str();
    }

    cout << "label: "<<"[" << label_str <<"]"<< "\tres: "<<"[" << res_str <<"]"<< endl;
    if (label_str !=""){
        label_map.insert(std::make_pair(label_str, line_num));
    }
    program_map.insert(std::make_pair(line_num, res_str));
}


// public

string loader::get_program_by_label(string label){
    auto it = label_map.find(label);
    if (it == label_map.end() ) {
        cout << "not found label" << endl;
        exit(1);
    } else {
        int line_num_of_label = label_map[label];
        return program_map[line_num_of_label];
    }
}

void loader::print_label_map(){
    cout << "label_map\n";
    for(auto itr = label_map.begin(); itr != label_map.end(); ++itr){
        cout  << "\t" << itr->first << " => " << itr->second << "\n";
    }
}
void loader::print_program_map(){
    cout << "program_map\n";
    for(auto itr = program_map.begin(); itr != program_map.end(); ++itr){
        cout << "\t"  << itr->first << " => " << itr->second << "\n";
    }
}

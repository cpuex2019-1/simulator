// loader.h:
//
#ifndef LOADER_H
#define LOADER_H

#include <fstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class loader {
    // program file stream
    ifstream input;
    // current line number
    int line_num;
    // label
    map<string, int> label_map;
    // program
    vector<vector<string>> program_map;
    // load line
    void load_line(string line);
    // load file
    int load_file();

  public:
    int end_line_num;

    // constructor
    loader(const char *fname);
    // destructor
    ~loader();

    int get_line_num_by_label(string label);
    vector<string> get_program_by_label(string label);
    vector<string> get_program_by_line_num(int l_num);

    // loader のデバッグ用
    void print_label_map();
    void print_program_map();
};

#endif
// end of loader.h

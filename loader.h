// loader.h:
//
#ifndef	LOADER_H
#define	LOADER_H

#include <string>
#include <fstream>
#include <map>
#include <vector>
using namespace std;

class loader {
    // program file stream
    ifstream	input;
    // current line number being processed
    int line_num;
    // load line
    void load_line(string line);
    // load file
    void load_file();

public:
    // label
    map<string,int> label_map;
    // program
    map<int, string> program_map;

    // constructor
    loader(const char *fname);
    // destructor
    ~loader();

    string get_program_by_label(string label);

    // loader のデバッグ用
    void print_label_map();
    void print_program_map();
};

#endif
// end of loader.h

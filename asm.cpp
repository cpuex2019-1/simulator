// asm.cpp
//

#include <regex>
using namespace std;



// check regex at https://www.debuggex.com

// match: alphabets
string regex_alphas_str = "[A-Za-z]*";
regex regex_alphas(regex_alphas_str)

// match: white space and tab
string regex_space_str = "[\\t ]";
regex regex_space(regex_space_str)

// match: int
string regex_int_str = "[-+]?[0-9]+";
regex regex_int(regex_int_str);

// match: char ('.')
string regex_char_str = "\'.\'";
regex regex_char(regex_char_str);

// match: register
string regex_reg_str = "\\$(?:zero|at|v[01]|a[0-3]|t[0-9]|s[0-7]|k[0-1]|gp|fp|ra|[1-9]|[1-2][0-9]|3[0-1])";
regex regex_reg(regex_reg_str);

string regex_operand_str =
    "("+
        regex_int_str +
        "|" +
        regex_char_str+
        "|" +
        regex_reg_str+
    ")";
//string regex_operand_str ="("+regex_int_str +"|" +regex_char_str+"|" +regex_reg_str+")"


// match: one line
string line =
    "^"+ regex_space_str + "*" +
    "(?:([A-Za-z][\\w.]*)[:])?" + // Group 1: Label
    "(?:"+
        regex_space_str + "*" +
        "("+  // Group 2: opecode or pseudo-instruction
            "(" + regex_alphas_str + ")" +  // Group 3: opecode
            "|" +
            "(.[[A-Za-z][\\w]*)" + // Group 4: pseudo-instruction
        ")"+
        "((?:"+regex_operand_str + regex_space_str+"+" + "," + regex_space_str+"+)*" + // Group 5~: operand
        regex_operand_str +
    ")?)?"
//string line ="^"+ regex_space_str + "*" +"(?:([A-Za-z][\w.]*)[:])?" + "(?:"+regex_space_str + "*" +"("+  "(" + regex_alphas_str + ")" +  "|" +"(.[[A-Za-z][\w]*)" +")"+"((?:"+regex_operand_str + regex_space_str+"+" + "," + regex_space_str+"+)*" +regex_operand_str+")?)?"

// 正規表現
// ^[\t ]*(?:([A-Za-z][\w.]*)[:])?(?:[\t ]*(([A-Za-z]*)|(.[[A-Za-z][\w]*))(?:([-+]?[0-9]+|'.'|\$(?:zero|at|v[01]|a[0-3]|t[0-9]|s[0-7]|k[0-1]|gp|fp|ra|[1-9]|[1-2][0-9]|3[0-1]))[\t ]+,[\t ]+)*([-+]?[0-9]+|'.'|\$((?:zero|at|v[01]|a[0-3]|t[0-9]|s[0-7]|k[0-1]|gp|fp|ra|[1-9]|[1-2][0-9]|3[0-1])))?)?

// c++ での表記(エスケープが二重に必要になる)
//^[\\t ]*(?:([A-Za-z][\\w.]*)[:])?(?:[\\t ]*(([A-Za-z]*)|(.[[A-Za-z][\\w]*))(?:([-+]?[0-9]+|'.'|\\$(?:zero|at|v[01]|a[0-3]|t[0-9]|s[0-7]|k[0-1]|gp|fp|ra|[1-9]|[1-2][0-9]|3[0-1]))[\\t ]+,[\\t ]+)*([-+]?[0-9]+|'.'|\\$((?:zero|at|v[01]|a[0-3]|t[0-9]|s[0-7]|k[0-1]|gp|fp|ra|[1-9]|[1-2][0-9]|3[0-1])))?)?

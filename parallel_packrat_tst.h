#pragma once

#ifndef INCLUDE_GUARD_PARALLEL_PACKRAT_TST
#define INCLUDE_GUARD_PARALLEL_PACKRAT_TST

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <stdio.h>
#include <thread>

//メモ化表の中身
constexpr auto SUCCESS = 1;
constexpr auto FAILED = -2;
constexpr auto INITIAL_CELL_VALUE = -1;;

//エラー出力
constexpr auto NOT_REGISTERED = -100;
constexpr auto INCOLLECT_ACCESS = -100;

constexpr auto DEFAULT_BLOCK_SIZE = 128;
constexpr auto MAX_WORKERTHREAD = 7;
constexpr auto DEFAULT_BLOCKLINE = 128;  //デフォルトのブロックの横幅





class Memorialize_table {
    std::vector<std::vector<int>> table;
    std::unordered_map<std::string, int> funcname_to_line;  //非終端記号名と行数の対応表
    std::unordered_map<int, std::string> line_to_funcname;//行数から関数ポインタの参照
    int line;   //行数
    int column; //列数
public:
    Memorialize_table();
    void init_table(const int filesize, const std::vector<std::string>& funcnames);
    void add_column(const int add_num);
    int read_cell(const int pos, const std::string& funcname);
    int read_cell(const int pos, const int funcline);
    int write_cell(const int& pos, const std::string& funcname, const int value);
    std::string get_funcname_from_line(int line);
    int get_column();
    int get_line();
    std::vector<std::vector<int>> get_table();
    std::unordered_map<std::string, int> get_funcname_to_line();
    std::unordered_map<int, std::string> get_line_to_funcname();

private:
    int init_maps(const std::vector<std::string>& funcnames);
        
};

class Strategy_var {  
    std::unordered_map<std::string, int> success_count_runtime;
    int input_num_of_char;  //入力サイズ
    std::vector<int> newline_positions;   //入力文字列の改行位置
    int rightmost_pos;           //最右のブロック
public:
    Strategy_var();

    void init_strategy_var(std::vector<std::string>& funcnames);
    void add_success_count_runtime(std::string funcname, int used_count);
    void add_input_num_of_char(int new_char);
    void push_back_newline(int newline_pos);
    void set_rightmost_pos(int pos);
    std::unordered_map<std::string, int> get_success_count_runtime();
    int get_input_num_of_char();
    std::vector<int> get_newline_positions();
    int get_rightmost_pos();

};


class Block {
    int upper_left_x;
    int upper_left_y;
    int lower_light_x;
    int lower_light_y;
    int latest_memo_x;  //最後に記入したメモかひょうの内容(添え字ではない)
    int latest_memo_y;  //最後に記入したメモかひょうの添え字
public:
    Block();
    bool sellect_cell(int& next_x, int& next_y);
    void strategy1();
    void strategy2();
    void strategy3();
    bool allocation();
    void set_latest(int x, int y);
};

int woeker_thread(bool is_mainthread);
int main_thread(const std::string& input_filename,
    const std::vector<std::string>& _funcnames,
    const std::unordered_map<std::string, int>& _appearance_in_syntax);

namespace no_exclusion {
    extern std::vector<std::string> funcnames;    //解析関数リスト
    extern std::unordered_map<std::string, int> appearance_in_syntax;
    //・構文規則内の各非終端記号の出現数
    extern std::string input;  //解析文字列     
    extern Memorialize_table memorialize_table;            //メモ化表
    extern Strategy_var strategy_var;
};




#endif //INCLUDE_GUARD_PARALLEL_PACKRAT_TST






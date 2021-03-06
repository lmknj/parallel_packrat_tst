#pragma once

#ifndef INCLUDE_GUARD_PARSER

#define INCLUDE_GUARD_PARSER
#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include<unordered_map>
//#include "parallel_packrat_tst.h"

constexpr auto PARSING_START = false;
constexpr auto PARSING_FINISH = true;

//解析失敗処理
bool parsing_failed(const int pos, const std::string& funcname);
bool parsing_failed(const int pos, const std::string& funcname, std::string print);

bool parsing_success(const int pos, const std::string& funcname, const int value,
	std::unordered_map<std::string, int>& success_runtime);

bool check_memo(const int pos, const std::string& funcname,
	std::unordered_map<std::string, int>& success_runtime);

//解析関数の呼び出し
bool parse(const std::string& funcname, int& pos,
	const std::string& input, std::unordered_map<std::string, int>& success_runtime);
bool parse_mainthread(int& pos, const std::string& input,
	std::unordered_map<std::string, int>& success_runtime);


//解析関数
bool A(int& pos, const std::string& input,
	std::unordered_map<std::string, int>& success_runtime);
bool B(int& pos, const std::string& input,
	std::unordered_map<std::string, int>& success_runtime);
bool C(int& pos, const std::string& input,
	std::unordered_map<std::string, int>& success_runtime);

#endif //INCLUDE_GUARD_PARSER
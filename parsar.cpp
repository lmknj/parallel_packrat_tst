#include "parser.h"
#include "parallel_packrat_tst.h"


//演算子順位の構文解析器
/*

A	←　B＋B / B
B	←　C*C / C
C   ←　[1-9]+ / (A)

*/
//メモ化表どうやって作るか問題
//ランダムアクセス必須
//二次元配列：関数名でアクセスしたい
//しかし順序関係が欲しい
inline bool parsing_failed(const int pos, const std::string& funcname) {
	no_exclusion::memorialize_table.write_cell(pos, funcname, FAILED);
	return false;	//FAILEDではない
}


bool parsing_failed(const int pos, const std::string& funcname, std::string print) {
	std::cout << "failed:" << print << " init_pos:" << pos << std::endl;
	no_exclusion::memorialize_table.write_cell(pos, funcname, FAILED);
	return false;	//FAILEDではない
}
bool parsing_success(const int pos, const std::string& funcname, const int value,
				std::unordered_map<std::string, int>& success_runtime){
	std::cout << "success " << funcname << " init_pos:" << pos << 
				" value:" << value << std::endl;
	no_exclusion::memorialize_table.write_cell(pos, funcname, value);
	success_runtime[funcname] += 1;

	return true;	//SUCCESSではない
}

bool check_memo(const int pos, const std::string& funcname,
	std::unordered_map<std::string, int>& success_runtime) {
	//trueで解析しない
	//falseで解析開始
	std::cout << "called " << funcname << std::endl;
	switch (no_exclusion::memorialize_table.read_cell(pos, funcname)) {
	case INITIAL_CELL_VALUE:
		return PARSING_START;
	case FAILED:
		//失敗時は何もしない
		break;
	case INCOLLECT_ACCESS:
		abort();
		break;
	default:
		//値を追加する
		success_runtime[funcname] += 1;
		break;

	}
	return PARSING_FINISH;


}


//ハッシュ計算:数値→文字列→数値：きっしょ

bool parse(const std::string& funcname, int& pos,
	const std::string& input, std::unordered_map<std::string, int>& success_runtime) {
	if (funcname == "C")C(pos, input, success_runtime);
	else if (funcname == "B")C(pos, input, success_runtime);
	else if (funcname == "A")A(pos, input, success_runtime);
	else abort();
	return true;
}


bool parse_mainthread(int & pos,const std::string& input, 
		std::unordered_map<std::string, int>& success_runtime) {
	A(pos, input, success_runtime);
	
	return true;
}



bool A(int& pos, const std::string& input,
	std::unordered_map<std::string, int>& success_runtime) {
	if (check_memo(pos, __func__, success_runtime))return true;
	int init_pos = pos;

	if (!B(pos, input, success_runtime))return parsing_failed(init_pos, __func__,"A_1");
	
	if (input[pos] == '+')++pos;
	else return parsing_failed(init_pos, __func__, "A_2");
	
	if (!B(pos, input, success_runtime))return parsing_failed(init_pos, __func__, "A_3");



	return parsing_success(init_pos, __func__, pos, success_runtime);


}


bool B(int& pos, const std::string& input,
	std::unordered_map<std::string, int>& success_runtime) {
	if (check_memo(pos, __func__, success_runtime))return true;
	int init_pos = pos;

	if (!C(pos, input, success_runtime))return parsing_failed(init_pos, __func__, "B_1");
	
	if (input[pos] == '*')++pos;
	else return parsing_failed(init_pos, __func__, "B_2");
	
	if (!C(pos, input, success_runtime))return parsing_failed(init_pos, __func__, "B_3");
	//tasikani

	return parsing_success(init_pos, __func__, pos, success_runtime);

}

bool C(int& pos, const std::string& input,
	std::unordered_map<std::string, int>& success_runtime) {
	//C   ←　[1 - 9] + / (A)
	if (check_memo(pos, __func__, success_runtime))return true;
	int init_pos = pos;

	//文字列が数字である限り繰り返す

	if (isdigit(static_cast<unsigned char>(input[pos]))) {//1文字目判定
		++pos;
		while (isdigit(static_cast<unsigned char>(input[pos]))) {//2文字目以降
			++pos;
		}
	}
	else if (input[pos] == '(') {
		pos = init_pos;	//必要ない
		++pos;

		if (!A(pos, input, success_runtime))return parsing_failed(init_pos, __func__, "C_1");

		if (input[pos] == ')')++pos;
		else return parsing_failed(init_pos, __func__, "C_1");
	}
	else return parsing_failed(init_pos, __func__, "C_1");



	return parsing_success(init_pos, __func__, pos, success_runtime);
}

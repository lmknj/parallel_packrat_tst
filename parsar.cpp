#include "parser.h"
#include "parallel_packrat_tst.h"


//���Z�q���ʂ̍\����͊�
/*

A	���@B�{B / B
B	���@C*C / C
C   ���@[1-9]+ / (A)

*/
//�������\�ǂ�����č�邩���
//�����_���A�N�Z�X�K�{
//�񎟌��z��F�֐����ŃA�N�Z�X������
//�����������֌W���~����
inline bool parsing_failed(const int pos, const std::string& funcname) {
	no_exclusion::memorialize_table.write_cell(pos, funcname, FAILED);
	return false;	//FAILED�ł͂Ȃ�
}


bool parsing_failed(const int pos, const std::string& funcname, std::string print) {
	std::cout << "failed:" << print << " init_pos:" << pos << std::endl;
	no_exclusion::memorialize_table.write_cell(pos, funcname, FAILED);
	return false;	//FAILED�ł͂Ȃ�
}
bool parsing_success(const int pos, const std::string& funcname, const int value,
				std::unordered_map<std::string, int>& success_runtime){
	std::cout << "success " << funcname << " init_pos:" << pos << 
				" value:" << value << std::endl;
	no_exclusion::memorialize_table.write_cell(pos, funcname, value);
	success_runtime[funcname] += 1;

	return true;	//SUCCESS�ł͂Ȃ�
}

bool check_memo(const int pos, const std::string& funcname,
	std::unordered_map<std::string, int>& success_runtime) {
	//true�ŉ�͂��Ȃ�
	//false�ŉ�͊J�n
	std::cout << "called " << funcname << std::endl;
	switch (no_exclusion::memorialize_table.read_cell(pos, funcname)) {
	case INITIAL_CELL_VALUE:
		return PARSING_START;
	case FAILED:
		//���s���͉������Ȃ�
		break;
	case INCOLLECT_ACCESS:
		abort();
		break;
	default:
		//�l��ǉ�����
		success_runtime[funcname] += 1;
		break;

	}
	return PARSING_FINISH;


}


//�n�b�V���v�Z:���l�������񁨐��l�F��������

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
	//C   ���@[1 - 9] + / (A)
	if (check_memo(pos, __func__, success_runtime))return true;
	int init_pos = pos;

	//�����񂪐����ł������J��Ԃ�

	if (isdigit(static_cast<unsigned char>(input[pos]))) {//1�����ڔ���
		++pos;
		while (isdigit(static_cast<unsigned char>(input[pos]))) {//2�����ڈȍ~
			++pos;
		}
	}
	else if (input[pos] == '(') {
		pos = init_pos;	//�K�v�Ȃ�
		++pos;

		if (!A(pos, input, success_runtime))return parsing_failed(init_pos, __func__, "C_1");

		if (input[pos] == ')')++pos;
		else return parsing_failed(init_pos, __func__, "C_1");
	}
	else return parsing_failed(init_pos, __func__, "C_1");



	return parsing_success(init_pos, __func__, pos, success_runtime);
}

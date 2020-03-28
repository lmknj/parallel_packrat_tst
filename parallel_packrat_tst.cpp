#include "parallel_packrat_tst.h"
#include "parser.h"

std::vector<std::string> no_exclusion::funcnames;    //解析関数リスト
std::unordered_map<std::string, int> no_exclusion::appearance_in_syntax;
//・構文規則内の各非終端記号の出現数
std::string no_exclusion::input;  //解析文字列     
Memorialize_table no_exclusion::memorialize_table;            //メモ化表
Strategy_var no_exclusion::strategy_var;

//メモ化表クラス
//とりあえずシングルスレッド用に作る
Memorialize_table::Memorialize_table() {
    line = 0;
    column = 0;
}
    
void Memorialize_table::init_table(const int filesize, 
                const std::vector<std::string>& funcnames) {
    //構文規則から列数を決定
    //入力文字列未定だがとりあえず領域確保を
    //取り合えずファイルサイズからvectorのサイズを決定
    column = 0;
    line = init_maps(funcnames); //行数の取得
    //vector<int> one_column(line);            //1行のサイズ
    //領域確保
    table.reserve(filesize * line); //これ動くの？
    
}

void Memorialize_table::add_column(const int add_num) {
    //add_num列メモ化表を追加
    for (int i = 0; i < add_num; ++i) {
        table.emplace_back(line);
        for (auto& a : table[column])a = INITIAL_CELL_VALUE;
        ++column;
        //要素数lineのvector<int>を追加して各要素を-1で初期化
    }
}

int Memorialize_table::read_cell(const int pos, const std::string& funcname) {
    //unordered_map<string, int> funcname_to_line;
    if (funcname_to_line.find(funcname) == funcname_to_line.end()) {
        return NOT_REGISTERED;  //解析関数が登録されていない
    }
    if (pos < 0 || column <= pos)return INCOLLECT_ACCESS;
    return table[pos][funcname_to_line[funcname]];  //要素を返す
}

int Memorialize_table::read_cell(const int pos, const int funcline) {
    //unordered_map<string, int> funcname_to_line;
    if (funcline < 0 || line <= funcline ||
                    pos < 0 || column <= pos)return INCOLLECT_ACCESS;


    return table[pos][funcline];  //要素を返す
}

int Memorialize_table::write_cell(const int& pos, const std::string& funcname, const int value) {
    //メモ化表に値を書き込む
    //書き込まれていたらposにその値を入力→あっぶねえ
    if (funcname_to_line.find(funcname) == funcname_to_line.end()) {
        return NOT_REGISTERED;  //解析関数が登録されていない
    }
    if (pos < 0 || column <= pos)return INCOLLECT_ACCESS;

    table[pos][funcname_to_line[funcname]] = value;  //書き込み
    return SUCCESS;
}

std::string Memorialize_table::get_funcname_from_line(int line){
    if (line_to_funcname.find(line) == line_to_funcname.end()) {
        abort();
        return "";  //解析関数が登録されていない
    }
    return line_to_funcname[line];   //関数名を返す
}

int Memorialize_table::get_column() { return column; }

int Memorialize_table::get_line() { return line; }

std::vector<std::vector<int>> Memorialize_table::get_table() {return table;};
std::unordered_map<std::string, int> Memorialize_table::get_funcname_to_line() {
    return funcname_to_line;
}
std::unordered_map<int, std::string> Memorialize_table::get_line_to_funcname() {
    return line_to_funcname;
}



int Memorialize_table::init_maps(const std::vector<std::string>& funcnames) {
    //2つのmapの初期化
    //構文規則の非終端記号リスト(String)から値を受け取る
    //サイズを返す
    int line = 0;
    for (const auto& funcname : funcnames) {
        funcname_to_line[funcname] = line;  //関数と行の対応
        line_to_funcname[line] = funcname;  //逆引き
        ++line;
    }
    //行数から関数を呼び出せないといけない

    return line;
}


//strategy_var
Strategy_var::Strategy_var() {
    success_count_runtime = {};
    input_num_of_char = 0;  //入力サイズ
    newline_positions = {};   //入力文字列の改行位置
    rightmost_pos = 0;
}
        
void Strategy_var::init_strategy_var(std::vector<std::string>& funcnames ) {
    for (auto& funcname : funcnames)
        success_count_runtime[funcname] = 0;

    input_num_of_char = 0;
    //newline_positions.push_back(0);
    rightmost_pos = 0;
}
//以下の関数で制御（RO排他）
void Strategy_var::add_success_count_runtime(std::string funcname, int used_count) {
    if (success_count_runtime.find(funcname) == success_count_runtime.end()) {
        abort();
    }
    success_count_runtime[funcname] += used_count;
}

void Strategy_var::add_input_num_of_char(int new_char) {
    input_num_of_char += new_char;

}

void Strategy_var::push_back_newline(int newline_pos) {
    newline_positions.push_back(newline_pos);
}

void Strategy_var::set_rightmost_pos(int pos) {
    rightmost_pos = pos;
}

std::unordered_map<std::string, int> Strategy_var::get_success_count_runtime() {
    return success_count_runtime;
}
int Strategy_var::get_input_num_of_char() {
    return input_num_of_char;
}
std::vector<int> Strategy_var::get_newline_positions() {
    return newline_positions;
};
int Strategy_var::get_rightmost_pos() {
    return rightmost_pos;
}



//名前空間っていうかインスタンスにまとめた方がいいんじゃねこれ
//で変更が排他的ってことを保証したほうがいいでしょこれ
//→まああとでいいわ

Block::Block() {
    upper_left_x = 0;
    upper_left_y = 0;
    lower_light_x = 0;
    lower_light_y = 0;
    latest_memo_x = 0;
    latest_memo_y = 0;
}

bool Block::sellect_cell(int& next_x, int& next_y) {
    //latest_memo_xの隣はいいけどyのはどうすっかな
    //記録できそうなセルを検索
    for (int x = latest_memo_x; x < lower_light_x; ++x) {
        for (int y = upper_left_y; y < lower_light_y; ++y) {
            if (no_exclusion::memorialize_table.read_cell(x, y)
                == INITIAL_CELL_VALUE) {
                next_x = x;
                next_y = y;
                return true;
            }

        }
    }
    //書くべきセルがない
    return false;
}
void Block::strategy1() {
    //均等分割

}

void Block::strategy2() {
    //左下
}


void Block::strategy3() {
    //集中割り当て

}
bool Block::allocation() {
    //ブロックの割り当て
    //戦略変数の参照はここで行う:参照は排他制御無視でしくよろ
    //とりあえず
    int remain_domain = no_exclusion::memorialize_table.get_line() -
        no_exclusion::strategy_var.get_rightmost_pos();

    if (remain_domain <= 0) return false;

    upper_left_x = no_exclusion::strategy_var.get_rightmost_pos();     //戦略変数の参照
    upper_left_y = 0;

    //デフォルトサイズ割り当てたらサイズオーバーする？
    lower_light_x = (remain_domain < DEFAULT_BLOCK_SIZE) ?
        upper_left_x + remain_domain :
        upper_left_x + DEFAULT_BLOCK_SIZE;

    lower_light_y = no_exclusion::memorialize_table.get_column() - 1;

    no_exclusion::strategy_var.set_rightmost_pos(lower_light_x + 1);//戦略変数の更新

    latest_memo_x = upper_left_x;
    latest_memo_y = upper_left_y;
    return true;
}

void Block::set_latest(int x, int y) {
    latest_memo_x = x;
    latest_memo_y = y;
}




int worker_thread(bool is_mainthread = false){

    //戦略変数(ローカル)の宣言
    Block block;
    int pos=0;   //解析開始位置
    int line=0;   //解析開始位置
    //int answer_pos; //記録結果
    std::unordered_map<std::string, int> success_count_runtime;
        //初期化
    if (is_mainthread) {
        success_count_runtime = no_exclusion::strategy_var.get_success_count_runtime();
        parse(
            no_exclusion::memorialize_table.get_funcname_from_line(line),
            pos, no_exclusion::input, success_count_runtime);
    }else{
        while (block.allocation()) {//ブロックの割り当て
            //ブロックは基本的には長方形
            while (block.sellect_cell(pos, line)) {//セルの指定→いいのがなければ終了
                //構文解析：関数を呼び出す
                //解析関数：posと入力文字列を渡す:最終結果用の引数は必要なし
                //あっちはメモかどうやんの

                parse(
                    no_exclusion::memorialize_table.get_funcname_from_line(line),
                    pos, no_exclusion::input, success_count_runtime);




                //戦略変数(ローカル)の更新：これ解析中にやらないとじゃね

                //引数で渡すしかないか
                //最後に記録したセルに更新
                block.set_latest(pos, line);   //lineはsellect_cellから値変更なし
            }

            //戦略変数(グローバル)の更新→排他制御
            for (auto& success : success_count_runtime) {
                no_exclusion::strategy_var.add_success_count_runtime(
                    success.first, success.second);
            }

        }
    }
    return 0;
}



int main_thread(const std::string& input_filename,
            const std::vector<std::string>& _funcnames,
            const std::unordered_map<std::string, int>& _appearance_in_syntax
            ) {
    
    //戦略変数の初期化
    std::string input = "";  //解析文字列     
    no_exclusion::funcnames = _funcnames; //解析関数リスト     //moveすべき？
    no_exclusion::appearance_in_syntax = _appearance_in_syntax;//moveすべき？ 
                        //・構文規則内の各非終端記号の出現数 
    no_exclusion::strategy_var.init_strategy_var(no_exclusion::funcnames);
    
    //ファイルサイズ
    std::ifstream infile(input_filename);
    int file_size = 100;
    //メモ化表の初期化
    no_exclusion::memorialize_table.init_table(file_size, no_exclusion::funcnames);
    
    //この辺でワーカスレッドを生成→どうやって待たせるんだい
    std::string line;
    int column = 0;                 //メモ化表の行数
    int num_of_workerthread = 0;    //現在のワーカースレッド数
    while (std::getline(infile, line)) {//入力の受付とメモ化表の追加
        no_exclusion::input += line;  //解析可能文字列の追加
        no_exclusion::strategy_var.add_input_num_of_char(line.length());
        no_exclusion::memorialize_table.add_column(line.size());
        //メモ化表を生成
        column = no_exclusion::memorialize_table.get_column();
        no_exclusion::strategy_var.push_back_newline(column);
        //改行位置を記録

        if (column >= DEFAULT_BLOCKLINE * (num_of_workerthread+1) &&
            num_of_workerthread <= MAX_WORKERTHREAD) {
            //スレッドの生成
        }

    }

    //戦略変数の更新
    


    //入力全体の解析:
    worker_thread(true);
    return 0;

}

void printall() {
    //終了後の値を全表示
    //メモ化表の中身と戦略変数
    //extern std::vector<std::string> funcnames;    //解析関数リスト
    //extern std::unordered_map<std::string, int> appearance_in_syntax;
    //extern std::string input;  //解析文字列     
    std::cout << "input:" << no_exclusion::input << std::endl;
    //extern Memorialize_table memorialize_table;            //メモ化表
    std::cout << "memorialize_table member:" << std::endl;
    //std::vector<std::vector<int>> table;
    //std::unordered_map<std::string, int> funcname_to_line;  //非終端記号名と行数の対応表
    //std::unordered_map<int, std::string> line_to_funcname;//行数から関数ポインタの参照
    //int line;   //行数
    //int column; //列数

    int line = no_exclusion::memorialize_table.get_line();
    int colmn = no_exclusion::memorialize_table.get_column();
    std::cout << "line:" << no_exclusion::memorialize_table.get_line() << std::endl;
    std::cout << "colmn:" << no_exclusion::memorialize_table.get_column() << std::endl;

    std::cout << "table:" << std::endl;
    for (int lin = 0; lin < line; ++lin) {
        for (int col = 0; col < colmn; ++col) {
            std::cout << no_exclusion::memorialize_table.read_cell(col, line) << "\t";
        }
        std::cout << std::endl;
    }

    //std::vector<std::vector<int>> Memorialize_table::get_table() { return table; };
    std::cout << "funcname_to_line:" << std::endl;
    for (auto& funcname_and_line : no_exclusion::memorialize_table.get_funcname_to_line()) {
        std::cout << funcname_and_line.first << ", "<< funcname_and_line.second << "\t";
    }
    std::cout << std::endl;

    std::cout << "line_to_funcname:" << std::endl;
    for (auto& funcname_and_line : no_exclusion::memorialize_table.get_line_to_funcname()) {
        std::cout << funcname_and_line.first << ", " << funcname_and_line.second << "\t";
    }
    std::cout << std::endl;
    
    //extern Strategy_var strategy_var;
    std::cout << "success_count_runtime:" << std::endl;
    for (auto& success_count : no_exclusion::strategy_var.get_success_count_runtime()) {
        std::cout << success_count.first << ", " << success_count.second << "\t";
    }
    std::cout << std::endl;

    //input_num_of_char    
    std::cout << "input_num_of_char:" << 
            no_exclusion::strategy_var.get_input_num_of_char() << std::endl;
    
    //int rightmost_pos;
    std::cout << "rightmost_pos:" <<
        no_exclusion::strategy_var.get_rightmost_pos() << std::endl;

    //std::vector<int> newline_positions;   //入力文字列の改行位置
    std::cout << "newline_positions:" << std::endl;
    for (auto& newline_position : no_exclusion::strategy_var.get_newline_positions()) {
        std::cout << newline_position;
    }
    std::cout << std::endl;

    

}

int main()
{
   std::string input_filename = "input.txt";
   std::vector<std::string> funcnames{ "A", "B", "C" };
   std::unordered_map<std::string, int> appearance_in_syntax{
       {"A",1 }, { "B", 2 }, { "C", 2 }
   };
   //解析関数リストと出現頻度は計算する必要なし
   //構文規則も入力する必要がない
    main_thread(input_filename, funcnames, appearance_in_syntax);

    std::cout << "ok!"<<std::endl;
    printall();
}

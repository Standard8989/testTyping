// testTyping.exe {wordsFile}
// 1ワードの構成(暫定)
// Englishファイル
//  単語の意味
//  単語(日本語)
//  単語(ローマ字)
// Typingファイル
//  単語(日本語)
//  単語(ローマ字)
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <conio.h>
using namespace std;

#define ESCAPE_CODE 0x1B
#define ARGS_NUM 2

// ソフトの実行モード
enum class Mode
{
    practice,
    marathon,
    speed,
    test
};

enum class fileType
{
    english,
    typing
};

// 一単語の構成
struct WordStruct
{
    string wordMeaning;
    string word;
    string wordRoma;
};

void readWordsFile(ifstream&, vector<WordStruct>&, fileType);
void removeLFCR(vector<WordStruct>&);
Mode setMode();

int main(int argc, char** argv) {
    Mode mode;
    vector<WordStruct> wordsList;
    ifstream file;
    string temp;

    system("chcp 65001 > nul");
    srand(time(NULL));

    // 引数チェック
    if (argc != ARGS_NUM) {
        cout << "引数の個数を確認してください" << endl;
        return 0;
    }

    // ファイルが存在しない場合は新たに指定,存在する場合はそのままファイルオープン
    if (!filesystem::exists({ argv[1] })) {
        do {
            cout << "ファイルが存在しません" << endl;
            cout << "ファイルパスを指定してください" << endl;
            cin >> temp;
        } while (!filesystem::exists(temp));
    }

    file.open(argv[1]);
    if (!file) {
        cout << "file error" << endl;
        return 0;
    }

    if (!getline(file, temp)) {
        cout << "file is empty" << endl;
        return 0;
    }

    readWordsFile(file, wordsList);

    removeLFCR(wordsList);

    mode = setMode();

    switch ((int)mode) {
    case (int)Mode::marathon:
        break;
    case (int)Mode::practice:
        break;
    case (int)Mode::speed:
        break;
    case (int)Mode::test:
        break;
    }
}

void readWordsFile(ifstream& file, vector<WordStruct>& wordsList, fileType ftype) {
    WordStruct wordTemp;
    if (ftype == fileType::english) {
        while (true) {
            if (!(getline(file, wordTemp.wordMeaning) &&
                getline(file, wordTemp.word) &&
                getline(file, wordTemp.wordRoma))) {
                cout << "file is broken" << endl;
                return;
            }
            wordsList.push_back(wordTemp);
        }
    }
    else {
        while(true){
            if(!(getline(file, wordTemp.word) && getline(file, wordTemp.wordRoma))){
                cout << "file is broken" << endl;
                return;
            }
            wordTemp.wordMeaning = "";
            wordsList.push_back(wordTemp);
        }
    }
    cout << "succeed to read" << endl;
}

void removeLFCR(vector<WordStruct>& wordsList) {

}

Mode setMode() {

}
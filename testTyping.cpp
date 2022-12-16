// testTyping.exe {wordsFile}
// 1ワードの構成(暫定)
// Englishファイル
//  単語の意味
//  単語
// Typingファイル
//  単語
//  単語(ローマ字)
// Testファイル
//  単語の意味
//  単語
//  単語(ローマ字)
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <conio.h>
using namespace std;

#define ESCAPE_CODE 0x1B
#define ARGS_NUM 2
#define PRACTICE_NUM 30
#define F_SELECT_MODE 0x01u
#define F_RETRY 0x02u
#define F_EXIT 0x04u
#define F_ESCAPE 0x08u
#define F_ERR 0x10u
#define F_NORMAL_END 0x20u

// ソフトの実行モード
enum class Mode {
    practice,
    marathon,
    speed,
    test,
    wordConfig
};

// 一単語の構成
struct WordStruct {
    string wordMeaning;
    string word;
    string wordRoma;
};

struct TypingFuncConfig {

};

struct TypingFuncReturnValue {
    int flag = 0;
    int correctTypeCount = 0;
    int incorrectTypeCount = 0;
};

class TypingScore {
public:
    int wordsCount;
    int correctTypeCount;
    int incorrectTypeCount;
    time_t startTime;
    time_t endTime;

    TypingScore(): wordsCount(0), correctTypeCount(0), incorrectTypeCount(0) { }

    coutScore() {

    }
};

bool readWordsFile(ifstream&, vector<WordStruct>&);
void removeLFCR(vector<WordStruct>&);
void removeLFCRStr(string&);
Mode setMode();
void startTyping(vector<WordStruct>&, Mode&, string);
int marathonTyping(const vector<WordStruct>&);
int practiceTyping(const vector<WordStruct>&);
void typeWord(WordStruct, time_t, TypingFuncConfig&, TypingFuncReturnValue&);

int main(int argc, char** argv) {
    Mode mode;
    vector<WordStruct> wordsList;
    ifstream file;
    string temp;

    system("chcp 65001 > nul");
    srand(time(NULL));

    // 引数チェック
    if (argc != ARGS_NUM) {
        cout << "引数を確認してください" << endl;
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

    if (!readWordsFile(file, wordsList)) {
        return 0;
    }

    mode = setMode();
    startTyping(wordsList, mode, argv[1]);

    return 0;
}

// 単語ファイルの読み込み
// 成功時はtrue, 失敗時はfalseを返す
bool readWordsFile(ifstream& file, vector<WordStruct>& wordsList) {
    WordStruct wordTemp;
    string temp;

    if (getline(file, temp)) {
        if (temp == "English" || temp == "english") {
            while (true) {
                if (!(getline(file, wordTemp.wordMeaning) &&
                    getline(file, wordTemp.wordRoma))) {
                    cout << "file is broken" << endl;
                    return true;
                }
                wordTemp.word = "";
                wordsList.push_back(wordTemp);
            }
        }
        else if (temp == "Typing" || temp == "typing") {
            while (true) {
                if (!(getline(file, wordTemp.word) &&
                    getline(file, wordTemp.wordRoma))) {
                    cout << "file is broken" << endl;
                    return true;
                }
                wordTemp.wordMeaning = "";
                wordsList.push_back(wordTemp);
            }
        }
        else if (temp == "Test" || temp == "test") {
            while (true) {
                if (!(getline(file, wordTemp.wordMeaning) &&
                    getline(file, wordTemp.word) &&
                    getline(file, wordTemp.wordRoma))) {
                    cout << "file is broken" << endl;
                    return true;
                }
                wordsList.push_back(wordTemp);
            }
        }
        else {
            cout << "file format error" << endl;
            cout << "ファイルの形式が選ばれていません" << endl;
            return false;
        }
    }
    else {
        cout << "ファイルが空です" << endl;
        return false;
    }
    removeLFCR(wordsList);

    cout << "succeed to read" << endl;
    return true;
}

// 改行コードを取り除く
void removeLFCR(vector<WordStruct>& wordsList) {
    for (WordStruct i : wordsList) {
        removeLFCRStr(i.wordMeaning);
        removeLFCRStr(i.word);
        removeLFCRStr(i.wordRoma);
    }
}

// 改行コードを取り除く : string
void removeLFCRStr(string& str) {
    if (str[str.length() - 1] == '\n')
        str.erase(str.length() - 1);
    if (str[str.length() - 1] == '\r')
        str.erase(str.length() - 1);
}

// モードの設定
Mode setMode() {
    char input;
    cout << "動作モードを設定します" << endl;
    do {
        cout << "Marathonモードなら 0 or m" << '\n'
            << "Practiceモードなら 1 or p" << '\n'
            << "Speedモードなら 2 or s" << '\n'
            << "Testモードなら 3 or t" << '\n'
            << "単語ファイル関係の設定なら 4 or w を入力してください : ";
        input = cin.get();
        switch (input) {
        case '0':
        case 'm':
            cout << "Marathonモードに設定されました" << endl;
            return Mode::marathon;
        case '1':
        case 'p':
            cout << "Practiceモードに設定されました" << endl;
            return Mode::practice;
        case '2':
        case 's':
            cout << "Speedモードに設定されました" << endl;
            return Mode::speed;
        case '3':
        case 't':
            cout << "Testモードに設定されました" << endl;
            return Mode::test;
        case '4':
        case 'w':
            cout << "単語ファイルの設定を行います" << endl;
            return Mode::wordConfig;
        default:
            cout << "正しい値を入力してください" << endl;
        }
    } while (true);
}

// タイピングスタート
void startTyping(vector<WordStruct>& WordsList, Mode& mode, string wordFilePath) {
    int flag = 0;
    while (true) {
        switch ((int)mode) {
        case (int)Mode::marathon:
            flag = marathonTyping(WordsList);
            break;
        case (int)Mode::practice:
            break;
        case (int)Mode::speed:
            break;
        case (int)Mode::test:
            break;
        case (int)Mode::wordConfig:
            break;
        }

        if (flag == F_EXIT)
            break;
        else if (flag == F_SELECT_MODE)
            mode = setMode();
        else if (flag == F_RETRY)
            continue;
        else if (flag == F_ERR)
            return;
    }
}

// marathonモード本体
int marathonTyping(const vector<WordStruct>& wordsList) {
    TypingScore score;
    WordStruct nowTyping;
    int typeWordIndex = 0;
    TypingFuncConfig cfg;
    TypingFuncReturnValue returnValue;
    int gotCh = 0;

    cout << "キーを入力してスタート" << endl;
    while (!kbhit()) {}
    getch();
    system("cls");

    score.startTime = time(NULL);
    while (true) {
        typeWordIndex = rand() % wordsList.size();
        nowTyping = wordsList.at(typeWordIndex);

        typeWord(nowTyping, time(NULL), cfg, returnValue);
        if (returnValue.flag == F_NORMAL_END) {
            continue;
        }
        else if (returnValue.flag == F_ESCAPE) {
            break;
        }
        else if (returnValue.flag == F_ERR) {
            cout << "エラー発生" << endl;
            return F_ERR;
        }
    }
    score.endTime = time(NULL);

    score.correctTypeCount = returnValue.correctTypeCount;
    score.incorrectTypeCount = returnValue.incorrectTypeCount;
    score.coutScore();

    while (true) {
        cout << "練習を続けますか？(y/n) : " << flush;
        gotCh = getch();
        if (gotCh == 'y' || gotCh == 'Y') {
            return F_RETRY;
        }
        else if (gotCh == 'n' || gotCh == 'N') {
            while (true) {
                cout << "モード選択に戻りますか？戻らない場合はそのままソフトを終了します(y/n) : " << flush;
                gotCh = getch();
                if (gotCh == 'y' || gotCh == 'Y') {
                    return F_SELECT_MODE;
                }
                else if (gotCh == 'n' || gotCh == 'N') {
                    return F_EXIT;
                }
                else {
                    cout << "正しい値を入力してください"
                }
            }
        }
        else {
            cout << "正しい値を入力してください" << endl;
        }
    }
}

// practiceモード本体
int practiceTyping(const vector<WordStruct>& wordsList) {

}

// speedモード本体
int speedTyping(const vector<WordStruct>& wordsList) {

}

// testモード本体
int testTyping(const vector<WordStruct>& wordsList) {

}

// 単語ファイルの設定
int wordConfig(const vector<WordStruct>& wordsList, string wordFilePath) {

}

// タイピング1ワード分
// 引数は左からワード, 関数呼び出し時の時間, 関数に渡す引数をまとめたもの, 戻り値用構造体
void typeWord(WordStruct word, time_t now, const TypingFuncConfig& config, TypingFuncReturnValue& returnValue) {

}
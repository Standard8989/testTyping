// testTyping.exe {wordsFile} -nt
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
#include <cstring>
using namespace std;

#define ESCAPE_CODE 0x1B
#define ARGS_NUM_MIN 2
#define ARGS_NUM_MAX 3
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
    bool isTimeCountOn;
};

struct TypingFuncReturnValue {
    int flag = 0;
    int correctTypeCount = 0;
    int incorrectTypeCount = 0;
};

struct argvOptions {
    bool isTimeCountOn = false;
};


class TypingScore {
public:
    int wordsCount;
    int correctTypeCount;
    int incorrectTypeCount;
    time_t startTime;
    time_t endTime;

    TypingScore(): wordsCount(0), correctTypeCount(0), incorrectTypeCount(0) { }

    void coutScore() {
        time_t countTime = endTime - startTime;
        cout << "score : " << endl;
        cout << '\t' << "タイプミス数 : " << incorrectTypeCount << endl;
        cout << '\t' << "成功タイプ数 : " << correctTypeCount << endl;
        cout << '\t' << "ワード数 : " << wordsCount << endl;
        cout << '\t' << "タイプミス率 (%) : " << (correctTypeCount != 0 ? (double)incorrectTypeCount / correctTypeCount * 100 : 0) << endl;
        cout << '\t' << "タイプ速度 (type/s) : " << (countTime != 0 ? (double)correctTypeCount / countTime : 0) << endl;
        cout << '\t' << "タイプ速度 (wpm) : " << (countTime != 0 ? (double)wordsCount / countTime * 60 : 0) << endl;
        cout << '\t' << "練習時間 (s) : " << countTime << endl;
    }
};

bool readWordsFile(ifstream&, vector<WordStruct>&);
void removeLFCR(vector<WordStruct>&);
void removeLFCRStr(string&);
Mode setMode();
void startTyping(vector<WordStruct>&, Mode&, const argvOptions&);
int marathonTyping(const vector<WordStruct>&, const argvOptions&);
int practiceTyping(const vector<WordStruct>&, const argvOptions&);
void typeWord(const WordStruct&, time_t, const TypingFuncConfig&, TypingFuncReturnValue&);
bool makeWordsFile(vector<WordStruct>&, char*);
void getFileList(filesystem::path, vector<filesystem::directory_entry>&);

int main(int argc, char** argv) {
    Mode mode;
    vector<WordStruct> wordsList;
    ifstream file;
    string temp;
    argvOptions argvList;

    system("chcp 65001 > nul");
    srand(time(NULL));

    // 引数チェック
    if (argc > ARGS_NUM_MAX) {
        cout << "引数の個数が多すぎます" << endl;
        cout << "一部を無視して続行します" << endl;
    }

    // ファイルパスの指定
    if (argc < ARGS_NUM_MIN) {
        cout << "ファイルパスが指定されていません" << endl;
        cout << "ファイルパスを指定してください : " << flush;
        cin >> temp;
        argv[1] = const_cast<char*>(temp.c_str());
    }

    // ファイルが存在しない場合は新たに指定,存在する場合はそのままファイルオープン
    if (!filesystem::exists({ argv[1] })) {
        do {
            cout << "ファイルが存在しません" << endl;
            cout << "ファイルパスを指定してください : " << flush;
            cin >> temp;
        } while (!filesystem::exists(temp));
        argv[1] = const_cast<char*>(temp.c_str());
    }

    if(!makeWordsFile(wordsList, argv[1])) {
        return 0;
    }

    // file.open(argv[1]);
    // if (!file) {
    //     cout << "file error" << endl;
    //     return 0;
    // }

    // if (!readWordsFile(file, wordsList)) {
    //     return 0;
    // }

    // if (argc > 2) {
    //     argvList.isTimeCountOn = strcmp(argv[2], "-nt") ? false : true;
    // }

    mode = setMode();
    startTyping(wordsList, mode, argvList);

    return 0;
}

// 総合的な単語ファイルの作成
// 成功時はtrue, 失敗時はfalseを返す
bool makeWordsFile(vector<WordStruct>& wordsList, char* filePathStr) {
    filesystem::path filePath(filePathStr);
    if(filesystem::is_regular_file(filePath)){
        ifstream file;
        file.open(filePath);
        if(!file) {
            return false;
        }
        if(!readWordsFile(file, wordsList)) {
            return false;
        }
        file.close();
    }
    else if(filesystem::is_directory(filePath)) {
        vector<filesystem::directory_entry> fileList;
        getFileList(filePath, fileList);
        ifstream file;
        for(auto i : fileList) {
            file.open(i.path());
            readWordsFile(file, wordsList);
            file.close();
        }
    }
    else {
        cout << "file read error" << endl;
        cout << "対応していないパスです" << endl;
        return false;
    }

    return true;
}

// accessFile以下のファイルを取得する
void getFileList(filesystem::path accessFile, vector<filesystem::directory_entry>& fileListOutput) {
        for (const auto& fileName : filesystem::directory_iterator(accessFile)) {
            if (fileName.status().type() == filesystem::file_type::regular) {
                fileListOutput.push_back(fileName);
            }
        }
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
    system("cls");
    cout << "動作モードを設定します" << endl;
    do {
        cout << "Marathonモードなら 0 or m" << '\n'
            << "Practiceモードなら 1 or p" << '\n'
            << "Speedモードなら 2 or s" << '\n'
            << "Testモードなら 3 or t" << '\n'
            << "単語ファイル関係の設定なら 4 or w を入力してください : ";
        input = getch();
        cout << endl;
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
void startTyping(vector<WordStruct>& WordsList, Mode& mode, const argvOptions& argvList) {
    int flag = 0;

    while (true) {
        switch ((int)mode) {
        case (int)Mode::marathon:
            flag = marathonTyping(WordsList, argvList);
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
int marathonTyping(const vector<WordStruct>& wordsList, const argvOptions& argvList) {
    TypingScore score;
    WordStruct nowTyping;
    int typeWordIndex = 0;
    TypingFuncConfig cfg;
    TypingFuncReturnValue returnValue;
    int gotCh = 0;
    cfg.isTimeCountOn = argvList.isTimeCountOn;

    cout << "キーを入力してスタート" << endl;
    while (!kbhit()) {}
    getch();
    system("cls");

    score.startTime = time(NULL);
    while (true) {
        typeWordIndex = rand() % wordsList.size();
        nowTyping = wordsList.at(typeWordIndex);

        typeWord(nowTyping, score.startTime, cfg, returnValue);
        if (returnValue.flag == F_NORMAL_END) {
            score.wordsCount++;
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
    cout << "終了しました" << endl;
    score.coutScore();

    while (true) {
        cout << '\n' << "リトライしますか？(y/n) : " << flush;
        gotCh = getch();
        cout << endl;
        if (gotCh == 'y' || gotCh == 'Y') {
            return F_RETRY;
        }
        else if (gotCh == 'n' || gotCh == 'N') {
            while (true) {
                cout << "モード選択に戻りますか？戻らない場合はそのままソフトを終了します(y/n) : " << flush;
                gotCh = getch();
                cout << endl;
                if (gotCh == 'y' || gotCh == 'Y') {
                    return F_SELECT_MODE;
                }
                else if (gotCh == 'n' || gotCh == 'N') {
                    return F_EXIT;
                }
                else {
                    cout << "正しい値を入力してください" << endl;
                }
            }
        }
        else {
            cout << "正しい値を入力してください" << endl;
        }
    }
}

// practiceモード本体
int practiceTyping(const vector<WordStruct>& wordsList, argvOptions& argvList) {
    return F_EXIT;
}

// speedモード本体
int speedTyping(const vector<WordStruct>& wordsList, const argvOptions& argvList) {
    return F_EXIT;
}

// testモード本体
int testTyping(const vector<WordStruct>& wordsList, const argvOptions& argvList) {
    return F_EXIT;
}

// 単語ファイルの設定
int wordConfig(const vector<WordStruct>& wordsList, string wordFilePath) {
    return F_EXIT;
}

// タイピング1ワード分
// 引数は左からワード, 関数呼び出し時の時間, 関数に渡す引数をまとめたもの, 戻り値用構造体
void typeWord(const WordStruct& word, time_t startTime, const TypingFuncConfig& config, TypingFuncReturnValue& returnValue) {
    int wordLength = word.wordRoma.length();
    int gotChar = 0;
    system("cls");

    if (config.isTimeCountOn) {

    }
    else {
        cout << "time : " << time(NULL) - startTime << endl;
        if (word.wordMeaning != "") {
            cout << word.wordMeaning << endl;
        }
        if (word.word != "") {
            cout << word.word << endl;
        }

        for (int i = 0;i < wordLength;i++) {
            for (int j = i;j < wordLength;j++) {
                putchar(word.wordRoma.at(j));
            }
            for (int j = 0;j < i;j++) {
                putchar(' ');
            }
            fflush(stdout);

            do {
                gotChar = getch();
                if (gotChar == word.wordRoma.at(i)) {
                    returnValue.correctTypeCount++;
                    break;
                }
                else if (gotChar == ESCAPE_CODE) {
                    returnValue.flag = F_ESCAPE;
                    return;
                }
                else {
                    returnValue.incorrectTypeCount++;
                }
            } while (true);

            for (int j = 0;j < wordLength;j++) {
                putchar('\b');
            }
        }
        returnValue.flag = F_NORMAL_END;
    }
}
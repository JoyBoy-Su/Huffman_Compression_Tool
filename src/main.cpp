#include "Tools.h"

using namespace std;

void getOption(short& op);
void getPath(string& path);

int main() {
    while(true) {
        cout << "Please select a function:" << endl;
        cout << "1. compress\t2.decompress\t3.exit"<< endl;
        short op;
        getOption(op);
        if(op == 3) {
            cout << "Exit program !" << endl;
            break;
        }
        cout << "Please enter the path to be compressed (decompressed):" << endl;
        string path_in = string(), path_out = string();
        getPath(path_in);
        cout << "Please enter the compression (decompression) destination path:" << endl;
        getPath(path_out);
        if(op == 1) {
            Compress* compress = new Compress();
            compress->work(path_in, path_out);
            delete compress;
        } else {
            Decompress* decompress = new Decompress();
            decompress->work(path_in, path_out);
            delete decompress;
        }
    }
    return 0;
}

void getOption(short& op){
    cin >> op;
    while(op != 1 && op != 2 && op != 3) {
        cout << "Please select the correct option:" << endl;
        cin >> op;
    }
    cin.get();
}

void getPath(string& path) {
    cin.sync(); //清空缓冲区
    getline(cin, path);
    while(!Tool::isPath(path)) {
        cout << "The path is illegal. Please enter a legal path!" << endl;
        cin.sync(); //清空缓冲区
        getline(cin, path);
    }
}
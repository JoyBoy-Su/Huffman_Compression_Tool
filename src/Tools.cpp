//
// Created by Su on 2021/11/18.
//
#include "Tools.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <io.h>
#include <sys/stat.h>
#include <direct.h>
#include <regex>

using namespace std;

bool Tool::isPath(string name) {
    std::regex reg("^[a-zA-Z]:\\\\[^/:*?\"<>|]+$");
    bool ret = std::regex_match(name, reg);
    return ret;
}

bool Tool::judgeHuf(string filename) {
    //找到最后扩展名前的'.'的位置
    int index = filename.find_last_of('.');
    //判断扩展名
    string extension = filename.substr(index + 1);
    if(extension == "huf") return true;
    return false;
}

bool Compress::getCode(std::string filename) {
    //创建文件输入流
    ifstream ifs;
    ifs.open(filename.c_str(), ios::in | ios::binary);
    //打开失败返回false
    if(!ifs.is_open()) return false;
    //利用哈希表统计字符个数
    Long count = 0;
    char c = 0;
    while(ifs.read(&c, sizeof(char))){
        if(ch_counts.find(c) == ch_counts.end()) ch_counts[c] = 1;
        else ch_counts[c]++;
        count++;
    }
    ifs.close();
    //若文件为空则直接退出返回false
    if(count == 0) return false;
    //利用哈希表构建哈夫曼树并获得哈夫曼编码
    HuffmanTree<char>* tree = new HuffmanTree<char>(ch_counts);
    tree->getCode(inf);
    delete tree;
    return true;
}

string Compress::getLast(string filename) {
    int index = filename.find_last_of('\\');
    return filename.substr(index + 1);
}

unsigned char Compress::getByte(Long count, char*& code) {
    int bits = (int)(log2(count)) + 1; //bit数
    unsigned char byte = bits / 8 + (bits % 8 == 0 ? 0 : 1);
    code = new char[byte];
    for(int i = byte - 1; i >= 0; i--) {
        code[i] = count & 0xFF;
        count = count >> 8;
    }
    return byte;
}

void Compress::compressFile(string file_path, ofstream& ofs) {
    ofs.seekp(0, ios::cur);
    //输出解压所需信息
    unsigned char state = F_FILE;    //文件标志
    ofs.write((char*)&state, sizeof(unsigned char));
    string file_name = getLast(file_path);
    char size = file_name.length();
    ofs.write(&size, sizeof(size)); //文件名大小
    //文件名
    for(auto c : file_name)
        ofs.write(&c, sizeof(char));
    //输出文本内容
    getCode(file_path);   //获取哈夫曼编码
    //词频信息
    for(auto element : ch_counts) {
        ofs.write(&element.first, sizeof(char));
        char* c = nullptr;
        unsigned char byte = getByte(element.second, c);
        ofs.write((char*)&byte, sizeof(unsigned char)); //字节数
        for(int i = 0; i < byte; i++)
            ofs.write(c + i, sizeof(char));
        delete c;
    }
    //输出压缩信息与压缩文本分割标志
    char flag = 0;
    unsigned char byte = 0;
    ofs.write(&flag, sizeof(char));
    ofs.write((char*)&byte, sizeof(unsigned char));
    if(ch_counts.size() == 1) return;   //若只有一种字符，则不再压缩正文
    //输出正文
    int k = 0;  //存储目前待输出哈夫曼编码的bit长度
    char c = 0; //存储从文件中获得的字符
    string contents = ""; //存储目前待输出的哈夫曼编码
    //读取字符，将哈夫曼编码以8位为单位写入文件
    ifstream ifs;
    ifs.open(file_path.c_str(), ios::in | ios::binary);
    while(ifs.read(&c, sizeof(char))) {
        string tempCode = inf[c];
        contents += tempCode;
        k = contents.length();
        c = 0;
        while(k >= 8) {  //将之前剩下的字符和当前字符存到一个字节写入文件中
            //通过位运算逐个bit输出
            for(int i = 0; i < 8; i++) {
                if(contents[i] == '1') c = (c << 1) | 1;
                else c = c << 1;
            }
            ofs.write((char*)&c, sizeof(char));
            //输出8位bit后调整待输出的哈夫曼编码信息
            contents.erase(0, 8);
            k = contents.length();
        }
    }
    //若字符读取完后待输出的哈夫曼编码小于8位，后续补0后输出
    if(k > 0) {
        contents += "00000000";
        for(int i = 0; i < 8; i++) {
            if(contents[i] == '1') c = (c << 1) | 1;
            else c = c << 1;
        }
        ofs.write((char*)&c, sizeof(char));
    }
    //关闭文件输入输出流
    ifs.close(); ofs.close();
    //清空哈希表，防止数据影响
    ch_counts.clear(); inf.clear();
}

void Compress::compressStart(string folder_path, ofstream& ofs) {
    ofs.seekp(0, ios::cur);
    unsigned char start = F_START;
    ofs.write((char*)&start, sizeof(start));    //文件夹开始标志
    string filename = getLast(folder_path);
    char name_size = filename.length();
    ofs.write(&name_size, sizeof(char));    //开始的文件夹名的大小
    //开始的文件夹名称
    for(char c : filename)
        ofs.write(&c, sizeof(char));
}

void Compress::compressSingle(string path_in, string path_out) {
    //创建文件输出流
    ofstream ofs;
    ofs.open(path_out.c_str(), ios::out | ios::binary | ios::app);  //以追加模式打开文件
    //判断是否为文件夹目录信息
    if(path_in.at(0) == '<') {   //文件夹开始
        compressStart(path_in, ofs);
        ofs.seekp(0, ios::cur);
        return;
    }
    if(path_in == ">") {
        unsigned char end = F_END;
        ofs.write((char*)&end, sizeof(end));    //文件夹结束标志
        return;
    }
    compressFile(path_in, ofs);
    ofs.seekp(0, ios::cur);
}

void Compress::getFiles(string name_in, vector<string>& vec) {
    //判断传入的文件属性
    struct stat state;
    bool isFile = false;
    if(stat(name_in.c_str(), &state) == 0) {
        if(state.st_mode & S_IFREG) isFile = true;
    }
    //若是文件，直接入数组并退出
    if(isFile) {vec.push_back(name_in); cout << name_in << endl; return;}
    //若是文件夹，则先将文件夹开始标志进数组，再查找文件夹中所有文件
    vec.push_back(string("<") + name_in);
    //拿到句柄
    _finddata_t fileInfo;
    string strfind = name_in + "\\*";
    long handle = _findfirst(strfind.c_str(), &fileInfo);
    //文件入数组
    do {
        if(fileInfo.attrib & _A_SUBDIR) {
            if( (strcmp(fileInfo.name,".") != 0 ) &&(strcmp(fileInfo.name,"..") != 0))
                //若为子文件夹，则搜查子文件夹，并将"start"入数组（用类似栈的结构存储文件夹之间的目录信息）
                getFiles(name_in + "\\" + fileInfo.name, vec);
        }
        else vec.push_back(name_in + "\\" + fileInfo.name);
    } while(!_findnext(handle, &fileInfo));
    //结束查找
    _findclose(handle);
    //文件夹结束标志入数组
    vec.push_back(">");
}

void Compress::work(string path_in, string path_out) {
    if(!isPath(path_in)) {
        cout << "The current input is not a path. Please enter the correct full path !" << endl;
        return;
    }
    if(!isPath(path_out)) {
        cout << "The destination path is incorrect. Please enter the correct destination path !" << endl;
        return;
    }
    //判断输入的路径是否存在
    int ret = 0;
    ret = _access(path_in.c_str(), 0);
    if(ret == -1) {
        cout << "The path to be decompressed does not exist." << endl;
        cout << "Compression failed!" << endl;
        return;
    }
    //判断输出的路径是否为.huf路径
    if(!judgeHuf(path_out)) {
        cout << "The destination path extension is not huf." << endl;
        cout << "Compression failed!" << endl;
        return;
    }
    cout << "Compress start!" << endl;
    //先将输出文件清空
    ofstream ofs;
    ofs.open(path_out.c_str(), ios::out);
    ofs.close();
    //处理解压文件信息
    vector<string> paths;
    getFiles(path_in, paths);
    for(auto path : paths)
        compressSingle(path, path_out);
    cout << "Compress compelete!" << endl;
}

void Decompress::setByte(Long &count, char* code, unsigned char byte) {
    count = 0;
    for(int i = 0; i < byte; i++) {
        Long temp = code[i];
        temp = temp & 0x00000000000000FF;
        count = count << 8;
        count = count | temp;
    }
}

HuffmanTree<char>* Decompress::getHuffmanTree(ifstream& ifs, Long& count) {
    //若文件为空，返回空指针
    if(ifs.eof()) return nullptr;
    //利用哈希表接受文件的解压信息，注意这里要用二进制方式读取
    char c = 0;
    //读取并用count该压缩单元保存总字符信息
    count = 0;
    //读取文件前端各个字符信息
    map<char, Long> ch_counts;
    Long temp = 0;
    unsigned char byte;
    int co = 0;
    while(ifs.read(&c, sizeof(char))) {
        ifs.read((char*)&byte, sizeof(unsigned char));
        if(byte == 0) break;    //若count为0说明已经读到了与正文的分割位置
        char* code = new char[byte];
        for(int i = 0; i < byte; i++)
            ifs.read(code + i, sizeof(char));
        setByte(temp, code, byte);
        delete code;
        ch_counts[c] = temp;
        count += temp;
    }
    //通过哈夫曼树得到哈夫曼编码
    HuffmanTree<char>* tree = new HuffmanTree<char>(ch_counts);
    //返回根节点，便于读取哈夫曼编码文件
    return tree;
}

void Decompress::decompressFile(string path_out, ifstream &ifs) {
    ifs.seekg(0, ios::cur); //文件流定位
    char size = 0;
    string file_name = string();
    ifs.read(&size, sizeof(char));    //读取文件名大小
    //读取文件名
    while(size--) {
        char c;
        ifs.read(&c, sizeof(char));
        file_name += c;
    }
    //创建文件输出流
    ofstream ofs;
    ofs.open((path_out + "\\" + file_name).c_str(), ios::out | ios::binary);
    //读取文件内容信息
    Long count = 0;
    HuffmanTree<char>* tree = getHuffmanTree(ifs, count);
    HuffmanNode<char>* node = tree->getRoot();
    //根节点左右孩子为空，则说明只有一种字符
    if(node != nullptr && node->left == nullptr && node->right == nullptr) {
        char c = node->data;
        while(count-- > 0) ofs.write(&c, sizeof(char));
        return;
    }
    //以8bit为单位读取文件，进行解码输出
    char c = 0;
    string contents = string();   //待解码的哈夫曼码
    string result = string();     //解码后的数据
    //循环读取文件直到count为0
    while(count > 0 && ifs.read(&c, sizeof(char))) {
        //读取到正文时，把8bit存入字符串（在构造哈夫曼树时已经将前面的信息读取完成，仅剩的就是正文内容，直接解压即可）
        for(unsigned char i = 0x80; i > 0; i = i >> 1)
            contents += ((i & c) != 0) ? "1" : "0";
        //通过遍历哈夫曼树对当前8bit进行解码
        tree->deCode(node, contents, result);
        //逐个字符输出
        for(int i = 0; i < result.length(); i++) {
            if(count > 0) { //这里需要限制count > 0，因为最后8bit中补充的0可能会导致count多解压出一些字符
                char r = result.at(i);
                ofs.write(&r, sizeof(char));
                count--;    //输出一个字符，count自减一
            }
        }
        result.erase(0, result.length());
    }
    //解压完成后删除哈夫曼树，关闭文件输出流，并清理有关哈希表
    delete tree;
    ofs.close();
}

void Decompress::newFolder(string& path_out, ifstream& ifs) {
    ifs.seekg(0, ios::cur); //文件流定位
    string folder_name = string();
    char size = 0;
    ifs.read(&size, sizeof(char));
    //读取文件夹名
    while(size--) {
        char c;
        ifs.read(&c, sizeof(c));
        folder_name += c;
    }
    path_out = path_out + "\\" + folder_name;
    if (access(path_out.c_str(), 0))
        mkdir(path_out.c_str());   // 返回 0 表示创建成功，-1 表示失败
}

void Decompress::work(string path_in, string path_out) {
    if(!isPath(path_in)) {
        cout << "The current input is not a path. Please enter the correct full path !" << endl;
        return;
    }
    if(!isPath(path_out)) {
        cout << "The destination path is incorrect. Please enter the correct destination path !" << endl;
        return;
    }
    //若传入待解压文件扩展名不是.huf，则无法解压直接退出
    if(!judgeHuf(path_in)) {
        cout << "The extension of the path to be decompressed is not huf." << endl;
        cout << "Decompression failed!" << endl;
        return;
    }
    //若路径目录不存在，则创建目录
    int ret = access(path_out.c_str(), 0);
    if(ret == -1) {
        if(mkdir(path_out.c_str())) {
            cout << "Folder creation failed." << endl;
            cout << "Decompression failed!" << endl;
            return;
        }
    }
    //若目标路径为文件，则退出
    struct stat sta;
    if(stat(path_out.c_str(), &sta) == 0)
        if(sta.st_mode & S_IFREG) {
            cout << "The current output path is a file." << endl;
            cout << "Decompression failed!" << endl;
            return;
        }
    //创建文件输入流
    ifstream ifs;
    ifs.open(path_in.c_str(), ios::in | ios::binary);
    //若文件为空，则直接退出
    if(ifs.eof()) return;
    //开始逐个解压每个压缩单元
    cout << "Decompress start!" << endl;
    unsigned char state;
    while(ifs.read((char*)&state, sizeof(unsigned char))) {
        //若压缩单元属性为文件内容
        if(state & F_FILE) {
            decompressFile(path_out, ifs);
            continue;
        }
        if(state & F_START) {
            newFolder(path_out, ifs);
            continue;
        }
        if(state & F_END) {
            int index = path_out.find_last_of("\\");
            path_out = path_out.substr(0, index);
            continue;
        }
        ifs.seekg(0, ios::cur);
    }
    cout << "Decompress compelete!" << endl;
    ifs.close();
}
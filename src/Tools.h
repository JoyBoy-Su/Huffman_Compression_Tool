//
// Created by Su on 2021/11/18.
//

#ifndef PROJECT1_TEST_TOOLS_H
#define PROJECT1_TEST_TOOLS_H
#include "HuffmanTree.h"

class Tool{
protected:
    static const unsigned char F_FILE = 0x01;   //文件标志
    static const unsigned char F_START = 0x02;  //文件夹开始标志
    static const unsigned char F_END = 0x4;    //文件夹结束标志
public:
    Tool() {}
    static bool isPath(std::string name);
    static bool judgeHuf(std::string filename);
    virtual void work(std::string path_in, std::string path_out) = 0;
};

class Compress : public Tool {
    std::map<char, Long> ch_counts;     //目标文件各字符出现次数
    std::map<char, std::string> inf;    //目标文件对应哈夫曼编码
protected:
    bool getCode(std::string filename);
    std::string getLast(std::string filename);
    unsigned char getByte(Long count, char*& code);
    void compressFile(std::string file_path, std::ofstream& ofs);
    void compressStart(std::string folder_path, std::ofstream& ofs);
    void compressSingle(std::string filename_in, std::string filename_out);
    void getFiles(std::string name_in, std::vector<std::string>& vec);
public:
    Compress() {}
    void work(std::string name_in, std::string name_out);
};

class Decompress : public Tool {
protected:
    HuffmanTree<char>* getHuffmanTree(std::ifstream& ifs, Long& count);
    void setByte(Long& count, char* code, unsigned char byte);
    void decompressFile(std::string path_out, std::ifstream& ifs);
    void newFolder(std::string& path_out, std::ifstream& ifs);
public:
    Decompress() {}
    void work(std::string path_in, std::string path_out);
};

#endif //PROJECT1_TEST_TOOLS_H
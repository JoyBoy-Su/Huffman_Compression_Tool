//
// Created by Su on 2021/11/18.
//

#ifndef PROJECT1_TEST_HUFFMANTREE_H
#define PROJECT1_TEST_HUFFMANTREE_H
#include "Heap.h"
#include <iostream>
#include <map>
#include <queue>

typedef long long Long;
template<typename T>
struct HuffmanNode {
    T data;
    Long weight;
    HuffmanNode<T>* left;
    HuffmanNode<T>* right;
    HuffmanNode<T>* parent;
    HuffmanNode(Long w, const T& t = T()) :
            data(t), weight(w), left(nullptr), right(nullptr), parent(nullptr) {}
};

template<typename T>
class HuffmanTree{
    typedef HuffmanNode<T> Node;
    typedef std::map<T, std::string> HashMap;
    Node* root;
protected:
    void destroy(Node* node);
    void code(HashMap& hash, Node* node, std::string codeHuf= "");
public:
    HuffmanTree() : root(nullptr) {}
    HuffmanTree(std::map<T, Long> hash);
    Node* getRoot() const {return root;}
    void getCode(HashMap& hash);
    void deCode(HuffmanNode<char>*& node, std::string& inf, std::string& result);
    ~HuffmanTree() { destroy(root); }
};

template<typename T>
HuffmanTree<T>::HuffmanTree(std::map<T, Long> hash) {
    if(hash.empty()) {root = nullptr; return;}
    struct NodeLess{
        bool operator()(Node* left, Node* right) {
            return left->weight < right->weight;
        }
    };
    //完成建堆
    Heap<Node*, NodeLess> heap;
    for(auto inf : hash) {
        Node* node = new Node(inf.second, inf.first);
        heap.push(node);
    }
    //构造哈夫曼树
    while(heap.size() > 1) {
        //从堆中取前两个结点
        Node* left = heap.front();
        heap.pop();
        Node* right = heap.front();
        heap.pop();
        //构造父节点，修改各个指针以实现结点的链接关系
        Node* parent = new Node(left->weight + right->weight);
        parent->left = left;
        parent->right = right;
        left->parent = parent;
        right->parent = parent;
        //父节点入堆
        heap.push(parent);
    }
    root = heap.front();
}

template<typename T>
void HuffmanTree<T>::destroy(Node *node) {
    if(node == nullptr) return;
    Node* left = node->left;
    Node* right = node->right;
    destroy(left);
    destroy(right);
    delete node;
    node = nullptr;
}

template<typename T>
void HuffmanTree<T>::getCode(HashMap &hash) {
    if(root == nullptr) return;
    code(hash, root);
}

template<typename T>
void HuffmanTree<T>::code(HashMap &hash, Node* node, std::string codeHuf) {
    if(node->left == nullptr && node->right == nullptr) {
        hash[node->data] = codeHuf;
        return;
    }
    if(node->left != nullptr) code(hash, node->left, codeHuf + "0");
    if(node->right != nullptr) code(hash, node->right, codeHuf + "1");
}

template<typename T>
void HuffmanTree<T>::deCode(HuffmanNode<char> *&node, std::string &inf, std::string &result) {
    for(int i = 0; i < 8; i++) {
        //当遇到叶子结点时，将对应字符记录，并重置node
        if(node->left == nullptr && node->right == nullptr) {
            result += node->data;
            node = root;
        }
        if(inf.at(i) == '0') node = node->left;
        else node = node->right;
    }
    //当遇到叶子结点时，将对应字符记录，并重置node
    if(node->left == nullptr && node->right == nullptr) {
        result += node->data;
        node = root;
    }
    //当前8bit解码完成后清空contents
    inf.erase(0, 8);
}

#endif //PROJECT1_TEST_HUFFMANTREE_H
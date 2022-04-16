//
// Created by Su on 2021/11/18.
//

#ifndef PROJECT1_TEST_HEAP_H
#define PROJECT1_TEST_HEAP_H
#include <vector>
#include <cstddef>

template<typename T>
struct Less {
    bool operator()(const T& a, const T& b) {return a < b;}
};

template<typename T>
struct Great {
    bool operator()(const T& a, const T& b) {return a > b;}
};

template<typename T>
void swap(T& a, T&b) {
    T temp = a;
    a = b;
    b = temp;
}

template<typename T, class Compare = Less<T>>
class Heap{
    //顺序存储二叉堆
    std::vector<T> data;
protected:
    void adjustUp(int index);
    void adjustDown(int root, int size);
public:
    Heap() {}
    Heap(T* t, int size);
    inline void push(const T& t);
    void pop();
    int size() {return data.size();}
    bool empty() {return data.empty();}
    T& front() {return data[0];}
};

template<typename T, class Compare>
void Heap<T, Compare>::adjustUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (Compare()(data[index], data[index])) {
            swap(data[parent], data[index]);
            index = parent;
        } else break;
    }
}

template<typename T, class Compare>
void Heap<T, Compare>::adjustDown(int root, int size) {
    int parent = root;
    int child = parent * 2 + 1;
    while (child < size) {
        if ((child + 1) < size && Compare()(data[child + 1], data[child]))
            // 找左右孩子中最大的下标
            child++;
        if (Compare()(data[child], data[parent])) {
            swap(data[parent], data[child]);
            parent = child;
            child = parent * 2 + 1;
        } else {
            break;
        }
    }
}

template<typename T, class Compare>
Heap<T, Compare>::Heap(T *t, int size) {
    data.resize(size);
    //进容器
    for(int i = 0; i < size; i++)
        data.push_back(t[i]);
    //建堆
    for(int i = (size - 2) / 2; i >= 0; i--)
        adjustDown(i, size);
}

template<typename T, class Compare>
void Heap<T, Compare>::push(const T &t) {
    //插到尾部，再向上调整
    data.push_back(t);
    adjustUp(data.size() - 1);
}

template<typename T, class Compare>
void Heap<T, Compare>::pop() {
    if(data.empty()) return;
    // 将堆顶与最后一个元素交换，再从堆顶下滑调整；因为vector没有提供直接pop掉首元素的方法
    T& temp = data[0];
    swap(data[0], data[data.size() - 1]);
    data.pop_back();
    if(data.size() > 1) adjustDown(0, data.size());
}


#endif //PROJECT1_TEST_HEAP_H

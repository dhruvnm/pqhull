#ifndef CIRCULAR_LINKED_LIST_H
#define CIRCULAR_LINKED_LIST_H

#include <vector>

template<class T>
struct circular_linked_list {
    T data;
    circular_linked_list<T> *next, *prev;
    std::vector<T> cll_to_vector();
    circular_linked_list(T data);
    ~circular_linked_list();
    void insert_after(T next);
    void insert_before(T next);
    static circular_linked_list<T>* vector_to_cll(std::vector<T>);
    void assert_valid_cll();
};


#endif
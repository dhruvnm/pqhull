#include "circular_linked_list.hpp"
#include "utils.h"

template<class T>
circular_linked_list<T>::circular_linked_list(T data) {
    this->data = data;
    next = this;
    prev = this;
}

template<class T>
void circular_linked_list<T>::insert_after(T val) {
    circular_linked_list<T>* next = new circular_linked_list(val);
    next->next = this->next;
    this->next->prev = next;
    next->prev = this;
    this->next = next;
    //assert_valid_cll();
}

template<class T>
std::vector<T> circular_linked_list<T>::cll_to_vector() {
    circular_linked_list<T>* first = this;
    std::vector<T> output;
    output.push_back(first->data);
    for (auto temp = first->next; temp != first; temp = temp->next) {
        output.push_back(temp->data);
    }
    return output;
}

template<class T>
circular_linked_list<T>* circular_linked_list<T>::vector_to_cll(std::vector<T> v) {
    circular_linked_list<T>* out = new circular_linked_list<Point>(v[0]);
    auto temp = out;
    for (int i = 1; i < v.size(); ++i) {
        temp->insert_after(v[i]);
        temp = temp->next;
    }
    return out;
}

template<class T>
void circular_linked_list<T>::assert_valid_cll() {
    circular_linked_list<T>* curr = this->next, *prev = this;
    while (curr != this) {
        assert(curr->prev == prev);
        prev = curr;
        curr = curr->next;
    }

}


/* explicit instantiation */
template class circular_linked_list<Point>;


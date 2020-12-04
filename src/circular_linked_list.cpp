#include "circular_linked_list.hpp"

circular_linked_list::circular_linked_list(T data) {
    this->data = data;
    next = nullptr;
    prev = nullptr;
}

void circular_linked_list::insert_after(T val) {
    circular_linked_list* next = new circular_linked_list(val);
    next->next = this->next ? this->next : this;
    next->prev = this;
    this->next = next;
    if (!this->prev)
        this->prev = next;
}


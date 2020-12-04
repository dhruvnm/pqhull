
template<typename T>
struct circular_linked_list {
    T data;
    circular_linked_list<T> *next, *prev;
    circular_linked_list(T data);
    void insert_after(T next);
};
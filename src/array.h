#if !defined(ARRAY_H)
#define ARRAY_H

#define free_array(a) {if ((a).data) { free((a).data); } }

#define array_foreach(a, it) for ((it) = (a).data; ((it) - (a).data) < (i64)(a).length; (it)++)
#define array_iterate_back(a, it, idx) for ((idx) = ((a).length - 1), (it) = ((a).data + (a).length - 1); (idx) >= 0; (idx)--, (it)--)
#define array_bytes(a) ((a).length * sizeof((a)[0]))

//
// Dynamic Array
//

template <typename T>
struct Array {
    size_t length   = 0;
    size_t capacity = 0;
    T *data      = nullptr;

    inline bool is_full();

    void reserve(size_t amount);

    void add(T item);
    void fast_add(T item);  // Adds an item without checking for a resize.
    T* allocate_item(); // Allocates space for an item and returns pointer to that space.

    T& operator[] (size_t index);
    T& last();

    void remove(size_t index);
    void fast_remove(size_t index); // Swaps last element with element on provided index.

    void remove_last();
    void remove_last_and_zero_out();

    void clear();
};

template <typename T>
internal void allocate_array(Array<T> &array, size_t slots) {
    array = {};
    array.capacity = slots;

    if (slots == 0) {
        array.data = nullptr;
    }
    else {
        array.data = (T*)malloc(sizeof(T) * slots);
    }
}

template <typename T>
inline bool Array<T>::is_full() {
    return this->length == this->capacity;
}

template <typename T>
void Array<T>::reserve(size_t amount) {
    size_t new_capacity = this->capacity + amount;

    if (this->data) {
        this->data = (T*)realloc(this->data, new_capacity * sizeof(T));
    }
    else {
        this->data = (T*)malloc(new_capacity * sizeof(T));
        this->length = 0;   // Just to be sure...
    }

    this->capacity = new_capacity;
}

template <typename T>
void Array<T>::add(T item) {
    if (this->length >= this->capacity) {
        // Double the capacity
        this->reserve(this->capacity);
    }

    assert(this->length < this->capacity);

    this->data[this->length] = item;
    this->length += 1;
}

template <typename T>
T* Array<T>::allocate_item() {
    if (this->length >= this->capacity) {
        // @Todo: do we want to +1 the capacity or more?
        this->reserve(1);
    }

    assert(this->length < this->capacity);

    this->length += 1;

    return (this->data + this->length - 1);
}

template <typename T>
void Array<T>::fast_add(T item) {
    assert(this->length < this->capacity);

    this->data[this->length] = item;
    this->length += 1;
}

template <typename T>
T& Array<T>::operator[] (size_t index) {
    assert(index < this->length);

    T& result = this->data[index];

    return result;
}

template <typename T>
T& Array<T>::last() {
    assert(this->length > 0);

    return (this->data + this->length - 1);
}

template <typename T>
void Array<T>::remove(size_t index) {
    assert(index < this->length);

    // When removing the last element
    if (index == this->length - 1) {
        this->length -= 1;
    }
    else {
        size_t right_part = this->length - 1 - index;
        memmove(this->data + index, this->data + index + 1, sizeof(T) * right_part);
        this->length -= 1;
    }
}

template <typename T>
void Array<T>::fast_remove(size_t index) {
    assert(index < this->length);

    if (this->length == 1) {
        this->length = 0;
    }
    // When removing the last element
    else if (index == this->length - 1) {
        this->length -= 1;
    }
    // Move last element to the place of removal.
    // This ensusres that we do not need to shift the rest of the array.
    else {
        this->data[index] = this->data[this->length - 1];

        this->length -= 1;
    }
}

template <typename T>
void Array<T>::remove_last() {
    assert(this->length > 0);

    this->length--;
}

template <typename T>
void Array<T>::remove_last_and_zero_out() {
    assert(this->length > 0);

    memset(this->data + this->length - 1, 0, sizeof(T));

    this->length--;
}

template <typename T>
void Array<T>::clear() {
    this->length = 0;
}

//
// Static Array
//

template <typename T>
struct StaticArray {
    size_t length   = 0;
    size_t capacity = 0;
    T *data      = nullptr;

    inline bool is_full();

    void add(T item);
    T* allocate_item(); // Allocates space for an item and returns pointer to that space.

    T& operator[] (size_t index);
    T& last();

    void remove(size_t index);
    void fast_remove(size_t index); // Swaps last element with element on provided index.

    void remove_last();
    void remove_last_and_zero_out();

    void clear();
};

template <typename T>
internal void allocate_array(StaticArray<T> &array, size_t slots) {
    assert(slots > 0);

    array = {};
    array.capacity = slots;
    array.data = (T*)malloc(sizeof(T) * slots);
}

#if defined(MEMORY_H)
template <typename T>
internal void allocate_array_from_block(StaticArray<T> &array, size_t slots, MemoryArena *block) {
    assert(slots > 0);

    array = {};
    array.capacity = slots;
    array.data = push_array(block, slots, T);
}
#endif

template <typename T>
inline bool StaticArray<T>::is_full() {
    return this->length == this->capacity;
}

template <typename T>
void StaticArray<T>::add(T item) {
    assert((this->length + 1) <= this->capacity);

    this->data[this->length] = item;
    this->length += 1;
}

template <typename T>
T* StaticArray<T>::allocate_item() {
    assert((this->length + 1) <= this->capacity);

    this->length += 1;

    return (this->data + this->length - 1);
}

template <typename T>
T& StaticArray<T>::operator[] (size_t index) {
    assert(index < this->length);

    T& result = this->data[index];

    return result;
}

template <typename T>
T& StaticArray<T>::last() {
    assert(this->length > 0);

    return (this->data + this->length - 1);
}

template <typename T>
void StaticArray<T>::remove(size_t index) {
    assert(index < this->length);

    // When removing the last element
    if (index == this->length - 1) {
        this->length -= 1;
    }
    else {
        size_t right_part = this->length - 1 - index;
        memmove(this->data + index, this->data + index + 1, sizeof(T) * right_part);
        this->length -= 1;
    }
}

template <typename T>
void StaticArray<T>::fast_remove(size_t index) {
    assert(index < this->length);

    if (this->length == 1) {
        this->length = 0;
    }
    // When removing the last element
    else if (index == this->length - 1) {
        this->length -= 1;
    }
    // Move last element to the place of removal.
    // This ensusres that we do not need to shift the rest of the array.
    else {
        this->data[index] = this->data[this->length - 1];

        this->length -= 1;
    }
}

template <typename T>
void StaticArray<T>::remove_last() {
    assert(this->length > 0);

    this->length--;
}

template <typename T>
void StaticArray<T>::remove_last_and_zero_out() {
    assert(this->length > 0);

    memset(this->data + this->length - 1, 0, sizeof(T));

    this->length--;
}

template <typename T>
void StaticArray<T>::clear() {
    this->length = 0;
}

#endif

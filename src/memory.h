#if !defined(MEMORY_H)
#define MEMORY_H

#define MEMORY_MIN_BLOCK_SIZE 32

struct MemoryBlock {
    MemoryBlock *next;
    size_t bytes;   // Without header
};

struct MemoryArena {
    size_t bytes;
    size_t used;
    u8 *base;
    MemoryBlock *first;
};

struct TemporaryMemoryArena {
    size_t bytes;
    size_t used;
    u8 *base;
};

#define push_struct(gen_mem, type) (type *)_push_to_memory_arena(gen_mem, sizeof(type))
#define push_array(gen_mem, capacity, type) (type *)_push_to_memory_arena(gen_mem, sizeof(type) * (capacity))

internal void* _push_to_memory_arena(MemoryArena *memory, size_t size) {
    assert((memory->used + size) <= memory->bytes);

    if (memory->first == nullptr) {
        memory->first = (MemoryBlock *)memory->base;
        memory->first->next = nullptr;
        memory->first->bytes = memory->bytes - sizeof(MemoryBlock);

        memory->used += sizeof(MemoryBlock);    // The first block header takes memory
    }

    size_t size_with_header = size + sizeof(MemoryBlock);

    MemoryBlock *best_previous = nullptr;
    MemoryBlock *best = nullptr;

    // Find best fit
    {
        MemoryBlock *previous = nullptr;
        MemoryBlock *walker = memory->first;
        while (walker) {
            if (walker->bytes >= size && (best == nullptr || walker->bytes < best->bytes)) {
                best_previous = previous;
                best = walker;
            }

            previous = walker;
            walker = walker->next;
        }
    }

    assert(best);

    i64 remaining_size = best->bytes - size_with_header;

    if (remaining_size >= MEMORY_MIN_BLOCK_SIZE) {  // We can split
        MemoryBlock *remaining_block = (MemoryBlock *)(((u8 *)best) + size_with_header);
        remaining_block->bytes = (size_t)remaining_size;
        remaining_block->next = best->next;

        if (best_previous) best_previous->next = remaining_block;
        else memory->first = remaining_block;
    }
    else {  // We don't need to split
        size = best->bytes;
        size_with_header = best->bytes + sizeof(MemoryBlock);

        if (best_previous) best_previous->next = best->next;
        else memory->first = best->next;
    }

    best->next = (MemoryBlock *)1;
    best->bytes = size;

    memory->used += size_with_header;

    void *sub_block = (((u8 *)best) + sizeof(MemoryBlock));
    return sub_block;
}

internal void _free_from_memory_arena(MemoryArena *memory, void *ptr) {
    MemoryBlock *block_to_insert = (MemoryBlock *)((u8 *)ptr - sizeof(MemoryBlock));

    assert(block_to_insert->next == (MemoryBlock *)1);

    memory->used -= block_to_insert->bytes;

    if (memory->first) {
        MemoryBlock *walker = memory->first;

        if (memory->first > block_to_insert) {
            block_to_insert->next = memory->first;
            memory->first = block_to_insert;
        }
        else {
            while (1) {
                if (walker < block_to_insert && (!walker->next || walker->next > block_to_insert)) {
                    break;
                }

                walker = walker->next;
            }

            assert(walker);

            block_to_insert->next = walker->next;
            walker->next = block_to_insert;
        }
    }
    else if (memory->bytes == memory->used) {
        memory->first = block_to_insert;
    }
    else {
        invalid_code_path("Freeing from memory without blocks!!");
    }

    {
        MemoryBlock *walker = memory->first;
        while (walker) {
            while (walker->next == (MemoryBlock *)((u8 *)walker + (walker->bytes + sizeof(MemoryBlock)))) {
                size_t next_block_size = walker->next->bytes + sizeof(MemoryBlock);

                walker->next = walker->next->next;
                walker->bytes += next_block_size;
            }

            walker = walker->next;
        }
    }
}

internal void reset_memory_arena(MemoryArena *memory, bool zero_out = false) {
    memory->used = 0;

    if (zero_out) {
        memset((void *)memory->base, 0, memory->bytes);
    }
}

#define push_temp_struct(temp_mem, type) (type *)_push_to_temporary_memory_arena(temp_mem, sizeof(type))
#define push_temp_array(temp_mem, capacity, type) (type *)_push_to_temporary_memory_arena(temp_mem, sizeof(type) * (capacity))

internal void* _push_to_temporary_memory_arena(TemporaryMemoryArena *memory, size_t size) {
    assert((memory->used + size) <= memory->bytes);

    void *result = (void *)(memory->base + memory->used);

    memory->used += size;

    return result;
}

#endif // MEMORY_H

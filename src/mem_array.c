#include "mem_array.h"

mem_array_t Init_Mem_Array () {
    mem_array_t arr;
    arr.start_addr = (mem_t*) malloc (ARR_CAPACITY_INIT * MEM_t_SIZE);
    arr.end_addr = arr.start_addr + ARR_CAPACITY_INIT * MEM_t_SIZE;
    arr.cur_addr = arr.start_addr;
    arr.capacity = ARR_CAPACITY_INIT;
    return arr;
}

void Append_Mem_Array (mem_array_t *arr, mem_t value) {
    if (arr->cur_addr >= arr->end_addr) {
        arr->capacity += ARR_CAPACITY_INCREASE;
        realloc (arr->start_addr, arr->capacity * MEM_t_SIZE);
        arr->end_addr += ARR_CAPACITY_INCREASE * MEM_t_SIZE;
    }
    *(arr->cur_addr) = value;
    arr->cur_addr += MEM_t_SIZE;
}

mem_t* Get_Mem_Array (mem_array_t *arr, size_t i) {
    return (arr->start_addr + i * MEM_t_SIZE < arr->cur_addr) ? arr->start_addr + i * MEM_t_SIZE : NULL;
}

void Remove_Mem_Array (mem_array_t *arr, size_t i) {
    mem_t* mem = Get_Mem_Array (arr, i);
    if (mem != NULL) return;  // not valid

    arr->cur_addr -= MEM_t_SIZE;
    if (mem < arr->cur_addr) {   // no movement needed for the last element
        *mem = *arr->cur_addr;   // last value replace the removed one   
    }
}

int Find_Addr_Mem_Array (mem_array_t *arr, uintptr_t addr) {
    int i = 0;
    mem_t* cur_mem = arr->start_addr;
    while (cur_mem < arr->cur_addr && cur_mem->addr != addr) {
        cur_mem += MEM_t_SIZE;
        i++;
    }
    if (cur_mem >= arr->cur_addr) i = -1;
    return i;
}

void Peek_Memory (pid_t pid, uintptr_t addr, char *buf, size_t len);

char* Serialized_Mem_Array (mem_array_t *arr, size_t* len, pid_t pid) {
    *len = 0;

    mem_t* cur_mem = arr->start_addr;
    while (cur_mem < arr->cur_addr) {
        *len += sizeof (uintptr_t);              // addr
        *len += sizeof (size_t);                 // len
        *len += cur_mem->len * sizeof (char);    // content
        cur_mem += MEM_t_SIZE;
    }

    char* data = (char*) malloc (*len * sizeof (char));

    size_t offset = 0;
    cur_mem = arr->start_addr;
    while (offset < *len) {
        // addr
        memcpy (&data [offset], (const char*) &cur_mem->addr, sizeof (uintptr_t));
        offset += sizeof (uintptr_t);

        // len
        memcpy (&data [offset], (const char*) &cur_mem->len, sizeof (size_t));
        offset += sizeof (size_t);

        // content
        char* buf = (char*) malloc (cur_mem->len * sizeof (char));
        Peek_Memory (pid, cur_mem->addr, buf, cur_mem->len);
        memcpy (&data [offset], (const char*) buf, cur_mem->len * sizeof (char));
        free (buf);
        offset += cur_mem->len * sizeof (char);

        cur_mem += MEM_t_SIZE;
    }

    return data;
}

void Free_Mem_Array (mem_array_t *arr) {
    free (arr->start_addr);
}

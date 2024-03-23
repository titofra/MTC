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


void Free_Mem_Array (mem_array_t *arr) {
    free (arr->start_addr);
}

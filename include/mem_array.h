#ifndef MEM_ARRAY_H
#define MEM_ARRAY_H

#include <stdlib.h>
#include <stdint.h>

#define ARR_CAPACITY_INIT 100
#define ARR_CAPACITY_INCREASE 100

typedef struct mem {
    uintptr_t addr;
    size_t len;
} mem_t;

#define MEM_t_SIZE sizeof (mem_t)

typedef struct mem_array {
    mem_t* start_addr;
    mem_t* end_addr;
    mem_t* cur_addr;
    size_t capacity;
} mem_array_t;

mem_array_t Init_Mem_Array ();

void Append_Mem_Array (mem_array_t *arr, mem_t value);

mem_t* Get_Mem_Array (mem_array_t *arr, size_t i);

void Remove_Mem_Array (mem_array_t *arr, size_t i);

int Find_Addr_Mem_Array (mem_array_t *arr, uintptr_t addr);

void Free_Mem_Array (mem_array_t *arr);

#endif  // MEM_ARRAY_H

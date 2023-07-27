//Authors: Scott Skibin (ss3793), Kevin Schwarz (kjs309)
//Course: CS416
//iLab server used: cp.cs.rutgers.edu

#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need

#define PGSIZE 4096 //1K=1024 | 4K=4096 | 16K=16384 | 32K=32768 | 64K=65536

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024 //200MB = 262144 | 1GB = 1024*1024*1024 | 2GB = 2ULL*1024*1024*1024 | 3GB = 3ULL*1024*1024*1024

// Represents a page table entry
typedef unsigned long pte_t;

// Represents a page directory entry
typedef unsigned long pde_t;

#define TLB_ENTRIES 512

/************V*Not in default header file*V**************/

typedef struct level_info level_info_t;
typedef struct VA_info VA_info_t;
typedef struct bitmap bitmap_t;
typedef struct level_bitmap level_bitmap_t;
typedef struct tlb_entry tlb_entry_t;

struct level_info{
    unsigned long bits;
    unsigned long build_bits;
    unsigned long entries;
    unsigned long bm_size;
};

struct VA_info {
    unsigned long address_space;
    unsigned long entry_size;
    unsigned long offset;
    unsigned long vpn_size;
    unsigned long vpn_max_val;
    unsigned long max_entries;
    unsigned long max_bits;
    unsigned long bits_rem;
    unsigned long levels;
    unsigned long phys_bm_size;
    level_info_t** level_info;
};

struct bitmap{
    int level;
    unsigned long pages;
    unsigned long* bitmap;
    int free_page;
    bitmap_t* parent;
    bitmap_t** children;
    unsigned long last_free;
    unsigned long num_free;
    int phys_idx;
};

struct tlb_entry{
    unsigned long vpn;
    unsigned long pfn;
};

/************^*Not in default header file*^**************/
/*Assume your TLB is a direct mapped TLB with the number of entries as TLB_ENTRIES
    * Think about the size of each TLB entry that performs virtual to physical
    * address translation.
    */
struct tlb{
   tlb_entry_t** cache;
   bitmap_t* valid;
};
struct tlb tlb_store;

void set_physical_mem();
void destroy_physical_mem();
int add_TLB(void *va, void *pa);
pte_t * check_TLB(void *va);
void print_TLB_missrate();
pte_t* translate(pde_t *pgdir, void *va);
int page_map(pde_t *pgdir, void *va, void* pa);
void *get_next_avail(int num_pages);
void *t_malloc(unsigned int num_bytes);
void t_free(void *va, int size);
int put_value(void *va, void *val, int size);
void get_value(void *va, void *val, int size);
void mat_mult(void *mat1, void *mat2, int size, void *answer);

/************V*Not in default header file*V**************/
VA_info_t* init_virtual_address_info();
void destroy_virtual_address_info(VA_info_t* va_info);
unsigned long va_to_vpi(unsigned long va);
unsigned long vpi_to_va(unsigned long vpi);
void display_va(unsigned long va);
void init_tlb(struct tlb* _tlb);
void destroy_tlb(struct tlb* _tlb);
bitmap_t* init_bitmap(unsigned int size, bitmap_t* p, bool has_children, int lvl);
void destroy_bitmap(bitmap_t* bm);
void display_bitmap(bitmap_t* bm, int clip);
bool is_set(bitmap_t* bm, unsigned long page_num);
void flip_bit(bitmap_t* bm, unsigned long page_num);
void clear_bits(unsigned long* va, unsigned long start, unsigned long end);
bool is_free_complete(bitmap_t* bm);
bool has_space(bitmap_t* bm, unsigned long num_pages);
bool find_first_contig(bitmap_t* bm, unsigned long num_pages, unsigned long* va);
void find_first_free(bitmap_t* bm);
bool find_first_free_rec(bitmap_t* bm, unsigned long* va, unsigned long track);
void* Malloc(size_t size);

#endif

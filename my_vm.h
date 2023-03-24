#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need

#define PGSIZE 4096

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024

// Represents a page table entry
typedef unsigned long pte_t;

// Represents a page directory entry
typedef unsigned long pde_t;

#define TLB_ENTRIES 512

//Structure to represents TLB
//Since it's a direct-mapped TLB, 
//i am using an array of entries, where each entry will store the VPN, PPN, 
//and a valid bit to indicate if the entry is valid.

//TLB Entry
typedef struct {
    unsigned long vpn;
    unsigned long ppn;
    bool valid;
} tlb_entry_t;

struct tlb {
    /*Assume your TLB is a direct mapped TLB with the number of entries as TLB_ENTRIES
    * Think about the size of each TLB entry that performs virtual to physical
    * address translation.
    */
   tlb_entry_t entries[TLB_ENTRIES];
} tlb_t;
struct tlb tlb_store;

void set_physical_mem();
pte_t* translate(pde_t *pgdir, void *va);
int page_map(pde_t *pgdir, void *va, void* pa);
int add_TLB(void *va, void *pa);
pte_t *check_TLB(void *va);
void print_TLB_missrate();
void *get_next_avail(int num_pages);
void *t_malloc(unsigned int num_bytes);
void t_free(void *va, int size);
int put_value(void *va, void *val, int size);
void get_value(void *va, void *val, int size);
void mat_mult(void *mat1, void *mat2, int size, void *answer);

#endif

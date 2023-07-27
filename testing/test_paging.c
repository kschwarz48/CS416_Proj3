#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

/*32-bit address space specs*/
#define ADDRESS_SPACE 32 // bits
#define ENTRY_SIZE ADDRESS_SPACE/8 //bytes

#define PGSIZE 4096 //16384 //4096 //2048 //1024

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024 // 4GB

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024 // 1GB

// Represents a page table entry
typedef unsigned long pte_t;
// Represents a page directory entry
typedef unsigned long pde_t;
typedef long align;

typedef union page page_t;
typedef struct level_info level_info_t;
typedef struct VA_info VA_info_t;
typedef struct pt pt_t;
typedef struct pd pd_t;

/*Globals*/
uint32_t* ram;
uint32_t* phys_bm;
uint32_t* first_free;

union page { /* block header */
    struct {
        page_t *ptr;
        uint32_t size;
    } s;
    align x; /* Force alignment */
};

struct level_info{
    uint32_t bits;
    uint32_t entries;
    uint32_t bm_size;
};

struct VA_info {
    uint32_t offset;
    uint32_t vpn_size;
    uint32_t max_entries;
    uint32_t max_bits;
    uint32_t levels;
    level_info_t** level_info;
};

struct pt{
    uint32_t* valid_bm;
    pte_t* pte;
    pd_t* root;
    pt_t* parent;
    pt_t** pt_nxt;
    uint32_t level;
};

struct pd{
    uint32_t* valid_bm;
    pde_t* pde;
    pt_t** pt;
};


VA_info_t* init_virtual_address_info();
void destroy_virtual_address_info(VA_info_t* va_info);
pd_t* init_page_directory(VA_info_t* va_info);
void destroy_page_directory(pd_t* pd, VA_info_t* va_info);
pt_t* init_page_table(VA_info_t* va_info, pd_t* pd_root, pt_t* pt_par, uint32_t lvl);
void destroy_page_table(pt_t* pt, VA_info_t* va_info);
void* Malloc(size_t size);
void *t_malloc(unsigned int num_bytes);

VA_info_t* init_virtual_address_info(){
    VA_info_t* va_info = (VA_info_t*)Malloc(sizeof(VA_info_t));
    /*offset = log_2(PAGE SIZE)*/
    va_info->offset = (uint32_t)log2(PGSIZE);
    /*VPN = ADDRESS SPACE - offset*/
    va_info->vpn_size = ADDRESS_SPACE - va_info->offset;
    
    va_info->max_entries = PGSIZE/ENTRY_SIZE;
    va_info->max_bits = (uint32_t)log2(va_info->max_entries);
    va_info->levels = (va_info->vpn_size/va_info->max_bits);
    uint32_t bits_rem = va_info->vpn_size % va_info->max_bits;

    if(bits_rem > 0){
        va_info->levels++;
        va_info->level_info = (level_info_t**)Malloc(va_info->levels*sizeof(level_info_t*));
        for(uint32_t i = 0; i < va_info->levels; i++){
            va_info->level_info[i] = (level_info_t*)Malloc(sizeof(level_info_t));
            if(i == va_info->levels-1){
                va_info->level_info[i]->bits = bits_rem;
                va_info->level_info[i]->entries = (uint32_t)pow(2.0, (double)bits_rem);
                va_info->level_info[i]->bm_size = va_info->level_info[i]->entries / 32;
            }
            else{
                va_info->level_info[i]->bits = va_info->max_bits;
                va_info->level_info[i]->entries = (uint32_t)pow(2.0, (double)va_info->max_bits);
                va_info->level_info[i]->bm_size = va_info->level_info[i]->entries / 32;
            }
        }
    }
    else{
        va_info->level_info = (level_info_t**)Malloc(va_info->levels*sizeof(level_info_t*));
        for(uint32_t i = 0; i < va_info->levels; i++){
                va_info->level_info[i]->bits = va_info->max_bits;
                va_info->level_info[i]->entries = (uint32_t)pow(2.0, (double)va_info->max_bits);
                va_info->level_info[i]->bm_size = va_info->level_info[i]->entries / 32;
        }
    }
    return va_info;
}

void destroy_virtual_address_info(VA_info_t* va_info){
    for(uint32_t i = 0; i < va_info->levels; i++){
        free(va_info->level_info[i]);
    }
    free(va_info->level_info);
    free(va_info);
}

pd_t* init_page_directory(VA_info_t* va_info){
    pd_t* pd = (pd_t*)Malloc(sizeof(pd_t));
    pd->valid_bm = (uint32_t*)Malloc(va_info->level_info[0]->bm_size);
    memset(pd->valid_bm, 0, va_info->level_info[0]->bm_size);
    pd->pde = (pde_t*)Malloc(va_info->level_info[0]->entries*sizeof(pde_t));
    pd->pt = (pt_t**)Malloc(va_info->level_info[0]->entries*sizeof(pt_t*));
    return pd;
}
void destroy_page_directory(pd_t* pd, VA_info_t* va_info){
    /*TODO*/
}
pt_t* init_page_table(VA_info_t* va_info, pd_t* pd_root, pt_t* pt_par, uint32_t lvl){
    pt_t* pt = (pt_t*)Malloc(sizeof(pt_t));
    pt->valid_bm = (uint32_t*)Malloc(va_info->level_info[lvl]->bm_size);
    memset(pt->valid_bm, 0, va_info->level_info[lvl]->bm_size);
    pt->pte = (pte_t*)Malloc(va_info->level_info[lvl]->entries*sizeof(pte_t));
    pt->root = pd_root;
    pt->parent = pt_par;
    if (lvl == va_info->levels-1){
        pt->pt_nxt = NULL;
    }
    else{
        pt->pt_nxt = (pt_t**)Malloc(va_info->level_info[lvl]->entries*sizeof(pt_t*));
    }
    pt->level = lvl;
    return pt;
}
void destroy_page_table(pt_t* pt, VA_info_t* va_info){
    if(pt->pt_nxt != NULL){
        uint32_t t = va_info->levels - pt->level;
        /*TODO:*/
    }
}
void *t_malloc(unsigned int num_bytes){
    void* va;
    if(num_bytes <= PGSIZE){
        if(first_free == ram){
            uint32_t address = 0x00000000;
            va = &address;
        }
        uint32_t virtual_offset = PGSIZE/ENTRY_SIZE;
        first_free = (ram + virtual_offset);
    }
    return va;
}

/*Util functions*/
void* Malloc(size_t size){
    void* ptr = malloc(size);
    if(ptr == NULL){
        perror("malloc");
        exit(1);
    }
    return ptr;
}


int main(){
    /*Allocate RAM*/
    ram = mmap(NULL, MEMSIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
    if(ram == MAP_FAILED){
        printf("Mapping Failed\n");
        exit(1);
    }

    uint32_t phys_bm_size = (MEMSIZE / PGSIZE) / 32;
    phys_bm = (uint32_t*)Malloc(phys_bm_size);
    memset((void*)phys_bm, 0, phys_bm_size);
    first_free = ram;



    /*Deallocate RAM*/
    int err = munmap(ram, MEMSIZE);
    if(err != 0){
        printf("UnMapping Failed\n");
        exit(1);
    }  
    
    return 0;
}








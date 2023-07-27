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
#define ENTRY_SIZE 4 //bytes

#define PGSIZE 4096 //16384 //4096 //2048 //1024

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024 // 4GB

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024 // 1GB

// Represents a page table entry
typedef unsigned long pte_t;
// Represents a page directory entry
typedef unsigned long pde_t;

typedef unsigned long align;

typedef union page page_t;

typedef struct page2 page2_t;

typedef struct pt pt_t;

typedef struct bm bm_t;

union page { /* block header */
    struct {
        char x;
    } s;
    align a; /* Force alignment */
};

struct page2{
    char x;
};

struct pt{
    char* bitmap;
    pte_t entry;
};

struct bm{
    unsigned long pages;
    unsigned long* bitmap;
    int free_page;
};


/*Globals*/
unsigned long* ram;
bm_t* ram_bm;
bm_t* pd_bm;
bm_t** pt_bms;
bool pt_bms_is_empty = 1;
int free_pt = 0;


void* Malloc(size_t size){
    void* ptr = malloc(size);
    if(ptr == NULL){
        perror("malloc");
        exit(1);
    }
    return ptr;
}

unsigned long* init_bitmap(size_t size){
    unsigned long* bm = (unsigned long*)Malloc(size);
    memset(bm, 0, size);
    return bm;
}

bm_t* init_bm(unsigned int size){
    bm_t* bm = (bm_t*)Malloc(sizeof(bm_t));
    bm->pages = size*32;
    bm->bitmap = init_bitmap(size);
    bm->free_page = 0;
    return bm;
}

void destroy_bm(bm_t* bm){
    free(bm->bitmap);
    free(bm);
}
/*Check if a given bit is set in a given bitmap.*/
bool is_set(bm_t* bm, unsigned long page_num)
{
    if(page_num < 0 || page_num > bm->pages-1){
        perror("page_num out of range");
        return false;
    }
    unsigned long byte = page_num / 32;
    unsigned long bit = page_num % 32;
    unsigned long mask = (0x80000000>>bit);
    return bm->bitmap[byte] & mask;
}
/*Set a given bit in a given bitamp.*/
void flip_bit(bm_t* bm, unsigned long page_num)
{
    if(page_num < 0 || page_num > bm->pages-1){
        perror("page_num out of range");
        return;
    }
    unsigned long byte = page_num / 32;
    unsigned long bit = page_num % 32;
    unsigned long mask = (0x80000000>>bit);
    bm->bitmap[byte] ^= mask;
}
/*Finds first free bit in given bitmap. If not bits are free return -1.*/
void find_first_free(bm_t* bm){
    for(int i = 0; i < bm->pages; i++){
        if(!is_set(bm, i)){
            bm->free_page = i;
        }
    }
    bm->free_page = -1;
}

pte_t *translate(pde_t *pgdir, void *va) {
    unsigned long pd_idx = (unsigned long)va >> 22;
    printf("%lu\n", pd_idx);

    unsigned long pt_idx = ((unsigned long)va >> 12) & 1023;
    printf("%lu\n", pt_idx);

    unsigned long offs = (unsigned long)va & 1023;
    printf("%lu\n", offs);
    pte_t* page_table = (unsigned long*)pgdir[pd_idx];
    unsigned int ram_idx = page_table[pt_idx];
    pte_t* pa = ram + (ram_idx*1024) + offs;
    return pa;
}

int page_map(pde_t *pgdir, void *va, void* pa){
    return 0;
}

void *t_malloc(unsigned int num_bytes){
    void* va = 0;
    if(ram_bm->free_page == -1){
        perror("memory full");
        return NULL;
    }

    /*If no page table bitmaps have been initalized yet, initialize the first one*/
    if(pt_bms_is_empty){
        bm_t* new_bm = (bm_t*)Malloc(sizeof(bm_t));
        pt_bms[0] = new_bm;
        pt_bms_is_empty = 0;
    }
    /*If the current page table bitmap marked as free is not actually free 
     *and the page directory has free entries available, make a new page 
     *table bitmap at the free position in the page directory bitmap */
    else if(pt_bms[free_pt]->free_page == -1 && pd_bm->free_page != -1){
        bm_t* new_bm = (bm_t*)Malloc(sizeof(bm_t));
        pt_bms[pd_bm->free_page] = new_bm;
        find_first_free(pd_bm);
    }
    /*Page table entry = index of next free page in ram*/
    pte_t pte = (unsigned long)ram_bm->free_page;
    /*set bit corresponding to free entry in page table that will contain new PPN*/
    flip_bit(pt_bms[free_pt], pt_bms[free_pt]->free_page);
    /*find next free entry in page table where new PPN is stored.*/
    find_first_free(pt_bms[free_pt]);
    
    /*pte must be stored in page table at free position of free_pt page table*/
    unsigned long free = (unsigned long)pt_bms[free_pt]->free_page;
    unsigned long pt_mask = free << 12;
    /*pde_t pde = address of page table where pte is stored*/
    /*pd_idx = index of page directory that corresponds to pde */
    /*unsigned long pd_mask = pd_idx << 22;*/
    /*Build va with masks*/

    /*Set free_pt to index of next page table with free entries in pt_bms.*/
    bool min_set = false;
    int free_pt_prior = free_pt;
    for(int i = 0; i < pd_bm->pages; i++){
        if(pt_bms[i] == NULL && !min_set){
            free_pt = i;
            min_set = true;
        }
        if(pt_bms[i] != NULL && pt_bms[i]->free_page != -1){
            free_pt = i;
        }
    }
    /*No free page tables available*/
    if(free_pt_prior == free_pt && pt_bms[free_pt]->free_page == -1){
        free_pt = -1;
    }
    return va;
}


int main(){
    double vpn_level_bits = 10.0;
    ram_bm = init_bm((MEMSIZE / PGSIZE) / 32);
    pd_bm = init_bm((unsigned long)pow(2.0, vpn_level_bits)/32);
    pt_bms = (bm_t**)Malloc(pd_bm->pages*sizeof(bm_t*));
    printf("%i\n", sizeof(bm_t));

    ram = mmap(NULL, MEMSIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
    if(ram == MAP_FAILED){
        printf("Mapping Failed\n");
        exit(1);
    }
    // pde_t* pd = ram;
    // flip_bit(ram_bm, 0); //4096 bytes / 4 bytes per entry = 1024 entries
    // ram_bm->free_page = 1;
    // // for(unsigned int i = 0; i < 1024; i++){
    // //     pd[i] = i;
    // // }
    // pte_t* pt = ram + 1024;
    // flip_bit(ram_bm, 1);
    // ram_bm->free_page = 2;

    // pd[0] = (unsigned long)pt;
    // pt[4] = 5;
    // unsigned long idx = pt[4]*(PGSIZE/ENTRY_SIZE);
    // unsigned long offset = 10;
    // *(ram + idx + offset) = 1212121;

    // unsigned long address = 0;
    // address ^= 10;
    // address ^= (4 << 12);
    // printf("%lu\n", address);
    // pte_t* result = translate(pd, (unsigned int*)address);
    // printf("result: %lu\n", *result);


    // printf("%lu\n", *(unsigned long*)pd[0]);
    // printf("%lu\n", *(ram + 1024));

    // printf("%lu\n", *(ram + idx + offset));
    // printf("%lu\n", *(ram + (5*1024) + 10));

    // unsigned int pd_mask = 1 << 22;
    // unsigned int pt_mask = 1 << 12;
    // pt ^= pd_mask;
    // pt ^= pt_mask;
    // printf("%lu\n", pt);

    // printf("%i\n", sizeof(pte_t*));


    // printf("pd[1023] = %lu\n", pd[1023]);
    // printf("pt = %lu\n", *pt);






    // pte_t x = 0;
    // pde_t mask = 0 << 22;
    // // printf("%lu\n", mask);
    // pte_t mask2 =  1 << 12;

    // x ^= mask;
    // printf("%lu\n", x);
    // x^= mask2;
    // printf("%lu\n", x);
    
    // unsigned long va = 0;
    // unsigned long offset = 0;
    // printf("%lu\n", va);
    // va^=offset;
    // printf("%lu\n", va);
    // va^= mask2;
    // printf("%lu\n", va);
    // va^=mask;
    // printf("va: %lu\n", va); //0000000000 0000000001 000000000000 (0, 1, 0)

    // unsigned long pd_bits = va >> 22;
    // printf("%lu\n", pd_bits);

    // unsigned long pt_bits = (va >> 12) & 1023;
    // printf("%lu\n", pt_bits);

    // unsigned long offs = va & 1023;
    // printf("%lu\n", offs);

    
    // pde_t* pd = ram; //VA = 0000000000 0000000000 000000000000 (32-bit)
    // pd[0] = 4;

    // printf("%lu\n", pd[0]);
    // printf("%lu\n", ram[0]);
    // printf("%lu\n", ram[268435455]);

    

    // unsigned long store = 4294967295;
    
    // *(ram+(pt_bits*PGSIZE)+(offs*ENTRY_SIZE)) = store;

    // printf("%lu\n", *(ram+(pt_bits*PGSIZE)+(offs*ENTRY_SIZE)));

    // printf("page size: %u\n", sizeof(page2_t));
    // printf("aligned page size: %u\n", sizeof(page_t));

    // printf("page table size: %u\n", sizeof(pt_t));



    /*Deallocate RAM*/
    int err = munmap(ram, MEMSIZE);
    if(err != 0){
        printf("UnMapping Failed\n");
        exit(1);
    }  
    return 0;
}
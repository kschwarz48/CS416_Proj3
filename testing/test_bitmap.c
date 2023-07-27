#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need
/*32-bit address space specs*/
#define ADDRESS_SPACE 32 // bits
#define ENTRY_SIZE 4 //bytes

/*64-bit address space specs*/
// #define ADDRESS_SPACE 64 // bits
// #define ENTRY_SIZE 8 //bytes

#define PGSIZE 4096 //16384 //4096 //2048 //1024

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024 // 4GB

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024 // 1GB

uint32_t* bitmap_phys;

void* Malloc(uint32_t size){
    void* ptr = malloc(size);
    if(ptr == NULL){
        perror("malloc");
        exit(1);
    }
    return ptr;
}
/*page_num indicates which byte in the bitmap to reference*/
/*is_free() references bits from left to right(most significant bit to least significant bit)*/
/*returns 1 if a page is free at bit location page_num (bit == 0)*/
/*returns 0 if page is allocated (bit == 1)*/
bool is_set(uint32_t* bitmap, uint32_t page_num)
{
    uint32_t byte = page_num / 32;
    uint32_t bit = page_num % 32;
    uint32_t mask = (0x80000000>>bit);
    return bitmap[byte] & mask;
}

void flip_bit(uint32_t* bitmap, uint32_t page_num)
{
    uint32_t byte = page_num / 32;
    uint32_t bit = page_num % 32;
    uint32_t mask = (0x80000000>>bit);
    bitmap[byte] ^= mask;
}

void test_bitmap_funcs(uint32_t* bitmap, uint32_t max_pages){
    for(uint32_t pn = 0; pn < max_pages; pn++){
        printf("page %i is set: %s\n", pn, is_set(bitmap, pn) ? "true" : "false");
        flip_bit(bitmap, pn);
        uint32_t byte = pn / 32;
        printf("decimal value: %u\n", bitmap[byte]);
        printf("flipped bit at page %i\n", pn);
        printf("page %i is set: %s\n", pn, is_set(bitmap, pn) ? "true" : "false");
        flip_bit(bitmap, pn);
        printf("********************************\n"); 
    }
}

unsigned long* init_bitmap(size_t size){
    unsigned long* bm = (unsigned long*)Malloc(size);
    memset(bm, 0, size);
    return bm;
}

int main(int argc, char** argv){
    /*VIRTUAL MEMORY BITMAPS (32-bit address space)*/
    /*offset = log_2(PAGE SIZE)*/
    uint32_t offset = (uint32_t)log2(PGSIZE);
    /*VPN = ADDRESS SPACE - offset*/
    uint32_t vpn_size = ADDRESS_SPACE - offset;
    
    uint32_t max_entries = PGSIZE/ENTRY_SIZE;
    uint32_t max_bits = (uint32_t)log2(max_entries);
    uint32_t levels = (vpn_size/max_bits) + 1;
    uint32_t bits_rem = vpn_size % max_bits;

    uint32_t* level_sizes;

    if(bits_rem > 0){
        levels++;
        level_sizes =  (uint32_t*)Malloc(levels * 3 * sizeof(uint32_t));
        for(uint32_t i = 0; i < levels; i++){
            if(i == levels-1){
                level_sizes[i*3] = offset;
                level_sizes[i*3+1] = PGSIZE;
                level_sizes[i*3+2] = level_sizes[i*3+1] / 32;
            }
            else if(i == levels-2){
                level_sizes[i*3] = bits_rem;
                level_sizes[i*3+1] = (uint32_t)pow(2.0, (double)bits_rem);
                level_sizes[i*3+2] = level_sizes[i*3+1] / 32;
            }
            else{
                level_sizes[i*3] = max_bits;
                level_sizes[i*3+1] = (uint32_t)pow(2.0, (double)max_bits);
                level_sizes[i*3+2] = level_sizes[i*3+1] / 32;
            }
        }
    }
    else{
        level_sizes =  (uint32_t*)Malloc(levels * 3 * sizeof(uint32_t));
        for(uint32_t i = 0; i < levels; i++){
            if(i == levels-1){
                level_sizes[i*3] = offset;
                level_sizes[i*3+1] = PGSIZE;
                level_sizes[i*3+2] = level_sizes[i*3+1] / 32;
            }
            else{
                level_sizes[i*3] = max_bits;
                level_sizes[i*3+1] = (uint32_t)pow(2.0, (double)max_bits);
                level_sizes[i*3+2] = level_sizes[i*3+1] / 32;
            }
        }
    }
    /*TODO: allocate virtual memory bitmaps*/
    // /*Allocate bitmaps*/
    // bitmap_l1 = (char*)Malloc(bitmap_size_l1);
    // bitmap_l2 = (char*)Malloc(bitmap_size_l2);
    // bitmap_l3 = (char*)Malloc(bitmap_size_l3);
    // /*Set all bits to 0*/
    // memset((void*)bitmap_l1, 0, bitmap_size_l1);
    // memset((void*)bitmap_l2, 0, bitmap_size_l2);
    // memset((void*)bitmap_l3, 0, bitmap_size_l3);

    /*PHYSICAL MEMORY BITMAP (32-bit address space)*/
    uint32_t bitmap_phys_size = (MEMSIZE / PGSIZE) / 32;
    bitmap_phys = (uint32_t*)Malloc(bitmap_phys_size);
    memset((void*)bitmap_phys, 0, bitmap_phys_size);

    /*Print stats*/
    printf("ADDRESS SPACE: %i | PAGE SIZE: %i | MEM SIZE: %i | MAX MEM SIZE: %lli\n", ADDRESS_SPACE, PGSIZE, MEMSIZE, MAX_MEMSIZE);
    printf("offset: %i\n", offset);
    printf("VPN: %i\n", vpn_size);
    printf("max_entries: %i | max_bits: %i | levels: %i | bits_rem: %i\n", max_entries, max_bits, levels, bits_rem);
    printf("level_sizes: | ");
    for(uint32_t i = 0; i < levels; i++){
        printf("level %i: %i bits, %i entries per page, %i (bitmap size) | ", i+1, level_sizes[i*3], level_sizes[i*3+1], level_sizes[i*3+2]);
    }
    printf("\n");
    printf("PHYSICAL BITMAP SIZE: %i\n", bitmap_phys_size);
    printf("\n");

    // uint32_t* bitmap = malloc(5*sizeof(uint32_t));
    // memset((uint32_t*)bitmap, 0, 5*sizeof(uint32_t));
    
    // // flip_bit(bitmap, 0);
    // // flip_bit(bitmap, 17);
    // // flip_bit(bitmap, 29);
    // // flip_bit(bitmap, 31);

    // test_bitmap_funcs(bitmap, 5*32);

    // test_bitmap_funcs(bitmap_phys, (MEMSIZE / PGSIZE));

    // if(argc == 2){
    //     int arg = atoi(argv[1]);
    //     if(arg == 0){
    //         printf("*****************************************************\n");
    //         printf("*** Testing Bitmap Functions on Virtual Bitmap L1 ***\n");
    //         printf("*****************************************************\n");
    //         test_bitmap_funcs(bitmap_l1, max_pages_l1);
    //     }
    //     else if(arg == 1){
    //         printf("*****************************************************\n");
    //         printf("*** Testing Bitmap Functions on Virtual Bitmap L2 ***\n");
    //         printf("*****************************************************\n");
    //         test_bitmap_funcs(bitmap_l2, max_pages_l2);
    //     }
    //     else if(arg == 2){
    //         printf("*****************************************************\n");
    //         printf("*** Testing Bitmap Functions on Virtual Bitmap L3 ***\n");
    //         printf("*****************************************************\n");
    //         test_bitmap_funcs(bitmap_l3, PGSIZE);
    //     }
    //     else if(arg == 3){
    //         printf("***************************************************\n");
    //         printf("*** Testing Bitmap Functions on Physical Bitmap ***\n");
    //         printf("***************************************************\n");
    //         test_bitmap_funcs(bitmap_phys, (MEMSIZE / PGSIZE));
    //     }
    //     else{
    //         printf("Bad Usage: Must pass in integer(0 = bm1, 1 = bm2, 2 = bm3, 3 = bmp)\n");
    //     }
    // }

    // /*free bitmaps*/
    // free(bitmap_l1);
    // free(bitmap_l2);
    // free(bitmap_l3);
    // free(bitmap);
    free(bitmap_phys);
    free(level_sizes);

    return 0;
}
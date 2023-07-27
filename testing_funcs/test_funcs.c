//Authors: Scott Skibin (ss3793), Kevin Schwarz (kjs309)
//Course: CS416
//iLab server used: cp.cs.rutgers.edu
//This is a file to test correctness of library functions
#include "../my_vm.h"
#include <limits.h>

pte_t* ram;
pde_t* page_dir;
VA_info_t* va_info;
bitmap_t* ram_bm;
bitmap_t* page_dir_bm;
bitmap_t* virtual_bm;

int main(int argc, char** argv){
    set_physical_mem();
    /*******************************************************************************************************/
    /*Display Calculated Virtual Address Info*/
    /*******************************************************************************************************/
    printf("MEM SIZE: %u | MAX MEM SIZE: %lli\n", MEMSIZE, MAX_MEMSIZE);
    printf("ADDRESS SPACE: %lu | ENTRY SIZE: %lu| PAGE SIZE: %u\n", va_info->address_space, va_info->entry_size, PGSIZE);
    printf("VPN: %lu | OFFSET: %lu\n", va_info->vpn_size, va_info->offset);
    printf("MAX ENTRIES: %lu | MAX BITS: %lu | LEVELS: %lu | REMAINING BITS: %lu\n", va_info->max_entries, va_info->max_bits, va_info->levels, va_info->bits_rem);
    printf("PHYSICAL MEM BITMAP SIZE: %lu\n", va_info->phys_bm_size);
    printf("LEVEL INFO:\n");
    printf("------------\n");
    for(unsigned long i = 0; i < va_info->levels; i++){
        printf("level %lu: %lu bits, %lu build bits, %lu entries per page, %lu (bitmap size)\n", i+1, va_info->level_info[i]->bits, va_info->level_info[i]->build_bits, va_info->level_info[i]->entries, va_info->level_info[i]->bm_size);
    }
    printf("***********************************************************************************************\n");
    if(argc == 2){    
    /*******************************************************************************************************/
    /*Testing t_malloc() / t_free() / put_value() / get_value()*/
    /*******************************************************************************************************/
        int arg = atoi(argv[1]);
        if(arg == 0){
            int n = (2*(PGSIZE/va_info->entry_size))+20;
            printf("n: %i\n", n);

            void* va = t_malloc(400);
            void* va1 = t_malloc(400);
            void* va2 = t_malloc(n*sizeof(int));
            // void* va2 = t_malloc(PGSIZE);
            printf("va: %u\n", (unsigned int)va2);

            //[0, 4, 0]
            int addr = 0;
            addr = (unsigned int)va2 + 5;
            printf("addr: %u\n", addr);

            // int x = 1030;
            // int x_arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
            int* x_darr = (int*)Malloc(n*sizeof(int));
            for(int a = 0; a < n; a++){
                x_darr[a] = a+1;
            }
            // put_value((void *)addr, &x, sizeof(int));
            // put_value((void *)addr, &x_arr, 10*sizeof(int));
            put_value((void *)addr, (void*)x_darr, n*sizeof(int));

            // int y;
            // int y_arr[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            int* y_darr = (int*)Malloc(n*sizeof(int));
            for(int b = 0; b < n; b++){
                y_darr[b] = 0;
            }
            // get_value((void *)addr, &y, sizeof(int));
            // get_value((void *)addr, &y_arr, 10*sizeof(int));
            get_value((void *)addr, (void*)y_darr, n*sizeof(int));

            // printf("y: %d\n", y);
            // for(int i = 0; i < 10; i++){
            //     printf("y %i: %d\n", i, y_arr[i]);
            // }
            for(int i = n-10; i < n; i++){
                printf("y %i: %d\n", i, y_darr[i]);
            }

            // printf("ram: %lu\n", *(ram + (4*(PGSIZE/va_info->entry_size)) + 5));
            for(int i = n-10; i < n; i++){
                printf("ram %i: %lu\n", i, *(ram + (4*(PGSIZE/va_info->entry_size)) + i));
            }

            display_bitmap(ram_bm, 1);
            display_bitmap(virtual_bm, 1);

            printf("free1:\n");
            t_free(va1, PGSIZE);
            display_bitmap(ram_bm, 1);
            display_bitmap(virtual_bm, 1);

            printf("free2:\n");
            t_free(va2, n*sizeof(int));
            display_bitmap(ram_bm, 1);
            display_bitmap(virtual_bm, 1);

            printf("free3:\n");
            t_free(va, PGSIZE);
            display_bitmap(ram_bm, 1);
            display_bitmap(virtual_bm, 1);


            
        }
        else if(arg == 1){
        /*******************************************************************************************************/
        /*Testing filling physical memory with t_malloc() and then freeing everything with t_free() and then repeating this once*/
        /*******************************************************************************************************/

            //(MEMSIZE/PGSIZE) //1*65536 = 16319 //2*65536 = 8159 //3*65536 = 5439
            //(MEMSIZE/PGSIZE) //1*32768 = 32639 //2*32768 = 16319 //3*32768 = 10879
            //(MEMSIZE/PGSIZE) //1*16384 = 64526 //2*16384 = 32263 //3*16384 = 21508
            //(MEMSIZE/PGSIZE) //1*4096 = 261887 //2*4096 = 130943 //3*4096 = 87294 //4*4096 = 65471
            //(MEMSIZE/PGSIZE) //1*1024 = 1032380 //2*1024 = 516190 //3*1024 = 344126

            unsigned long bytes = 261887;
            unsigned long n = 1;
            pte_t** virts = (pte_t**)Malloc(bytes*sizeof(pte_t*));
            printf("Allocating all pages in physical memory...\n");
            for(int i = 0; i < bytes; i++){ 
                virts[i] = t_malloc((n*PGSIZE));
                if(i % 10000 == 0){
                    printf("%i:: ", i);
                    printf("va: %lu\n", (pte_t)virts[i]);
                }
            }
            printf("Freeing all allocations...\n");
            for(int l = 0; l < bytes; l++){ 
                if(l % 10000 == 0){
                    printf("free %i:: ", l);
                    printf("va: %lu\n", (pte_t)virts[l]);
                }
                t_free(virts[l], (n*PGSIZE)); 
            }
            free(virts);

            display_bitmap(page_dir_bm, 0);

            virts = (pte_t**)calloc(bytes, sizeof(pte_t*));
            printf("Allocating all pages in physical memory again...\n");
            for(int i = 0; i < bytes; i++){ 
                virts[i] = t_malloc((n*PGSIZE));
                if(i % 10000 == 0){
                    printf("%i:: ", i);
                    printf("va: %lu\n", (pte_t)virts[i]);
                }
            }
            printf("Freeing all allocations again...\n");
            for(int l = 0; l < bytes; l++){ 
                if(l % 10000 == 0){
                    printf("free %i:: ", l);
                    printf("va: %lu\n", (pte_t)virts[l]);
                }
                t_free(virts[l], (n*PGSIZE)); 
            }
            free(virts);
            printf("Program Completed Succesfully.\n");
        }
        else if(arg == 2){
        /*******************************************************************************************************/
        /*Testing Bitmaps*/
        /*******************************************************************************************************/
            bitmap_t* bm0 = init_bitmap(128, NULL, true, 0);
            printf("free bm0: %i\n", bm0->free_page);
            bitmap_t* bm1 = init_bitmap(128, bm0, true, 1);
            flip_bit(bm1, 20);
            printf("free bm1: %i\n", bm1->free_page);
            bitmap_t* bm2 = init_bitmap(128, bm1, false, 2);
            flip_bit(bm2, 22*32);
            printf("free bm2: %i\n", bm2->free_page);

            bm0->children[0] = bm1;
            bm1->children[0] = bm2;
            display_bitmap(bm0, 0);
            display_bitmap(bm0->children[0], 0);
            display_bitmap(bm0->children[0]->children[0], 0);
        }
        else{
            printf("Bad Usage: Must pass in integer(0 = simple library function testing, 1 = filling memory testing. 2 = bitmap testing)\n");
            exit(1);
        }
    }
    else{
        printf("Bad Usage: Must pass in integer(0 = simple library function testing, 1 = filling memory testing. 2 = bitmap testing)\n");
        exit(1); 
    }
    destroy_physical_mem();
    return 0;
}
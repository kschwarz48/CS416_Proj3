#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define MEMSIZE 1024*1024*1024
#define N 10
int* ram;

int main(int argc, char** argv){
    if(argc == 2){
        int arg = atoi(argv[1]);
        if(arg == 0){
            printf("malloc() in use...\n");
            ram = malloc(MEMSIZE);
            if(ram == NULL){
                printf("Malloc Failed\n");
                exit(1);
            }
        }
        else if(arg == 1){
            printf("mmap() in use...\n");
            ram = mmap(NULL, MEMSIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
            if(ram == MAP_FAILED){
                printf("Mapping Failed\n");
                exit(1);
            }
        }
        else{
            printf("Bad Usage: Must pass in integer(0 = malloc(), 1 = mmap())\n");
            exit(1);
        }

        // for(int i=0; i<N; i++){
        //     ram[i] = i*10;
        // }
        

        // for(int i=0; i<N; i++){
        //     printf("[%d] ", ram[i]);
        // }
        // printf("\n");

        printf("ram addr: %X\n", ram);
        int* value;
        for(int i=0; i<N; i++){
            *(ram + i) = i*10;
            if(i == 4){
                value = (ram + i);
            }
        }
        int* t;
        t = 5;
        printf("%i\n", t);
        printf("%X\n", &t);
        printf("value: %i\n", *value);
        *value += 5;
        

        for(int i=0; i<N; i++){
            printf("[%d] ", *(ram + i));
        }
        printf("\n");

        if(arg == 0){
            free(ram);
        }
        else{
            int err = munmap(ram, MEMSIZE);
            if(err != 0){
                printf("UnMapping Failed\n");
                exit(1);
            }   
        }
    }
    else{
        printf("Bad Usage: Must pass in integer(0 = malloc(), 1 = mmap())\n");
        exit(1); 
    }
    return 0;
}
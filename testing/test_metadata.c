#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#define ALIGNMENT 8 // must be a power of 2
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t))) // header size


/* align to long boundary */
typedef long Align;

typedef union header { /* block header */
    struct {
        union header *ptr;
        uint32_t size;
    } s;
    Align x; /* Force alignment */
} Header;

typedef struct header2 header_t;

struct header2 {
    size_t size;
    int alloc;
    header_t *next;
};

int main(int argc, char** argv){

    printf("%i\n", sizeof(Header));
    printf("%i\n", sizeof(Align));
    printf("%i\n", sizeof(size_t));
    printf("%i\n", sizeof(header_t));
    printf("%i\n", sizeof(header_t*));
    printf("%i\n", sizeof(int));

    Header* h1 = (Header*)malloc(sizeof(Header)); 
    Header* h2 = (Header*)malloc(sizeof(Header));

    h1->s.ptr = h2;
    h1->s.size = 4;
    h1->s.ptr->s.ptr = NULL;
    h1->s.ptr->s.size = 5;

    printf("h1: %u\n", h1->s.size);
    printf("h2: %u\n", h2->s.size);

    free(h1->s.ptr);
    free(h1);

    return 0;
}
#include "my_vm.h"


// Global Variables
pde_t *page_directory;

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {
    
    // Try to allocate physical memory using mmap()
    physical_mem = mmap(NULL, PHYSICAL_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (physical_mem == MAP_FAILED) {
        // If mmap() fails, fall back to malloc()
        physical_mem = malloc(PHYSICAL_MEM_SIZE);
        if (physical_mem == NULL) {
            perror("Error allocating physical memory");
            exit(EXIT_FAILURE);
        }
    }
    
    // Initialize virtual and physical bitmaps
    num_physical_pages = PHYSICAL_MEM_SIZE / PGSIZE;
    num_virtual_pages = num_physical_pages * NUM_PROCESSES;
    
    // Allocate memory for the virtual bitmap and initialize it to zero
    virtual_bitmap = (uint8_t *)malloc(num_virtual_pages / 8);
    if (!virtual_bitmap) {
        perror("Error allocating memory for virtual bitmap");
        exit(EXIT_FAILURE);
    }
    memset(virtual_bitmap, 0, num_virtual_pages / 8);
    
    // Allocate memory for the physical bitmap and initialize it to zero
    physical_bitmap = (uint8_t *)malloc(num_physical_pages / 8);
    if (!physical_bitmap) {
        perror("Error allocating memory for physical bitmap");
        exit(EXIT_FAILURE);
    }
    memset(physical_bitmap, 0, num_physical_pages / 8);

    // Allocate memory for the page directory (root node) and initialize it to zero
    page_directory = (pde_t *)malloc(PGSIZE);
    if (!page_directory) {
        perror("Error allocating memory for the page directory");
        exit(EXIT_FAILURE);
    }
    memset(page_directory, 0, PGSIZE);
    
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
}



/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */

//Global Variables
unsigned int tlb_hits = 0;
unsigned int tlb_accesses = 0;

int
add_TLB(void *va, void *pa)
{
    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    unsigned long vpn = (unsigned long)va / PGSIZE;
    unsigned long ppn = (unsigned long)pa / PGSIZE;
    unsigned long tlb_index = vpn % TLB_ENTRIES;

    tlb_store.entries[tlb_index].vpn = vpn;
    tlb_store.entries[tlb_index].ppn = ppn;
    tlb_store.entries[tlb_index].valid = true;
    
    return 0;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {
    unsigned long vpn = (unsigned long)va / PGSIZE;
    unsigned int tlb_index = vpn % TLB_ENTRIES;
    /* Part 2: TLB lookup code here */
    if (tlb_store.entries[tlb_index].valid && tlb_store.entries[tlb_index].vpn == vpn) {
        unsigned long pa = tlb_store.entries[tlb_index].ppn * PGSIZE;
        return (pte_t *)pa;
    }

   /*This function should return a pte_t pointer*/
   return NULL;
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/




    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */

    // Checking if translation exists in TLB
    pte_t *tlb_result = check_TLB(va);
    if (tlb_result != NULL) {
        return tlb_result;
    }

    void *pa = NULL;


    // if not found in tlb, do translation
    unsigned long pgd_index = PGD_INDEX(va); //get page directory index
    unsigned long pt_index = PT_INDEX(va); // get page table index
    pde_t pgd_entry = pgdir[pgd_index]; // get page directory entry

    //check if entry exists
    if (pgd_entry & 0x1) {
        pte_t *pte_base = (pte_t *)(pgd_entry & ~0xFFF); // get base address
        pte_t pte_entry = pte_base[pt_index]; // get page table entry

        // check if entry exists
        if (pte_entry & 0x1) {
            pa = (void *)((pte_entry & ~0xFFF) | OFFSET_INDEX(va)); // Get the physical address
        }
    }    


    // If translation successful, add to TLB
    if (pa != NULL) {
        add_TLB(va, pa);
    } else {
        return NULL; //If translation not successful, then return NULL
    }

    return pa;
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    return -1;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
 
    //Use virtual address bitmap to find the next free page
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *t_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

    // Allocate physical memory if not already initialized
    if (physical_mem == NULL) {
        set_physical_mem();
    }

    // Initialize the page directory if not already initialized
    if (page_directory == NULL) {
        page_directory = (pde_t *)get_next_avail(1); // Allocate a page for the page directory
        add_TLB(page_directory, physical_mem); // Map the page directory to the first page of physical memory
    }

    // Calculate the number of pages needed
    int num_pages = (num_bytes + PGSIZE - 1) / PGSIZE;

    // Find the next available pages
    void *va = get_next_avail(num_pages);
    if (va == NULL) {
        perror("Error: no available pages");
        return NULL;
    }

    // Mark the pages as used in the virtual and physical bitmaps
    mark_virtual_bitmap(va, num_pages);
    unsigned long pa = mark_physical_bitmap(num_pages);

    // Map the pages in the page table
    for (int i = 0; i < num_pages; i++) {
        unsigned long vpn = ((unsigned long)va + i * PGSIZE) / PGSIZE;
        unsigned long pte_addr = (unsigned long)get_next_avail(1); // Allocate a page for the page table
        add_TLB((void *)(vpn * PGSIZE), (void *)(pte_addr & ~0xFFF)); // Map the page table to a new physical page
        page_directory[PGD_INDEX((void *)(vpn * PGSIZE))] = (pde_t)(pte_addr | 0x1); // Set the valid bit and store the page table's physical address in the page directory

        pte_t *pte = (pte_t *)pte_addr;
        pte[PT_INDEX((void *)(vpn * PGSIZE))] = (pte_t)(pa + i * PGSIZE | 0x1); // Set the valid bit and store the physical address in the page table entry
    }

    return va;
   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void t_free(void *va, int size) {

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */
    int num_pages = (size + PGSIZE - 1) / PGSIZE;
    unsigned long vpn_start = (unsigned long)va / PGSIZE;

    for (int i = 0; i < num_pages; i++) {
        unsigned long vpn = vpn_start + i;
        pte_t *pte = translate(page_directory, (void *)(vpn * PGSIZE));

        if (pte == NULL || !(*pte & 0x1)) {
            // Invalid or not allocated page, can't free
            fprintf(stderr, "t_free(): trying to free a not allocated page, vpn: %lu\n", vpn);
            return;
        }

        // Free the physical page
        unsigned long ppn = *pte / PGSIZE;
        clear_physical_bitmap(ppn);
        *pte &= ~0x1; // Clear the valid bit of the PTE

        // Invalidate the TLB entry
        unsigned long tlb_index = vpn % TLB_ENTRIES;
        if (tlb_store.entries[tlb_index].valid && tlb_store.entries[tlb_index].vpn == vpn) {
            tlb_store.entries[tlb_index].valid = false;
        }
    }
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
 * The function returns 0 if the put is successfull and -1 otherwise.
*/
int put_value(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */
    int put_value(void *va, void *val, int size) {
    // Translate virtual address to physical address
    pte_t *pte = translate(page_directory, va);
    if (pte == NULL || !(*pte & 0x1)) {
        // Invalid or not allocated page, put_value fails
        fprintf(stderr, "put_value(): invalid or not allocated page, va: %p\n", va);
        return -1;
    }
    unsigned long pa = (*pte & ~0xFFF) + OFFSET_INDEX(va);

    // Copy value to physical memory
    unsigned char *phys_ptr = physical_mem + pa;
    unsigned char *val_ptr = (unsigned char *)val;
    for (int i = 0; i < size; i++) {
        phys_ptr[i] = val_ptr[i];
    }

    return 0; // Success
    /*return -1 if put_value failed and 0 if put is successfull*/
}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */


}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */
    int x, y, val_size = sizeof(int);
    int i, j, k;
    for (i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            unsigned int a, b, c = 0;
            for (k = 0; k < size; k++) {
                int address_a = (unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int));
                int address_b = (unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int));
                get_value( (void *)address_a, &a, sizeof(int));
                get_value( (void *)address_b, &b, sizeof(int));
                // printf("Values at the index: %d, %d, %d, %d, %d\n", 
                //     a, b, size, (i * size + k), (k * size + j));
                c += (a * b);
            }
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            // printf("This is the c: %d, address: %x!\n", c, address_c);
            put_value((void *)address_c, (void *)&c, sizeof(int));
        }
    }
}




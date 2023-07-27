//Authors: Scott Skibin (ss3793), Kevin Schwarz (kjs309)
//Course: CS416
//iLab server used: cp.cs.rutgers.edu

#include "my_vm.h"


<<<<<<< Updated upstream
// Global Variables
pde_t *page_directory;
=======
//Global Variables
pte_t* ram;
pde_t* page_dir;
VA_info_t* va_info;
bitmap_t* ram_bm;
bitmap_t* virtual_bm;
bitmap_t* page_dir_bm;
struct tlb* main_tlb = &tlb_store;
bool lib_init = false;

pthread_mutex_t lock_free;
pthread_mutex_t lock_put;
pthread_mutex_t lock_get;
pthread_mutex_t lock_matmul;

unsigned long tlb_misses = 0;
unsigned long tlb_lookups = 0;
>>>>>>> Stashed changes

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {
<<<<<<< Updated upstream
    
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
=======
    ram = Malloc(MEMSIZE);
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
    va_info = init_virtual_address_info();
    init_tlb(main_tlb);
    ram_bm = init_bitmap(va_info->phys_bm_size, NULL, false, -1);
    virtual_bm = init_bitmap(va_info->phys_bm_size, NULL, false, -1);
    flip_bit(virtual_bm, virtual_bm->last_free);
    virtual_bm->last_free--;
    page_dir_bm = init_bitmap(va_info->level_info[0]->bm_size, NULL, true, 0);
    page_dir = ram;
    page_dir_bm->phys_idx = 0;
    flip_bit(ram_bm, 0);
    
    if (pthread_mutex_init(&lock_free, NULL) != 0) {
        perror("mutex init has failed");
        exit(1);
    }
    if (pthread_mutex_init(&lock_put, NULL) != 0) {
        perror("mutex init has failed");
        exit(1);
    }
    if (pthread_mutex_init(&lock_get, NULL) != 0) {
        perror("mutex init has failed");
        exit(1);
    }
    if (pthread_mutex_init(&lock_matmul, NULL) != 0) {
        perror("mutex init has failed");
        exit(1);
    }
>>>>>>> Stashed changes
}

void destroy_physical_mem(){
    pthread_mutex_destroy(&lock_free);
    pthread_mutex_destroy(&lock_put);
    pthread_mutex_destroy(&lock_get);
    pthread_mutex_destroy(&lock_matmul);
    destroy_bitmap(page_dir_bm);
    destroy_bitmap(virtual_bm);
    destroy_bitmap(ram_bm);
    destroy_tlb(main_tlb);
    destroy_virtual_address_info(va_info);
    free(ram);
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int add_TLB(void *va, void *pa){
    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    tlb_misses++;
    unsigned long _va = (unsigned long)va >> va_info->offset & va_info->vpn_max_val;
    unsigned long idx = _va % TLB_ENTRIES;
    if(main_tlb->cache[idx] == NULL){
        main_tlb->cache[idx] = (tlb_entry_t*)Malloc(sizeof(tlb_entry_t));
        flip_bit(main_tlb->valid, idx);
    }
    main_tlb->cache[idx]->vpn = _va;
    main_tlb->cache[idx]->pfn = (unsigned long)pa;
    return 0;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t * check_TLB(void *va) {
    /* Part 2: TLB lookup code here */
    tlb_lookups++;
    unsigned long _va = (unsigned long)va >> va_info->offset & va_info->vpn_max_val;
    unsigned long idx = _va % TLB_ENTRIES;
    if(is_set(main_tlb->valid, idx)){
        return ram + (main_tlb->cache[idx]->pfn*(PGSIZE/va_info->entry_size));
    }
    else{
        return NULL;
    }
   /*This function should return a pte_t pointer*/
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void print_TLB_missrate()
{
    double miss_rate = 0;	
    /*Part 2 Code here to calculate and print the TLB miss rate*/
    miss_rate = ((double)tlb_misses/(double)tlb_lookups)*100;
    fprintf(stderr, "TLB miss rate %lf %%\n", miss_rate);
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
    unsigned long _va = (unsigned long)va;
    unsigned long offset = _va & (PGSIZE-1);

    pte_t* pa = check_TLB(va);
    if(pa != NULL){
        return pa + offset;
    }
    
    unsigned long* vpn_idxs = (unsigned long*)Malloc(va_info->levels*sizeof(unsigned long));
    for(unsigned int i = 0; i < va_info->levels; i++){
        vpn_idxs[i] = _va >> (va_info->level_info[i]->build_bits) & (va_info->level_info[i]->entries-1); 
    }
     
    bitmap_t* curr_bm = page_dir_bm;
    pde_t* curr_table = pgdir;
    int lvl = 0;
    while(curr_bm->children != NULL){
        curr_table = (pde_t*)curr_table[vpn_idxs[lvl]];
        if(curr_table == NULL){
            free(vpn_idxs);
            return NULL;
        }
        curr_bm = curr_bm->children[vpn_idxs[lvl]];
        lvl++;
    }
    if(!is_set(curr_bm, vpn_idxs[lvl])){
        free(vpn_idxs);
        return NULL;
    }
    unsigned long pfn = curr_table[vpn_idxs[va_info->levels-1]]; 
    add_TLB(va, (void*)pfn);
    pa = (pte_t*)(ram + (pfn*(PGSIZE/va_info->entry_size)) + offset);
    free(vpn_idxs);
    return pa;
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int page_map(pde_t *pgdir, void *va, void *pa)
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
<<<<<<< Updated upstream
=======
    static pthread_mutex_t lock_alloc = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock_alloc);
    if(num_bytes < 1){
        perror("Can not allocate requested number of bytes");
        exit(1);
    }
    if(!lib_init){
        set_physical_mem();
        lib_init = true;
    }
    if(num_bytes > (va_info->entry_size*PGSIZE)){
        perror("Can not allocate requested number of bytes");
        exit(1);
    }
    if(ram_bm->free_page == -1){
        perror("Physical memory is full");
        exit(1);
    }
   
    unsigned long va = 0;
    unsigned long* va_ptr = &va;

    unsigned long page_req = ceil((double)num_bytes/(double)PGSIZE);

    bitmap_t* curr_bm = page_dir_bm;
    pde_t* curr_table = page_dir;

    if(page_req > 1){
        if(!has_space(ram_bm, page_req)){
            perror("Not enough space for requested number of bytes");
            exit(1);
        }
        if(!find_first_contig(virtual_bm, page_req, va_ptr)){ 
            perror("Not enough space for requested number of bytes");
            exit(1);
        }
        else{
            unsigned long curr_va = va;
            unsigned long curr_vpi = va_to_vpi(curr_va);
            for(unsigned long i = 0; i < page_req; i++){

                unsigned long* vpn_idx = (unsigned long*)Malloc(va_info->levels*sizeof(unsigned long));
                for(unsigned int n = 0; n < va_info->levels; n++){
                    vpn_idx[n] = curr_va >> (va_info->level_info[n]->build_bits) & (va_info->level_info[n]->entries-1);
                }

                bitmap_t* new_bm;
                pde_t* new_phys;
                curr_bm = page_dir_bm;
                curr_table = page_dir;
                bool chil_flag = true;

                for(unsigned long j = 0; j < va_info->levels; j++){
                    if(j == va_info->levels-2){
                        chil_flag = false;
                    }
                    if(curr_bm->children != NULL){
                        if(curr_bm->children[vpn_idx[j]] == NULL){
                            new_bm = init_bitmap(va_info->level_info[j+1]->bm_size, curr_bm, chil_flag, j+1);
                            curr_bm->children[vpn_idx[j]] = new_bm;
                            new_phys = ram + (ram_bm->free_page*(PGSIZE/va_info->entry_size));
                            new_bm->phys_idx = ram_bm->free_page;
                            flip_bit(ram_bm, ram_bm->free_page);
                            curr_table[vpn_idx[j]] = (pde_t)new_phys;
                            flip_bit(virtual_bm, virtual_bm->last_free);
                            virtual_bm->last_free--;
                        }
                        curr_table = (pde_t*)curr_table[vpn_idx[j]];
                        curr_bm = curr_bm->children[vpn_idx[j]];
                    }
                    else{
                        curr_table[vpn_idx[j]] = ram_bm->free_page;
                        flip_bit(virtual_bm, curr_vpi);
                        add_TLB((void*)curr_va, (void*)ram_bm->free_page);
                        flip_bit(ram_bm, ram_bm->free_page);
                        flip_bit(curr_bm, vpn_idx[j]);
                        if(curr_bm->free_page == -1){
                            bitmap_t* curr_parent = curr_bm->parent;
                            for(int l = j; l >= 0; l--){
                                flip_bit(curr_parent, vpn_idx[l-1]);
                                if(curr_parent->free_page == -1 && curr_parent->parent != NULL){
                                    curr_parent = curr_parent->parent;
                                }
                                else{
                                    break;
                                }
                            }
                        }
                    }
                }
                curr_vpi++;
                curr_va = vpi_to_va(curr_vpi);
                free(vpn_idx);
            }   
        }       
    }
    else{
        if(find_first_free_rec(page_dir_bm, va_ptr, 0)){ 
            int lvl = 0;
            unsigned long curr_idx;
            while(curr_bm->children != NULL){
                curr_idx = va >> (va_info->level_info[lvl]->build_bits) & (va_info->level_info[lvl]->entries-1);
                curr_table = (pde_t*)curr_table[curr_idx];
                curr_bm = curr_bm->children[curr_idx];
                lvl++;
            }
            curr_table[curr_bm->free_page] = ram_bm->free_page;
            unsigned long vpi = va_to_vpi(va);
            flip_bit(virtual_bm, vpi);
            add_TLB((void*)va, (void*)ram_bm->free_page);
            flip_bit(ram_bm, ram_bm->free_page);
            flip_bit(curr_bm, curr_bm->free_page);
        }
        else{
            va ^= (unsigned long)page_dir_bm->free_page << va_info->level_info[0]->build_bits;
            bitmap_t* new_bm;
            pde_t* new_phys;
            bool chil_flag = true;
            for(unsigned long i = 1; i < va_info->levels; i++){
                if(i == va_info->levels-1){
                    chil_flag = false;
                }
                if(curr_bm->children[curr_bm->free_page] == NULL){
                    new_bm = init_bitmap(va_info->level_info[i]->bm_size, curr_bm, chil_flag, i);
                    curr_bm->children[curr_bm->free_page] = new_bm;
                    new_phys = ram + (ram_bm->free_page*(PGSIZE/va_info->entry_size));
                    new_bm->phys_idx = ram_bm->free_page;
                    flip_bit(ram_bm, ram_bm->free_page);
                    curr_table[curr_bm->free_page] = (pde_t)new_phys;
                    flip_bit(virtual_bm, virtual_bm->last_free);
                    virtual_bm->last_free--;
                }
                va ^= (unsigned long)curr_bm->children[curr_bm->free_page]->free_page << va_info->level_info[i]->build_bits;
                if(chil_flag){
                    curr_table = (pde_t*)curr_table[curr_bm->free_page];
                    curr_bm = curr_bm->children[curr_bm->free_page];
                }
                else{
                    ((pde_t*)curr_table[curr_bm->free_page])[curr_bm->children[curr_bm->free_page]->free_page] = ram_bm->free_page;
                    unsigned long vpi = va_to_vpi(va);
                    flip_bit(virtual_bm, vpi);
                    add_TLB((void*)va, (void*)ram_bm->free_page);
                    flip_bit(ram_bm, ram_bm->free_page);
                    flip_bit(curr_bm->children[curr_bm->free_page], curr_bm->children[curr_bm->free_page]->free_page);
                }
            }
        }
    }
    pthread_mutex_unlock(&lock_alloc);
    return (void*)va;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
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
=======
    pthread_mutex_lock(&lock_free);
    unsigned long pages = ceil((double)size/(double)PGSIZE);

    unsigned long* sizes = (unsigned long*)Malloc(pages*sizeof(unsigned long));
    unsigned long rem = size % PGSIZE;
    for(int n = 0; n < pages; n++){
        if(n == pages-1){
            if(rem == 0){
                sizes[n] = PGSIZE;
            }
            else{
                sizes[n] = rem;
            }
        }
        else{
            sizes[n] = PGSIZE;
        }
    }
    unsigned long curr_vpi = va_to_vpi((unsigned long)va);
    pte_t* phys = translate(page_dir, va);
    if(phys == NULL){
        perror("virtual address does not exist");
        free(sizes);
        exit(1);
    }
    unsigned long _va = (unsigned long)va;

    for(unsigned long p = 0; p < pages; p++){
        for(int k = 0; k < (sizes[p]/va_info->entry_size); k++){
            phys[k] = 0;
        }
        unsigned long* vpn_idxs = (unsigned long*)Malloc(va_info->levels*sizeof(unsigned long));
        for(unsigned int m = 0; m < va_info->levels; m++){
            vpn_idxs[m] = _va >> (va_info->level_info[m]->build_bits) & (va_info->level_info[m]->entries-1); 
        }
        bitmap_t* curr_bm = page_dir_bm;
        pde_t* prev_table;
        pde_t* curr_table = page_dir;
        for(unsigned int i = 0; i < va_info->levels; i++){
            if(curr_bm->children == NULL){
                flip_bit(curr_bm, vpn_idxs[i]);
                unsigned long vpi = va_to_vpi(_va);
                flip_bit(virtual_bm, vpi);
                flip_bit(ram_bm, curr_table[vpn_idxs[i]]);
                curr_table[vpn_idxs[i]] = 0;
                if(curr_bm->num_free >= curr_bm->pages){
                    flip_bit(ram_bm, curr_bm->phys_idx);
                    curr_bm = curr_bm->parent;
                    if(curr_bm->level == 0 && is_set(curr_bm, vpn_idxs[0])){
                        flip_bit(curr_bm, vpn_idxs[0]);
                    }
                    destroy_bitmap(curr_bm->children[vpn_idxs[i-1]]);
                    virtual_bm->last_free++;
                    flip_bit(virtual_bm, virtual_bm->last_free);
                    curr_bm->children[vpn_idxs[i-1]] = NULL;
                    prev_table[vpn_idxs[i-1]] = 0;
                    for(int j = i-1; j > 0; j--){
                        if(is_free_complete(curr_bm)){
                            flip_bit(ram_bm, curr_bm->phys_idx);
                            curr_bm = curr_bm->parent;
                            if(curr_bm->level == 0 && is_set(curr_bm, vpn_idxs[0])){
                                flip_bit(curr_bm, vpn_idxs[0]);
                            }
                            destroy_bitmap(curr_bm->children[vpn_idxs[j-1]]);
                            virtual_bm->last_free++;
                            flip_bit(virtual_bm, virtual_bm->last_free);
                            curr_bm->children[vpn_idxs[j-1]] = NULL;
                            (ram + curr_bm->phys_idx)[vpn_idxs[j-1]] = 0;
                        }
                    }
                }
                break;
            }
            if(curr_bm->free_page == -1 && curr_bm->level > 0){
                flip_bit(curr_bm, vpn_idxs[i]);
            }
            prev_table = curr_table;
            curr_table = (pde_t*)curr_table[vpn_idxs[i]];
            curr_bm = curr_bm->children[vpn_idxs[i]]; 
        }
        free(vpn_idxs);
        if(p < pages-1){
            curr_vpi++;
            _va = vpi_to_va(curr_vpi);
            phys = translate(page_dir, (void*)_va);
        }
        
        
    }
    free(sizes);
    pthread_mutex_unlock(&lock_free);
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
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
=======
    pthread_mutex_lock(&lock_put);
    if(size < va_info->entry_size){
        perror("invalid size argument");
        pthread_mutex_unlock(&lock_put);
        return -1;
    }
    if(size > va_info->entry_size){
        unsigned long pages = ceil((double)size/(double)PGSIZE);
        unsigned long* sizes = (unsigned long*)Malloc(pages*sizeof(unsigned long));
        unsigned long rem = size % PGSIZE;
        for(int n = 0; n < pages; n++){
            if(n == pages-1){
                if(rem == 0){
                    sizes[n] = PGSIZE;
                }
                else{
                    sizes[n] = rem;
                }
            }
            else{
                sizes[n] = PGSIZE;
            }
        }

        unsigned long curr_vpi = va_to_vpi((unsigned long)va);
        pte_t* phys = translate(page_dir, va);
        if(phys == NULL){
            perror("virtual address does not exist");
            free(sizes);
            pthread_mutex_unlock(&lock_put);
            return -1;
        }
        unsigned long _va = (unsigned long)va;
        unsigned long* src;
        for(unsigned long p = 0; p < pages; p++){    
            src =  (unsigned long*)val + (p*(PGSIZE/va_info->entry_size));
            memcpy((void*)phys, (void*)src, sizes[p]);
            if(p < pages-1){
                curr_vpi++;
                _va = vpi_to_va(curr_vpi);
                phys = translate(page_dir, (void*)_va);
            }
        }
        free(sizes);
    }
    else{
        pte_t* phys = translate(page_dir, va);
        if(phys == NULL){
            pthread_mutex_unlock(&lock_put);
            return -1;
        }
        *phys = *(pte_t*)val;
    }
    pthread_mutex_unlock(&lock_put);
    return 0;
>>>>>>> Stashed changes
    /*return -1 if put_value failed and 0 if put is successfull*/
}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */
<<<<<<< Updated upstream
    // Check for presence of translation in TLB
    pte_t *pte = check_TLB(va);
    if (pte == NULL) {
        // If not present, perform translation
        pte = translate(page_directory, va);
    }

    if (pte == NULL) {
        fprintf(stderr, "get_value(): translation failed for virtual address %p\n", va);
        return;
    }

    // Get physical address and copy value from physical memory
    void *pa = (void *)(*pte & ~0xFFF) + OFFSET_INDEX(va);
    memcpy(val, pa, size);
=======
   pthread_mutex_lock(&lock_get);
   if(size < va_info->entry_size){
        perror("invalid size argument");
        exit(1);
    }
    if(size > va_info->entry_size){
        unsigned long pages = ceil((double)size/(double)PGSIZE);
        unsigned long* sizes = (unsigned long*)Malloc(pages*sizeof(unsigned long));
        unsigned long rem = size % PGSIZE;
        for(int n = 0; n < pages; n++){
            if(n == pages-1){
                if(rem == 0){
                    sizes[n] = PGSIZE;
                }
                else{
                    sizes[n] = rem;
                }
            }
            else{
                sizes[n] = PGSIZE;
            }
        }
        unsigned long curr_vpi = va_to_vpi((unsigned long)va);
        pte_t* phys = translate(page_dir, va);
        if(phys == NULL){
            perror("virtual address does not exist");
            free(sizes);
            exit(1);
        }
        unsigned long _va = (unsigned long)va;
        unsigned long* dest;
        for(unsigned long p = 0; p < pages; p++){
            dest =  (unsigned long*)val + (p*(PGSIZE/va_info->entry_size));
            memcpy((void*)dest, (void*)phys, sizes[p]);
            if(p < pages-1){
                curr_vpi++;
                _va = vpi_to_va(curr_vpi);
                phys = translate(page_dir, (void*)_va);
            }
        }
        free(sizes);
    }
    else{
        pte_t* phys = translate(page_dir, va);
        if(phys == NULL){
            perror("virtual address does not exist");
            exit(1);
        }
        *(pte_t*)val = *phys;
    }
    pthread_mutex_unlock(&lock_get);
>>>>>>> Stashed changes
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
    pthread_mutex_lock(&lock_matmul);
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
                printf("Values at the index: %d, %d, %d, %d, %d\n", 
                    a, b, size, (i * size + k), (k * size + j));
                c += (a * b);
            }
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            printf("This is the c: %d, address: %x!\n", c, address_c);
            put_value((void *)address_c, (void *)&c, sizeof(int));
        }
    }
    pthread_mutex_unlock(&lock_matmul);
}
/*General Virtual Address information functions*/
/*Initialize all necessary information regarding the library environment
 -Calculate bits for VPN and offset
 -Calculate number of levels needed as well as any information for each specific level*/
VA_info_t* init_virtual_address_info(){
    VA_info_t* va_info = (VA_info_t*)Malloc(sizeof(VA_info_t));
    va_info->address_space = (unsigned long)log2(MAX_MEMSIZE);
    va_info->entry_size = va_info->address_space/8;
    va_info->offset = (unsigned long)log2(PGSIZE);
    va_info->vpn_size = va_info->address_space - va_info->offset;
    va_info->vpn_max_val = ((unsigned long)pow(2.0, (double)va_info->vpn_size))-1;
    va_info->max_entries = PGSIZE/va_info->entry_size;
    va_info->max_bits = (unsigned long)log2(va_info->max_entries);
    va_info->levels = (va_info->vpn_size/va_info->max_bits);
    va_info->bits_rem = va_info->vpn_size % va_info->max_bits;
    unsigned long b_bits = 0;
    if(va_info->bits_rem > 0){
        va_info->levels++;
        va_info->level_info = (level_info_t**)Malloc(va_info->levels*sizeof(level_info_t*));
        if(va_info->bits_rem < 5){
            unsigned long vpn_bits_used = va_info->vpn_size/va_info->levels;
            unsigned long vpn_bits_rem = va_info->vpn_size%va_info->levels;
            for(unsigned long i = 0; i < va_info->levels; i++){
                va_info->level_info[i] = (level_info_t*)Malloc(sizeof(level_info_t));
                if(i == 0){
                    b_bits += vpn_bits_used + vpn_bits_rem;
                    va_info->level_info[i]->bits = vpn_bits_used + vpn_bits_rem;
                    va_info->level_info[i]->build_bits = va_info->address_space - b_bits;
                    va_info->level_info[i]->entries = (unsigned long)pow(2.0, (double)vpn_bits_used + vpn_bits_rem);
                    va_info->level_info[i]->bm_size = (va_info->level_info[i]->entries / va_info->address_space)*va_info->entry_size;
                }
                else{
                    b_bits += vpn_bits_used;
                    va_info->level_info[i]->bits = vpn_bits_used;
                    va_info->level_info[i]->build_bits = va_info->address_space - b_bits;
                    va_info->level_info[i]->entries = (unsigned long)pow(2.0, (double)vpn_bits_used);
                    va_info->level_info[i]->bm_size = (va_info->level_info[i]->entries / va_info->address_space)*va_info->entry_size;
                }
            }
        }
        else{
            for(unsigned long i = 0; i < va_info->levels; i++){
                va_info->level_info[i] = (level_info_t*)Malloc(sizeof(level_info_t));
                if(i == va_info->levels-1){
                    b_bits += va_info->bits_rem;
                    va_info->level_info[i]->bits = va_info->bits_rem;
                    va_info->level_info[i]->build_bits = va_info->address_space - b_bits;
                    va_info->level_info[i]->entries = (unsigned long)pow(2.0, (double)va_info->bits_rem);
                    va_info->level_info[i]->bm_size = (va_info->level_info[i]->entries / va_info->address_space)*va_info->entry_size;
                }
                else{
                    b_bits += va_info->max_bits;
                    va_info->level_info[i]->bits = va_info->max_bits;
                    va_info->level_info[i]->build_bits = va_info->address_space - b_bits;
                    va_info->level_info[i]->entries = (unsigned long)pow(2.0, (double)va_info->max_bits);
                    va_info->level_info[i]->bm_size = (va_info->level_info[i]->entries / va_info->address_space)*va_info->entry_size;
                }
            }
        }
        
    }
    else{
        va_info->level_info = (level_info_t**)Malloc(va_info->levels*sizeof(level_info_t*));
        for(unsigned long i = 0; i < va_info->levels; i++){
                b_bits += va_info->max_bits;
                va_info->level_info[i] = (level_info_t*)Malloc(sizeof(level_info_t));
                va_info->level_info[i]->bits = va_info->max_bits;
                va_info->level_info[i]->build_bits = va_info->address_space - b_bits;
                va_info->level_info[i]->entries = (unsigned long)pow(2.0, (double)va_info->max_bits);
                va_info->level_info[i]->bm_size = (va_info->level_info[i]->entries / va_info->address_space)*va_info->entry_size;
        }
    }
    va_info->phys_bm_size = ((MEMSIZE / PGSIZE) / va_info->address_space)*va_info->entry_size;
    return va_info;
}
/*Free memory allocated for virtual address info struct*/
void destroy_virtual_address_info(VA_info_t* va_info){
    for(unsigned long i = 0; i < va_info->levels; i++){
        free(va_info->level_info[i]);
    }
    free(va_info->level_info);
    free(va_info);
}
/*Translation Functions*/
/*Calculate virtual bitmap index from VPN indexes.*/
unsigned long va_to_vpi(unsigned long va){
    unsigned long curr_idx = va >> (va_info->level_info[0]->build_bits) & (va_info->level_info[0]->entries-1);
    unsigned long vpi = curr_idx;
    for(unsigned long i = 1; i < va_info->levels; i++){
        curr_idx = va >> (va_info->level_info[i]->build_bits) & (va_info->level_info[i]->entries-1);
        vpi = (vpi*va_info->level_info[i]->entries)+curr_idx; 
    }
    return vpi;
}
/*Calculate virtual address from virtual bitmap index*/
unsigned long vpi_to_va(unsigned long vpi){
    unsigned long va = 0;
    unsigned long curr_idx_build = vpi % va_info->level_info[va_info->levels-1]->entries;
    unsigned long idx_help = vpi / va_info->level_info[va_info->levels-1]->entries;
    va ^= (curr_idx_build << va_info->level_info[va_info->levels-1]->build_bits);
    for(int j = va_info->levels-2; j >= 0; j--){
        curr_idx_build = idx_help % va_info->level_info[j]->entries;
        idx_help = idx_help / va_info->level_info[j]->entries;
        va ^= (curr_idx_build << va_info->level_info[j]->build_bits);
    }
    return va;
}
/*Display indexes of virtual address from left to right (top level page directory to lowest level page table)*/
void display_va(unsigned long va){
    unsigned long curr_idx;
    printf("VPN idxs: (");
    for(unsigned int i = 0; i < va_info->levels; i++){
        curr_idx = va >> (va_info->level_info[i]->build_bits) & (va_info->level_info[i]->entries-1); //vpn_idxs[i]
        if(i == va_info->levels-1){
            printf("%lu", curr_idx);
        }
        else{
            printf("%lu, ", curr_idx);
        }
    }
    printf(")\n");
}
/*TLB Functions*/
void init_tlb(struct tlb* _tlb){
    _tlb->cache = (tlb_entry_t**)Malloc(TLB_ENTRIES*sizeof(tlb_entry_t*));
    _tlb->valid = init_bitmap((TLB_ENTRIES/8), NULL, false, -1);
}

void destroy_tlb(struct tlb* _tlb){
    for(unsigned long i = 0; i < TLB_ENTRIES; i++){
        if(is_set(_tlb->valid, i)){
            free(_tlb->cache[i]);
        }
    }
    free(_tlb->cache);
    destroy_bitmap(_tlb->valid);
}

/*Bitmap functions*/
bitmap_t* init_bitmap(unsigned int size, bitmap_t* p, bool has_children, int lvl){
    bitmap_t* bm = (bitmap_t*)Malloc(sizeof(bitmap_t));
    bm->level = lvl;
    bm->pages = (size*va_info->address_space)/va_info->entry_size;
    bm->bitmap = (unsigned long*)Malloc(size);
    memset(bm->bitmap, 0, size);
    bm->free_page = 0;
    bm->parent = p;
    if(has_children){
       bm->children = (bitmap_t**)calloc(bm->pages, sizeof(bitmap_t*)); 
    }
    else{
        bm->children = NULL;
    }
    bm->last_free = bm->pages-1;
    bm->num_free = bm->pages;
    bm->phys_idx = -1;
    return bm;
}

void destroy_bitmap(bitmap_t* bm){
    if(bm->children != NULL){
        for(unsigned long i = 0; i < bm->pages; i++){
            if(bm->children[i] != NULL){
                free(bm->children[i]);
            }
        }
        free(bm->children);
    }
    free(bm->bitmap);
    free(bm);
}
/*Display bitmap (if clip == 1 display only the first 10 rows of the bitmap, if clip == 2 display only the last 10 rows of the bitmap)*/
void display_bitmap(bitmap_t* bm, int clip){
    printf("|-->bit: |");
    for(int k = 0; k < bm->pages/(bm->pages/va_info->address_space); k++){
        if(k < 10){
            printf("0%i|", k);
        }
        else{
            printf("%i|", k);
        }
        
    }
    printf("\n");
    for(int l = 0; l < bm->pages/va_info->address_space; l++){ 
        if(clip == 1){
            if(l > 10){
                break;
            }
        }
        if(clip == 2){
            if(l < (bm->pages/va_info->address_space)-10){
                continue;
            }
        }
        
        if(l < 10){
            printf("byte 0%i: |", l);
        }
        else{
            printf("byte %i: |", l);
        }
        
        for(int m = 0; m < bm->pages/(bm->pages/va_info->address_space); m++){
            if(is_set(bm, (l*(bm->pages/(bm->pages/va_info->address_space)))+m)){
                printf("1 |");
            }
            else{
                printf("0 |");
            }
        }
        printf("\n");
    }
    printf("\n");
}

/*Check if a given bit is set in a given bitmap.*/
bool is_set(bitmap_t* bm, unsigned long page_num){
    if(page_num < 0 || page_num > bm->pages-1){
        perror("is_set(): page_num out of range");
        return false;
    }
    unsigned long byte = page_num / va_info->address_space;
    unsigned long bit = page_num % va_info->address_space;
    unsigned long mask = (0x80000000>>bit);
    return bm->bitmap[byte] & mask;
}
/*Set a given bit in a given bitmap.*/
void flip_bit(bitmap_t* bm, unsigned long page_num){
    if(page_num < 0 || page_num > bm->pages-1){
        perror("flip_bit(): page_num out of range");
        exit(1);
    }
    unsigned long byte = page_num / va_info->address_space;
    unsigned long bit = page_num % va_info->address_space;
    unsigned long mask = (0x80000000>>bit);
    bm->bitmap[byte] ^= mask;
    if(is_set(bm, page_num)){
        bm->num_free--;
    }
    else{
        bm->num_free++;
    }
    find_first_free(bm);
}
/*Reset bits in an virtual address to 0's from start index to end index*/
void clear_bits(unsigned long* va, unsigned long start, unsigned long end){
    for(unsigned long j = start; j < end; j++){
        *va &= ~(1 << j);
    }
}
/*Returns true if a given bitmap does not point to any child bitmaps */
bool is_free_complete(bitmap_t* bm){
    for(unsigned long i = 0; i < bm->pages; i++){
        if(bm->children[i] != NULL){
            return false;
        }
    }
    return true;
}
/*Check if a bitmap has enough free bits for given number of pages*/
bool has_space(bitmap_t* bm, unsigned long num_pages){
    unsigned long count = num_pages;
    for(unsigned long i = 0; i < (bm->pages/va_info->address_space); i++){
        if(~(bm->bitmap[i]) != 0){
            for(unsigned long j = 0; j < va_info->address_space; j++){
                if(!is_set(bm, (i*va_info->address_space)+j)){
                    count--;
                    if(count < 1){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
/*Function for checking if virtual memory has enough contiguous space for requested number of pages*/
bool find_first_contig(bitmap_t* bm, unsigned long num_pages, unsigned long* va){
    for(unsigned long i = 0; i < (bm->pages/va_info->address_space); i++){
        if(~(bm->bitmap[i]) != 0){
            for(unsigned long j = 0; j < va_info->address_space; j++){
                if(!is_set(bm, (i*va_info->address_space)+j)){
                    bool flag = true;
                    for(unsigned long k = 1; k < num_pages; k++){
                        if(((i*va_info->address_space)+(j+k)) >= bm->pages){
                            va = NULL;
                            return false;
                        }
                        if(is_set(bm, (i*va_info->address_space)+(j+k))){
                            flag = false;
                            break;
                        }
                        else{
                        }
                    }
                    if(flag){
                        unsigned long virt = vpi_to_va(((i*va_info->address_space)+j));
                        *va = virt;
                        return true;
                    }
                }
            }
        }
    }
    va = NULL;
    return false;
}

/*Finds first free bit in given bitmap. If no bits are free return -1.*/
void find_first_free(bitmap_t* bm){
    for(unsigned long i = 0; i < (bm->pages/va_info->address_space); i++){
        if(~(bm->bitmap[i]) != 0){
            for(unsigned long j = 0; j < va_info->address_space; j++){
                if(!is_set(bm, (i*va_info->address_space)+j)){
                    bm->free_page = (i*va_info->address_space)+j;
                    return;
                }
            }
        }
    }
    bm->free_page = -1;
    bm->num_free = 0;
}
/*Returns true if a free PFN is available, saves virtual address corresponding to free PFN in va.
 *Returns false if no free PFN is found(there still could be free space in physical mem at this point, but a new page table must be allocated).
 *If any level page table is full, flip the corresponding bit in it's parent's bitmap.*/
bool find_first_free_rec(bitmap_t* bm, unsigned long* va, unsigned long track){
    if(bm->children == NULL){
        if(bm->free_page == -1){
            flip_bit(bm->parent, track);
            return false;
        }
        else{
            *va ^= (bm->free_page << va_info->level_info[bm->level]->build_bits); 
            return true;
        }
    }
    for(unsigned long i = 0; i < bm->pages; i++){
        if(bm->children[i] != NULL && find_first_free_rec(bm->children[i], va, i)){ 
            if(bm->level == 0){
                clear_bits(va, va_info->level_info[bm->level]->build_bits, va_info->address_space);
            }
            else{
                clear_bits(va, va_info->level_info[bm->level]->build_bits, va_info->level_info[bm->level-1]->build_bits);
            }
            *va ^= (i << va_info->level_info[bm->level]->build_bits); 
            return true;  
        }
    }
    if(bm->free_page == -1 && bm->parent != NULL){
        flip_bit(bm->parent, track);
    }
    return false;
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
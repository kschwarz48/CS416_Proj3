#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 4096
#define PAGE_TABLE_SIZE 1024
#define MAX_CHILDREN 256

typedef struct page_table_entry {
    unsigned int present: 1;
    unsigned int read_write: 1;
    unsigned int user_supervisor: 1;
    unsigned int page_frame: 29;
} page_table_entry;

typedef struct radix_node {
    page_table_entry *entry;
    struct radix_node *children[MAX_CHILDREN];
} radix_node;

radix_node *create_radix_node() {
    radix_node *node = (radix_node*) malloc(sizeof(radix_node));
    node->entry = NULL;
    memset(node->children, 0, sizeof(node->children));
    return node;
}

void insert_page_table_entry(radix_node *root, unsigned int virtual_address, page_table_entry entry) {
    radix_node *curr = root;
    unsigned int level2_idx = (virtual_address >> 22) & 0x3FF;
    unsigned int level1_idx = (virtual_address >> 12) & 0x3FF;
    unsigned int level0_idx = virtual_address & 0xFFF;
    if (curr->children[level2_idx] == NULL) {
        curr->children[level2_idx] = create_radix_node();
    }
    curr = curr->children[level2_idx];
    if (curr->children[level1_idx] == NULL) {
        curr->children[level1_idx] = create_radix_node();
    }
    curr = curr->children[level1_idx];
    if (curr->entry == NULL) {
        curr->entry = (page_table_entry*) malloc(sizeof(page_table_entry) * PAGE_TABLE_SIZE);
        memset(curr->entry, 0, sizeof(page_table_entry) * PAGE_TABLE_SIZE);
    }
    curr->entry[level0_idx] = entry;
}

page_table_entry *search_page_table_entry(radix_node *root, unsigned int virtual_address) {
    radix_node *curr = root;
    unsigned int level2_idx = (virtual_address >> 22) & 0x3FF;
    unsigned int level1_idx = (virtual_address >> 12) & 0x3FF;
    unsigned int level0_idx = virtual_address & 0xFFF;
    if (curr->children[level2_idx] == NULL) {
        return NULL;
    }
    curr = curr->children[level2_idx];
    if (curr->children[level1_idx] == NULL) {
        return NULL;
    }
    curr = curr->children[level1_idx];
    if (curr->entry == NULL) {
        return NULL;
    }
    return &curr->entry[level0_idx];
}

void free_radix_node(radix_node *node) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < MAX_CHILDREN; ++i) {
        free_radix_node(node->children[i]);
    }
    if (node->entry != NULL) {
        free(node->entry);
    }
    free(node);
}

int main() {
    radix_node *root = create_radix_node();

    // Map virtual address 0x00001000 to physical address 0x1000 with read/write access for user mode
    page_table_entry entry;
    entry.present = 1;
    entry.read_write = 1;
    entry.user_supervisor = 1;
    entry.page_frame = 0x1; // Physical address 0x1000
    insert_page_table_entry(root, 0x00001000, entry);

    // Map virtual address 0x00100000 to physical address 0x2000 with read-only access for user mode
    entry.present = 1;
    entry.read_write = 0;
    entry.user_supervisor = 1;
    entry.page_frame = 0x2; // Physical address 0x2000
    insert_page_table_entry(root, 0x00100000, entry);

    // Access virtual address 0x00001000 (should return a non-null pointer to the page table entry)
    page_table_entry *pte = search_page_table_entry(root, 0x00001000);
    if (pte != NULL) {
        printf("Virtual address 0x00001000 maps to physical address 0x%x\n", pte->page_frame * PAGE_SIZE);
        printf("Read/write access for user mode: %s\n", pte->read_write ? "yes" : "no");
    } else {
        printf("Virtual address 0x00001000 does not have a page table entry\n");
    }

    // Access virtual address 0x00100000 (should return a non-null pointer to the page table entry)
    pte = search_page_table_entry(root, 0x00100000);
    if (pte != NULL) {
        printf("Virtual address 0x00100000 maps to physical address 0x%x\n", pte->page_frame * PAGE_SIZE);
        printf("Read/write access for user mode: %s\n", pte->read_write ? "yes" : "no");
    } else {
        printf("Virtual address 0x00100000 does not have a page table entry\n");
    }

    // Free the radix tree
    free_radix_node(root);

    return 0;
}


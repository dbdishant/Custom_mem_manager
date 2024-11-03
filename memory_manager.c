#include <stdlib.h>
#include <string.h>
#include "memory_manager.h"

static STRU_MEM_LIST *mem_pool = NULL;

/*
 * Print out the current status of the memory manager.
 * Reading this function may help you understand how the memory manager organizes the memory.
 */

void mem_mngr_print_snapshot(void)
{
    STRU_MEM_LIST *mem_list = NULL;

    printf("============== Memory snapshot ===============\n");

    mem_list = mem_pool; // Get the first memory list
    while (NULL != mem_list)
    {
        STRU_MEM_BATCH *mem_batch = mem_list->first_batch; // Get the first mem batch from the list

        printf("mem_list %p slot_size %d batch_count %d free_slot_bitmap %p\n",
               mem_list, mem_list->slot_size, mem_list->batch_count, mem_list->free_slots_bitmap);
        bitmap_print_bitmap(mem_list->free_slots_bitmap, mem_list->bitmap_size);

        while (NULL != mem_batch)
        {
            printf("\t mem_batch %p batch_mem %p\n", mem_batch, mem_batch->batch_mem);
            mem_batch = mem_batch->next_batch; // get next mem batch
        }

        mem_list = mem_list->next_list;
    }

    printf("==============================================\n");
}

/*
 * Initialize the memory manager with 16 bytes(defined by the macro MEM_ALIGNMENT_BOUNDARY) slot size mem_list.
 * Initialize this list with 1 batch of slots.
 */

void mem_mngr_init(void)
{
    // Allocate memory for the main memory list structure
    mem_pool = (STRU_MEM_LIST *)malloc(sizeof(STRU_MEM_LIST));
    if (!mem_pool)
    {
        printf("ERROR: Memory allocation failed for memory pool.\n");
        return;
    }

    // Initialize list structure fields
    mem_pool->slot_size = MEM_ALIGNMENT_BOUNDARY;
    mem_pool->batch_count = 1;
    mem_pool->bitmap_size = (MEM_BATCH_SLOT_COUNT + 7) / 8;

    // Allocate memory for the free slots bitmap
    mem_pool->free_slots_bitmap = (unsigned char *)malloc(mem_pool->bitmap_size);
    if (!mem_pool->free_slots_bitmap)
    {
        printf("ERROR: Memory allocation failed for free slots bitmap.\n");
        goto cleanup_mem_pool;
    }
    memset(mem_pool->free_slots_bitmap, 0xFF, mem_pool->bitmap_size); // Mark all slots as free

    // Allocate memory for the first batch structure
    mem_pool->first_batch = (STRU_MEM_BATCH *)malloc(sizeof(STRU_MEM_BATCH));
    if (!mem_pool->first_batch)
    {
        printf("ERROR: Memory allocation failed for first batch.\n");
        goto cleanup_bitmap;
    }

    // Allocate memory for the actual batch data
    mem_pool->first_batch->batch_mem = malloc(MEM_ALIGNMENT_BOUNDARY * MEM_BATCH_SLOT_COUNT);
    if (!mem_pool->first_batch->batch_mem)
    {
        printf("ERROR: Memory allocation failed for batch memory.\n");
        goto cleanup_first_batch;
    }

    // Initialize batch linkage
    mem_pool->first_batch->next_batch = NULL;
    mem_pool->next_list = NULL;
    return;

cleanup_first_batch:
    free(mem_pool->first_batch);

cleanup_bitmap:
    free(mem_pool->free_slots_bitmap);

cleanup_mem_pool:
    free(mem_pool);
    mem_pool = NULL;
}

/*
 * Clean up the memory manager (e.g., release all the memory allocated)
 */

void mem_mngr_leave(void)
{
    STRU_MEM_LIST *list = mem_pool;
    while (list)
    {
        STRU_MEM_BATCH *batch = list->first_batch;
        while (batch)
        {
            // Free the memory allocated for each batch
            free(batch->batch_mem);
            batch->batch_mem = NULL;

            // Move to the next batch and free the current one
            STRU_MEM_BATCH *next_batch = batch->next_batch;
            free(batch);
            batch = next_batch;
        }

        // Free the bitmap associated with the current memory list
        free(list->free_slots_bitmap);
        list->free_slots_bitmap = NULL;

        // Move to the next memory list and free the current one
        STRU_MEM_LIST *next_list = list->next_list;
        free(list);
        list = next_list;
    }

    // Set mem_pool to NULL to indicate memory manager is de-initialized
    mem_pool = NULL;
}

/*
 * Allocate a chunk of memory
 * @param size: size in bytes to be allocated
 * @return: the pointer to the allocated memory slot
 */
size_t align_size(size_t size)
{
    return (size + MEM_ALIGNMENT_BOUNDARY - 1) & ~(MEM_ALIGNMENT_BOUNDARY - 1);
}

void *mem_mngr_alloc(size_t size)
{
    /* You code here */
    size = SLOT_ALLINED_SIZE(size);
    if (size > (5 * MEM_ALIGNMENT_BOUNDARY))
    {
        fprintf(stderr, "SIZE LIMIT ERROR: size greater than 5 times of the defined MEM_ALIGNMENT_BOUNDARY [%d] size\n", MEM_ALIGNMENT_BOUNDARY);
        return NULL;
    }
    STRU_MEM_LIST *current_list = mem_pool;
    while (current_list)
    {
        if (current_list->slot_size == size)
        {
            for (int i = 0; i < current_list->batch_count * MEM_BATCH_SLOT_COUNT; i++)
            {
                if (current_list->free_slots_bitmap[i / 8] & (1 << (i % 8)))
                {
                    current_list->free_slots_bitmap[i / 8] &= ~(1 << (i % 8));
                    STRU_MEM_BATCH *batch = current_list->first_batch;
                    int batch_index = i / MEM_BATCH_SLOT_COUNT;
                    while (batch_index--)
                    {
                        batch = batch->next_batch;
                    }
                    void *slot_addr = (char *)batch->batch_mem + (i % MEM_BATCH_SLOT_COUNT) * current_list->slot_size;
                    return slot_addr;
                }
            }

            // Expand with a new batch if no free slots found
            STRU_MEM_BATCH *new_batch = (STRU_MEM_BATCH *)malloc(sizeof(STRU_MEM_BATCH));
            new_batch->batch_mem = malloc(size * MEM_BATCH_SLOT_COUNT);
            new_batch->next_batch = current_list->first_batch;
            current_list->first_batch = new_batch;
            current_list->batch_count++;

            // Expand the bitmap
            int new_bitmap_size = (current_list->batch_count * MEM_BATCH_SLOT_COUNT + 7) / 8;
            current_list->free_slots_bitmap = (unsigned char *)realloc(current_list->free_slots_bitmap, new_bitmap_size);
            memset(current_list->free_slots_bitmap + current_list->bitmap_size, 0xFF, new_bitmap_size - current_list->bitmap_size);
            current_list->bitmap_size = new_bitmap_size;

            // Allocate the first slot in the new batch
            current_list->free_slots_bitmap[(current_list->batch_count - 1) * MEM_BATCH_SLOT_COUNT / 8] &= ~(1 << 0);
            return new_batch->batch_mem;
        }
        current_list = current_list->next_list;
    }

    STRU_MEM_LIST *new_list = (STRU_MEM_LIST *)malloc(sizeof(STRU_MEM_LIST));
    new_list->slot_size = size;
    new_list->batch_count = 1;
    new_list->bitmap_size = (MEM_BATCH_SLOT_COUNT + 7) / 8;
    new_list->free_slots_bitmap = (unsigned char *)malloc(new_list->bitmap_size);
    memset(new_list->free_slots_bitmap, 0xFF, new_list->bitmap_size);
    new_list->free_slots_bitmap[0] &= ~(1 << 0);

    new_list->first_batch = (STRU_MEM_BATCH *)malloc(sizeof(STRU_MEM_BATCH));
    new_list->first_batch->batch_mem = malloc(size * MEM_BATCH_SLOT_COUNT);
    new_list->first_batch->next_batch = NULL;

    new_list->next_list = mem_pool;
    mem_pool = new_list;

    return new_list->first_batch->batch_mem;
}

/*
 * Free a chunk of memory pointed by ptr
 * Print out any error messages
 * @param: the pointer to the allocated memory slot
 */

void mem_mngr_free(void *ptr)
{
    if (ptr == NULL || mem_pool == NULL)
    {
        printf("ERROR: Invalid pointer or memory manager not initialized\n");
        return;
    }

    // Iterate through all memory lists
    STRU_MEM_LIST *list = mem_pool;
    while (list)
    {
        // Check each batch in the list
        STRU_MEM_BATCH *batch = list->first_batch;
        while (batch)
        {
            // Check if the pointer falls within this batch's memory range
            if ((unsigned char *)ptr >= (unsigned char *)batch->batch_mem &&
                (unsigned char *)ptr < (unsigned char *)batch->batch_mem + list->slot_size * MEM_BATCH_SLOT_COUNT)
            {

                // Calculate the slot position based on the offset
                int slot_pos = ((unsigned char *)ptr - (unsigned char *)batch->batch_mem) / list->slot_size;

                // Check if the pointer is correctly aligned to the start of the slot
                if ((unsigned char *)batch->batch_mem + slot_pos * list->slot_size != ptr)
                {
                    printf("ERROR: Pointer is not aligned with any slot start address.\n");
                    return;
                }

                // Check if the slot is already free
                int byte_idx = slot_pos / 8;
                int bit_idx = slot_pos % 8;
                if (list->free_slots_bitmap[byte_idx] & (1 << bit_idx))
                {
                    printf("ERROR: Double free or invalid free detected.\n");
                    return;
                }

                // Mark the slot as free by setting the bit in the bitmap
                list->free_slots_bitmap[byte_idx] |= (1 << bit_idx);
                return;
            }

            // Move to the next batch in the list
            batch = batch->next_batch;
        }

        // Move to the next list
        list = list->next_list;
    }

    // If we reached here, the pointer does not belong to any memory slot managed by the memory manager
    printf("ERROR: Pointer does not belong to any managed memory slot.\n");
}

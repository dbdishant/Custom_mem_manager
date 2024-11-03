#include "memory_manager.h"
#include "interposition.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    mem_mngr_init();

    // Test (a): Verify Bitmap Updates During Memory Allocation
    printf("Test (a): Verify Bitmap Updates During Memory Allocation\n");
    void *ptrs[8];
    for (int i = 0; i < 8; i++)
    {
        ptrs[i] = mem_mngr_alloc(16); // Allocating 8 blocks of 16 bytes
        printf("Allocated 16 bytes at: %p\n", ptrs[i]);
    }
    // After allocation, the bitmap should show 0000 0000 if all slots are filled.
    // We would need to print the bitmap here if there’s a function for it.

    // Test (b): Handle Allocation Requests Exceeding Current Batch Capacity
    printf("\nTest (b): Handle Allocation Requests Exceeding Current Batch Capacity\n");
    void *extra_ptr = mem_mngr_alloc(16); // This should trigger a new batch allocation
    printf("Allocated extra 16 bytes at (should be in a new batch): %p\n", extra_ptr);

    // Free the memory to reset the state
    for (int i = 0; i < 8; i++)
    {
        mem_mngr_free(ptrs[i]);
    }
    mem_mngr_free(extra_ptr);

    // Test (c): Validate Free Operation for Expected and Unexpected Cases
    printf("\nTest (c): Validate Free Operation for Expected and Unexpected Cases\n");

    // Allocate a block and then free it, observing the bitmap before and after
    void *block1 = mem_mngr_alloc(16);
    printf("Allocated 16 bytes at: %p\n", block1);
    printf("Freeing allocated block (expected case).\n");
    mem_mngr_free(block1); // Should succeed

    // Attempt to free the same block again (double free)
    printf("Attempting double free (should produce an error).\n");
    mem_mngr_free(block1); // Should print error message for double free

    // Attempt to free a pointer that is within a batch but not a block start
    printf("Attempting to free a non-block start address (should produce an error).\n");
    mem_mngr_free((char *)block1 + 1); // Should print error message for invalid free

    // Attempt to free a pointer that is outside of any allocated batch
    printf("Attempting to free an out-of-range pointer (should produce an error).\n");
    void *invalid_ptr = (void *)0xDEADBEEF; // Arbitrary invalid pointer
    mem_mngr_free(invalid_ptr);             // Should print error message for invalid free

    // Test (d): Handle Larger Memory Allocations with New Batches
    printf("\nTest (d): Handle Larger Memory Allocations with New Batches\n");

    // Allocate a block larger than 16 bytes to force a new list/batch creation
    void *large_block = mem_mngr_alloc(32);
    printf("Allocated 32 bytes at: %p\n", large_block);

    // Free the larger block
    mem_mngr_free(large_block);

    mem_mngr_leave();

    return 0;
}

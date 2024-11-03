# Custom Memory Manager

---
### Status
The code contains all necessarry functions as per the problem statement and generates the expected output successfully.

---
### How to Run the Code 

In order to execute the code use below command in the terminal->
```bash
$ make clean
$ make
$ gcc -Wall -Wextra -O2 test_main.c -L. -l:memory_manager.a -o test_memory_manager
$ ./test_memory_manager
```
<em>NOTE -> IF YOU ENCOUNTER PERMISSION DENIED ERROR THEN GIVE BELOW COMMAND</em><br> 
First come out to the base folder
```bash
$ chmod u+w Custom_mem_manager
```

---

### Description of the Code

#### 1. **bitmap.c**

1. **`bitmap_find_first_bit`**  
   * Searches through the bitmap to find the first bit with a specified value (0 or 1).
   * **Parameters:**
     * `bitmap`: Pointer to the start of the bitmap.
     * `size`: Size of the bitmap in bytes.
     * `val`: Value to search for (0 or 1).
   * **Returns:**
     * Position of the first bit that matches the desired value.
     * `BITMAP_OP_NOT_FOUND` if the value is not found.
     * `BITMAP_OP_ERROR` if parameters are invalid.

2. **`bitmap_set_bit`**  
   * Sets a specific bit in the bitmap to 1, often marking a resource as "allocated" or "in use".
   * **Parameters:**
     * `bitmap`: The bitmap to modify.
     * `size`: Size of the bitmap in bytes.
     * `target_pos`: Position of the bit to set.
   * **Returns:**
     * `BITMAP_OP_SUCCEED` on success.
     * `BITMAP_OP_ERROR` if parameters are invalid.

3. **`bitmap_clear_bit`**  
   * Clears a specific bit in the bitmap (sets it to 0), marking the resource as "free".
   * **Parameters:**
     * `bitmap`: The bitmap to modify.
     * `size`: Size of the bitmap in bytes.
     * `target_pos`: Position of the bit to clear.
   * **Returns:**
     * `BITMAP_OP_SUCCEED` on success.
     * `BITMAP_OP_ERROR` if parameters are invalid.

4. **`bitmap_bit_is_set`**  
   * Tests whether a specific bit in the bitmap is set to 1.
   * **Parameters:**
     * `bitmap`: The bitmap to read.
     * `size`: Size of the bitmap in bytes.
     * `pos`: Position of the bit to check.
   * **Returns:**
     * `1` if the bit is set, `0` if not.
     * `BITMAP_OP_ERROR` if parameters are invalid.

5. **`bitmap_print_bitmap`**  
   * Prints the bitmap's contents in binary form, adding a space every 4 bits for readability.
   * **Parameters:**
     * `bitmap`: The bitmap to display.
     * `size`: Size of the bitmap in bytes.
   * **Returns:**
     * `BITMAP_OP_SUCCEED` on success.
     * `BITMAP_OP_ERROR` if the bitmap pointer is NULL.
   * This function primarily aids in debugging, allowing visualization of the allocation status in the bitmap.

#### 2. **memory_manager.c**

1. **`mem_mngr_print_snapshot`**  
   * Prints a snapshot of the current memory manager state for debugging.
   * Iterates through each `STRU_MEM_LIST` and `STRU_MEM_BATCH`, displaying structure details and allocation status.
   * Uses `bitmap_print_bitmap` to show the current allocation status in the bitmap.

2. **`mem_mngr_init`**  
   * Initializes the memory manager with one batch of slots, each of size `MEM_ALIGNMENT_BOUNDARY` (set to 16 bytes).
   * **Functionality:**
     * Allocates a single `STRU_MEM_LIST` and initializes slot size, batch count, and bitmap.
     * Initializes the bitmap to mark all slots as free.
     * Handles memory allocation failures by deallocating previously allocated structures and setting `mem_pool` to NULL.

3. **`mem_mngr_leave`**  
   * Cleans up the memory manager by releasing all allocated memory.
   * **Functionality:**
     * Iterates through each `STRU_MEM_LIST` and `STRU_MEM_BATCH`, freeing memory and setting `mem_pool` to NULL.

4. **`mem_mngr_alloc`**  
   * Allocates a memory slot of the specified size.
   * **Functionality:**
     * Aligns the requested size to a fixed slot size.
     * Searches for a free slot in each list, marking it as occupied in the bitmap if found.
     * Expands the memory manager with a new batch if no free slots are found.
     * Creates a new `STRU_MEM_LIST` if no lists match the requested slot size.

5. **`mem_mngr_free`**  
   * Frees an allocated memory slot, making it available for future allocations.
   * **Functionality:**
     * Validates the pointer, checks if it’s already free, and flags it as free in the bitmap.
     * Prints error messages for invalid, double-free, or non-managed pointers.

---

### Test Cases and Logs

1. **Test Case 1** <br> 
   **Description:** Allocating 4, 5, 1, 2, 15, 6, 10, and 16 bytes to allocate each from 16 slot size.<br>  
   **Output:**<br>
   ```bash
      ============== Memory snapshot ===============
      mem_list 0x55af96cd32a0 slot_size 16 batch_count 1 free_slot_bitmap 0x55af96cd32d0
      bitmap 0x55af96cd32d0 size 1 bytes: 0000  0000  
      mem_batch 0x55af96cd32f0 batch_mem 0x55af96cd3310
      ==============================================
   ```
   **Status:** Working as expected.

3. **Test Case 2**<br> 
   **Description:** Allocate 5 and 9 bytes; slot size <= 16 should allocate the next batch.<br>  
   **Output:**<br>
   ```bash 
      ============== Memory snapshot ===============
      mem_list 0x55af96cd32a0 slot_size 16 batch_count 2 free_slot_bitmap 0x55af96cd32d0
      bitmap 0x55af96cd32d0 size 2 bytes: 0000  0000  0111  1111  
      mem_batch 0x55af96cd37b0 batch_mem 0x55af96cd37d0
      mem_batch 0x55af96cd32f0 batch_mem 0x55af96cd3310
      ==============================================
   ``` 
   **Status:** Working as expected.

4. **Test Case 3**<br>  
   **Description:** Free first block from 16 slot size.<br>  
   **Output:**<br>
   ```bash 
      ============== Memory snapshot ===============
      mem_list 0x55af96cd32a0 slot_size 16 batch_count 2 free_slot_bitmap 0x55af96cd32d0
      bitmap 0x55af96cd32d0 size 2 bytes: 1000  0000  0000  1111  
      mem_batch 0x55af96cd37b0 batch_mem 0x55af96cd37d0
      mem_batch 0x55af96cd32f0 batch_mem 0x55af96cd3310
      ==============================================
   ```  
   **Status:** Working as expected.

5. **Test Case 4**<br>  
   **Description:** Test double free.<br>  
   **Output:**<br>
   ```bash 
      ============== Memory snapshot ===============
      mem_list 0x55af96cd32a0 slot_size 16 batch_count 2 free_slot_bitmap 0x55af96cd32d0
      bitmap 0x55af96cd32d0 size 2 bytes: 1000  0000  0000  1111  
      mem_batch 0x55af96cd37b0 batch_mem 0x55af96cd37d0
      mem_batch 0x55af96cd32f0 batch_mem 0x55af96cd3310
      ==============================================
   ```
   **Status:** Working as expected.

6. **Test Case 5**<br>  
   **Description:** Verify bitmap updates during memory allocation.<br>  
   **Output:**<br>
   ```bash 
      Allocated 16 bytes at: 0x564f88605310
      Allocated 16 bytes at: 0x564f88605320
      Allocated 16 bytes at: 0x564f88605330
      Allocated 16 bytes at: 0x564f88605340
      Allocated 16 bytes at: 0x564f88605350
      Allocated 16 bytes at: 0x564f88605360
      Allocated 16 bytes at: 0x564f88605370
      Allocated 16 bytes at: 0x564f88605380
   ```  
   **Status:** Working as expected.

7. **Test Case 6**<br>  
   **Description:** Handle allocation requests exceeding current batch capacity.<br>  
   **Output:**<br>
   ```bash
      Allocated extra 16 bytes at (should be in a new batch): 0x564f886057d0
      ERROR: Double free or invalid free detected.
   ```  
   **Status:** Working as expected.

8. **Test Case 7**<br>  
   **Description:** Validate free operation for expected and unexpected cases.<br>  
   **Output:**<br>
   ```bash 
      Allocated 16 bytes at: 0x564f886057d0
      Freeing allocated block (expected case).
      Attempting double free (should produce an error).
      ERROR: Double free or invalid free detected.
      Attempting to free a non-block start address (should produce an error).
      ERROR: Pointer is not aligned with any slot start address.
      Attempting to free an out-of-range pointer (should produce an error).
      ERROR: Pointer does not belong to any managed memory slot.
   ```  
   **Status:** Working as expected.

9. **Test Case 8**<br>  
   **Description:** Handle larger memory allocations with new batches.<br>  
   **Output:**<br>
   ```bash
   Allocated 32 bytes at: 0x564f886058d0
   ```
   **Status:** Working as expected.

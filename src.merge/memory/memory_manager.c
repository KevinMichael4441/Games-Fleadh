#include <stdio.h>

#include "memory/memory_manager.h"

MemoryManager GlobalMemoryManager = {0};

//=================================================================
// malloc allocations
//=================================================================
void *MemoryAlloc(size_t size, const char *name)
{
    if (GlobalMemoryManager.count >= MAX_MEMORY_ALLOCATIONS)
    {
#if defined(DEBUG)
        fprintf(stderr, "ERROR : MemoryManager allocation limit reached!\n");
#endif
        return NULL;
    }

    // Attempt to allocate memory
    void *ptr = malloc(size);

#ifdef VALGRIND
    VALGRIND_MALLOCLIKE_BLOCK(ptr, size, 0, 0); // Backtrace Log
#endif

    if (!ptr)
    {
#if defined(DEBUG)
        fprintf(stderr, "ERROR : Memory Manager failed to allocate %zu bytes!\n", size);
#endif
        return NULL;
    }

    // Track the allocation
    GlobalMemoryManager.pointers[GlobalMemoryManager.count] = ptr;
    strncpy(GlobalMemoryManager.pointer_names[GlobalMemoryManager.count], name, MAX_NAME_LENGTH - 1);
    GlobalMemoryManager.pointer_names[GlobalMemoryManager.count][MAX_NAME_LENGTH - 1] = '\0'; // Ensure null termination
    GlobalMemoryManager.count++;

#if defined(DEBUG)
    // Log the allocation
    printf("INFO : Memory Allocated: %p Size: %zu, Name: %s (Caller: %p)\n",
           ptr, size, name, __builtin_return_address(0));
#endif

    // Print Memory Allocations
    MemoryPrintAllocations();

    return ptr;
}

//=================================================================
// Free Memory
//=================================================================
void MemoryFree(void *ptr)
{
    if (!ptr)
    {
        return;
    }

    // Find and remove the pointer from tracking array
    for (int i = 0; i < GlobalMemoryManager.count; i++)
    {
        if (GlobalMemoryManager.pointers[i] == ptr)
        {

#if defined(DEBUG)
            // Log the deallocation before freeing memory
            printf("INFO : Memory Freed: %p Name: %s (Freed by: %p)\n",
                   ptr, GlobalMemoryManager.pointer_names[i], __builtin_return_address(0));
#endif
            free(ptr);
            // Log the deallocation
            // Move the last pointer to this slot (if it's not already the last one)
            GlobalMemoryManager.pointers[i] = GlobalMemoryManager.pointers[--GlobalMemoryManager.count];

            // Print Memory Allocations
            MemoryPrintAllocations();
            return;
        }
    }
// If we get here, we tried to free an untracked pointer
#if defined(DEBUG)
    fprintf(stderr, "ERROR : Memory Manager attempted to free untracked pointer %p!\n", ptr);
#endif
}

//=================================================================
// Cleanup tracked memory
//=================================================================
void MemoryCleanup(void)
{
    // Free all tracked allocations
    for (int i = 0; i < GlobalMemoryManager.count; i++)
    {

#if defined(DEBUG)
        printf("WARNING : Cleaning unfreed memory: %p Name: %s (Freed by: %p)\n", GlobalMemoryManager.pointers[i], GlobalMemoryManager.pointer_names[i], __builtin_return_address(0));
#endif
        free(GlobalMemoryManager.pointers[i]);
    }

    // Reset the counter
    GlobalMemoryManager.count = 0;
}

//=================================================================
// Print all currently allocated memory
//=================================================================
void MemoryPrintAllocations(void)
{
    if (GlobalMemoryManager.count == 0)
    {
#if defined(DEBUG)
        printf("INFO : No active memory allocations.\n");
#endif
        return;
    }

#if defined(DEBUG)
    printf("**********************************************************************************\n");
    printf("************************Current Memory Allocations********************************\n");
    printf("%-5s | %-18s | %-10s\n", "Index", "Pointer", "Name");
    printf("**********************************************************************************\n");

    for (int i = 0; i < GlobalMemoryManager.count; i++)
    {
        printf("%-5d | %-18p | %-10s\n", i, GlobalMemoryManager.pointers[i], GlobalMemoryManager.pointer_names[i]);
    }

    printf("**********************************************************************************\n");
#endif
}

//=================================================================
// Automatically registers cleanup function at program exit
//=================================================================
__attribute__((constructor)) static void InitMemoryManager(void)
{
    atexit(MemoryCleanup);
}

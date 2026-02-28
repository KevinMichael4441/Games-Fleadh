//=================================================================
// Memory Manager
//=================================================================
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdlib.h>
#include <string.h>

#ifdef VALGRIND
#include <valgrind/memcheck.h>
#endif

#include "constants.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// Structure to track memory allocations
	//=================================================================
	typedef struct
	{
		void *pointers[MAX_MEMORY_ALLOCATIONS];						 // Array of allocated memory pointers
		int count;													 // Number of current allocations
		char pointer_names[MAX_MEMORY_ALLOCATIONS][MAX_NAME_LENGTH]; // Name of Object
	} MemoryManager;

	//----------------------------------------------------------------
	// Global instance of the memory manager
	//----------------------------------------------------------------
	extern MemoryManager GlobalMemoryManager;

	//=================================================================
	// Memory Manager Functions
	//=================================================================

	//-----------------------------------------------------------------
	// Allocates memory and tracks it in the memory manager
	//-----------------------------------------------------------------
	void *MemoryAlloc(size_t size, const char *name);

	//-----------------------------------------------------------------
	// Frees memory and removes it from the memory manager tracking
	//-----------------------------------------------------------------
	void MemoryFree(void *ptr);

	//-----------------------------------------------------------------
	// Cleans up any remaining allocations (e.g., at program exit)
	//-----------------------------------------------------------------
	void MemoryCleanup(void);

	//-----------------------------------------------------------------
	// Prints the current memory allocations (for debugging)
	//-----------------------------------------------------------------
	void MemoryPrintAllocations(void);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_MANAGER_H

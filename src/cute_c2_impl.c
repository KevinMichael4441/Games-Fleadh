// Enable cute_c2 implementation (required once in the project)
#define CUTE_C2_IMPLEMENTATION

// --- About warning suppression below ---
// The pragmas below temporarily disable specific GCC/Clang warnings that
// cute_c2 may trigger (like "unused-function" or "switch" warnings).
//
// This allows the Makefile to treat all other warnings as errors safely.
//
// Example Makefile flags:
//   CFLAGS := -std=c11 -Wall -Wextra -Werror -g -DDEBUG $(INCLUDES)
//
// If you prefer to see cute_c2 warnings, simply remove the pragma lines.
// Alternatively, you can relax the compiler flags instead, for example:
//
//   CFLAGS := -std=c11 -Wall -Wextra -g -DDEBUG $(INCLUDES)
// or
//   CFLAGS := -std=c11 -Wall -Wextra -Wno-error=attributes -g -DDEBUG $(INCLUDES)

// COMMENT OUT THE FOLLOWING LINES TO SEE cute_c2 WARNINGS
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "cute_c2.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
// --- End of warning suppression section ---
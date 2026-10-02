# `mem_arena.h` — Header-only Library for Virtual Memory Arenas

## Usage

Macros that enable function definitions (excluding the MAKE_STATIC macros) must only be defined once in one source file. See [here](https://github.com/nothings/stb/blob/f58f558c120e9b32c217290b80bad1a0729fbb2c/docs/stb_howto.txt) for more information.

On Linux/POSIX systems, define _DEFAULT_SOURCE before including the header to ensure functions are properly exposed.

This library relies on GNU C extensions. It is fully compatible with GCC and Clang. To compile on Windows using Visual Studio, you must use the ClangCL toolset.

### Flags

- `MEM_ARENA_IMPLEMENTATION`: enable function definitions
- `MEM_ARENA_DYNAMIC_ARRAY`: enable dynamic array function declarations
- `MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION`: enable dynamic array function definitions
- `MEM_ARENA_GIMME_DYNAMIC_ARRAY`: enable dynamic array function declarations + definitions
- `MEM_ARENA_STRING_BUILDER`: enable string function declarations
- `MEM_ARENA_STRING_BUILDER_IMPLEMENTATION`: enable string function definitions
- `MEM_ARENA_GIMME_STRING_BUILDER`: enable string function declarations + definitions
- `MEM_ARENA_GIMME_ALL`: define all of the above
- `MEM_ARENA_GIMME_ALL_DEC`: include all declarations; no definitions
- `MEM_ARENA_MAKE_STATIC`: make all functions static inline + include definitions
- `MEM_ARENA_GIMME_ALL_MAKE_STATIC`: same as `MEM_ARENA_GIMME_ALL` but all functions are static inline

### Optional Function Parameters

Several function-like macros (`arena_init()`, `arena_alloc()`, `sb_remove()`, etc.) accept optional parameters. You can override the defaults using C99 designated initializers:

```C
arena_init(.reserve_size = MiB(500), .name = "Arena");
```

Consult the parameter structs in the header for a full list of defaults.

### Example Code

```C
#define _DEFAULT_SOURCE
#define MEM_ARENA_GIMME_ALL
#include "mem_arena.h"

#include <stdio.h>

int main(void) {
  mem_arena *arena = arena_init(.name = "Main Arena");
  strb text = build_string(arena, "Hello, World");
  
  sb_append(arena, text, '!');  
  printf("%s\n", text); /* "Hello, World!" */
  
  arena_delete(arena); 
  return 0;
}
```

## License

This software is available under 2 licenses. See [LICENSE](LICENSE) for more information.

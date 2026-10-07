# `dklib`

Collection of header-only libraries à la [stb](https://github.com/nothings/stb) written in C, mainly for personal use.

---

Libraries
- [dk_arena.h](dk_arena.h)

## Usage

To use one of these libraries, you should define its implementation macro and include it in one of your source files:

```c
#define DK_*_IMPLEMENTATION
#include "dk_*.h"
```

Some libraries may require more steps and/or custom compile instructions.

# jvstr: StrAlloc and StrView

**jvstr** is a C library to handle string creation and manipulation, through two structures, `StrAlloc` and `StrView`.
It is licensed under the BSL 1.0.
This library was made to have a C counterpart of C++ `std::string` and `std::string_view`.

`StrView` is a non-owning view of a string, which is not necessary null-terminated. It simply has a pointer and a size.
It has utility functions for spliting, searching, and comparison, each being bound-aware. Its API and documentation is in `<jvstr/StrView.h>`.
Here is an example to split a string and print its parts:

```c
StrView view = STRVIEW_INIT("test, abc, def, ghi");
StrView delimiter = STRVIEW_INIT(", ");
int i = 0;
do {
    StrView part = jvstr_split(&view, jvstr_search(view, delimiter), delimiter.size);
    printf("Part #%d: " STRVIEW_FORMAT "\n", ++i, STRVIEW_ARGS(part));
} while (view.size > 0);
```
Output:
```
Part #1: test
Part #2: abc
Part #3: def
Part #4: ghi
```

`StrAlloc` is a owning string, which is guaranteed to be null-terminated (`str.array[str.size] == '\0'`).
It contains a pointer, a size and a capacity. It has utility functions for insertion and formatting like sprintf.
It is convertible to `StrView` to use the non-mutable API with `STRALLOC_ASVIEW`.
Its API and documentation is in `<jvstr/StrView.h>`. 

## Installation / building

You can compile the tests by using a C++ compiler:
```
g++ jvstr/*.c tests/*.cpp
```
Note: `jvstr` is a C library, but it uses C++ for tests because tests are more readable with C++ frameworks such as `doctest` in my opinion.

To install jvstr, the simplest way is to include the source files in your codebase.
`StrAlloc.h` and `StrView.h` must be located in a directory which is part of the include paths.
Instead, you can modify the `#include` of `StrAlloc.c` and `StrView.c` to accomodate your project's layout.

/*
This is the C implementation for the StrAlloc structure of the jvstr library, written by Julien Vernay ( jvernay.fr ) in 2021.
It contains both the API and the documentation.
jvstr is a library to manipulate strings in the C language.
The library is available under the Boost Software License 1.0, whose terms are below:

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include "StrAlloc.h"

#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#define ENSURES_ALLOC_SUCCESS(alloc_expr) if ((alloc_expr) == NULL) { puts("No more dynamic memory available, exiting the program."); exit(EXIT_FAILURE); }


StrAlloc StrAlloc_make(StrView initial_content) {
    StrAlloc str;
    str.size = initial_content.size;
    str.capacity = (str.size > 16 ? str.size : 16); // at least 16 bytes allocated
    ENSURES_ALLOC_SUCCESS(str.array = (char*)malloc(str.capacity + 1)); // +1 for '\0'
    memcpy(str.array, initial_content.begin, str.size);
    str.array[str.size] = '\0';
    return str;
}

void StrAlloc_destroy(StrAlloc* str) {
    free(str->array);
    memset(str, 0, sizeof(*str));
}

void StrAlloc_reserve(StrAlloc* str, size_t new_capacity) {
    // NOTE: potentially, str->array is NULL (due to {0}-initialization)
    if (str->capacity < new_capacity) {
        char* new_storage;
        ENSURES_ALLOC_SUCCESS(new_storage = (char*)malloc(new_capacity+1));
        if (str->array != NULL) {
            memcpy(new_storage, str->array, str->size);
            free(str->array);
        }
        str->array = new_storage;
        str->array[str->size] = '\0';
        str->capacity = new_capacity;
    }
}

void StrAlloc_append(StrAlloc* str, StrView view) {
    StrAlloc_insert(str, view, str->size);
}

void StrAlloc_insert(StrAlloc* str, StrView view, size_t position) {
    assert((view.begin < str->array || view.begin >= str->array + str->capacity)
           && "'view' must not point into 'str'!");
    StrAlloc_reserve(str, str->size + view.size);
    memcpy(str->array + position + view.size, str->array + position, str->size - position + 1); // copying '\0' too
    memcpy(str->array + position, view.begin, view.size);
    str->size += view.size;
}


StrAlloc jvsprintf(char const* format, ...) {
    va_list args, args_copy;
    va_start(args, format);
    va_copy(args_copy, args);
    size_t size = vsnprintf(NULL, 0, format, args);
    StrAlloc str;
    str.capacity = size;
    ENSURES_ALLOC_SUCCESS(str.array = (char*)malloc(str.capacity + 1));
    vsnprintf(str.array, str.capacity+1, format, args_copy);
    str.size = size;
    va_end(args);
    va_end(args_copy);
    return str;
}

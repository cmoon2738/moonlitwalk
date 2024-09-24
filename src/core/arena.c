#include <moonlitwalk/amw.h>

#include <stdio.h>

#define AMW_ARENA_BACKEND_LIBC_MALLOC 0
#define AMW_ARENA_BACKEND_LINUX_MMAP 1
#define AMW_ARENA_BACKEND_WIN32_VIRTUALALLOC 2
#define AMW_ARENA_BACKEND_WASM_HEAPBASE 3

#ifndef AMW_ARENA_BACKEND
#elif defined(AMW_PLATFORM_LINUX)
    #define AMW_ARENA_BACKEND AMW_ARENA_BACKEND_LINUX_MMAP
#elif defined(AMW_PLATFORM_WINDOWS)
    #define AMW_ARENA_BACKEND AMW_ARENA_BACKEND_WIN32_VIRTUALALLOC
#elif defined(AMW_PLATFORM_EMSCRIPTEN)
    #define AMW_ARENA_BACKEND AMW_ARENA_BACKEND_WASM_HEAPBASE
#else
    #define AMW_ARENA_BACKEND AMW_ARENA_BACKEND_LIBC_MALLOC
#endif /* AMW_ARENA_BACKEND */

#if AMW_ARENA_BACKEND == AMW_ARENA_BACKEND_LIBC_MALLOC
amw_slice_t *amw_slice_new(size_t capacity)
{
    size_t size_bytes = sizeof(amw_slice_t) + sizeof(uintptr_t) * capacity;
    amw_slice_t *slice = (amw_slice_t *)amw_malloc(size_bytes);
    amw_assert(slice);
    slice->next = NULL;
    slice->count = 0;
    slice->capacity = capacity;
    return slice;
}

void amw_slice_free(amw_slice_t *slice)
{
    amw_free(slice);
}

#elif AMW_ARENA_BACKEND == AMW_ARENA_BACKEND_LINUX_MMAP
    #ifndef AMW_PLATFORM_LINUX
        #error "Only linux platform."
    #endif

#include <unistd.h>
#include <sys/mman.h>

amw_slice_t *amw_slice_new(size_t capacity)
{
    size_t size_bytes = sizeof(amw_slice_t) + sizeof(uintptr_t) * capacity;
    amw_slice_t *slice = mmap(NULL, size_bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    amw_assert(slice != MAP_FAILED);
    slice->next = NULL;
    slice->count = 0;
    slice->capacity = capacity;
    return slice;
}

void amw_slice_free(amw_slice_t *slice)
{
    amw_log_warn("w dupee");
    size_t size_bytes = sizeof(amw_slice_t) + sizeof(uintptr_t) * slice->capacity;
    int32_t ret = munmap(slice, size_bytes);
    amw_log_warn("w dupee jebanaaa");
    amw_assert(ret == 0);
}

#elif AMW_ARENA_BACKEND == AMW_ARENA_BACKEND_WIN32_VIRTUALALLOC
    #ifndef AMW_PLATFORM_WINDOWS
        #error "Only windows platform."
    #endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define INV_HANDLE(x)   (((x) == NULL) || ((x) == INVALID_HANDLE_VALUE))

amw_slice_t *amw_slice_new(size_t capacity)
{
    SIZE_T size_bytes = sizeof(amw_slice_t) + sizeof(uintptr_t) * capacity;
    amw_slice_t *slice = VirtualAllocEx(
        GetCurrentProcess(),        /* allocate in current process address space */
        NULL,                       /* unknown position */
        size_bytes,                 /* bytes to allocate */
        MEM_COMMIT | MEM_RESERVE,   /* reserve and commit allocated page */
        PAGE_READWRITE              /* permissions to read/write */
    );
    if (INV_HANDLE(slice))
        amw_assert_release(0 && "VirtualAllocEx() failed allocating memory.");

    slice->next = NULL;
    slice->count = 0;
    slice->capacity = capacity;
    return NULL;
}

void amw_slice_free(amw_slice_t *slice)
{
    if (INV_HANDLE(slice))
        return;

    BOOL free_result = VirtualFreeEx(
        GetCurrentProcess(),        /* deallocate from current process address space */
        (LPVOID)slice,              /* address to deallocate */
        0,                          /* bytes to deallocate (unknown, deallocate entire page) */
        MEM_RELEASE                 /* release the page (and implicitly decommit it) */
    );

    if (FALSE == free_result)
        amw_assert_release(0 && "VirtualFreeEx() failed deallocating memory.");
}

#elif AMW_ARENA_BACKEND == AMW_ARENA_BACKEND_WASM_HEAPBASE
    #error "WebAssembly arena memory pool not implemented."
#else
    #error "Arena backend is unknown."
#endif

void *amw_arena_alloc(amw_arena_t *a, size_t size_bytes)
{
    size_t size = (size_bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
    if (a->end == NULL) {
        amw_assert(a->begin == NULL);
        size_t capacity = AMW_SLICE_DEFAULT_CAPACITY;
        if (capacity < size) 
            capacity = size;
        a->end = amw_slice_new(capacity);
        a->begin = a->end;
    }

    while (a->end->count + size > a->end->capacity && a->end->next != NULL) {
        a->end = a->end->next;
    }

    if (a->end->count + size > a->end->capacity) {
        amw_assert(a->end->next == NULL);
        size_t capacity = AMW_SLICE_DEFAULT_CAPACITY;
        if (capacity < size)
            capacity = size;
        a->end->next = amw_slice_new(capacity);
        a->end = a->end->next;
    }

    void *res = &a->end->data[a->end->count];
    a->end->count += size;
    return res;
}

void *amw_arena_realloc(amw_arena_t *a, void *oldptr, size_t old_size, size_t new_size)
{
    if (new_size <= old_size)
        return oldptr;
    void *newptr = amw_arena_alloc(a, new_size);
    char *newptr_char = (char *)newptr;
    char *oldptr_char = (char *)oldptr;
    for (size_t i = 0; i < old_size; ++i) {
        newptr_char[i] = oldptr_char[i];
    }
    return newptr;
}

char *amw_arena_strdup(amw_arena_t *a, const char *str)
{
    size_t n = strlen(str);
    char *dup = (char *)amw_arena_alloc(a, n + 1);
    amw_memcpy(dup, str, n);
    dup[n] = '\0';
    return dup;
}

void *amw_arena_memdup(amw_arena_t *a, void *data, size_t size_bytes)
{
    return amw_memcpy(amw_arena_alloc(a, size_bytes), data, size_bytes);
}

char *amw_arena_sprintf(amw_arena_t *a, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int32_t n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    amw_assert(n >= 0);
    char *res = (char *)amw_arena_alloc(a, n + 1);
    va_start(args, fmt);
    vsnprintf(res, n + 1, fmt, args);
    va_end(args);
    return res;
}

void amw_arena_reset(amw_arena_t *a)
{
    for (amw_slice_t *slice = a->begin; slice != NULL; slice = slice->next) {
        slice->count = 0;
    }
    a->end = a->begin;
}

void amw_arena_free(amw_arena_t *a)
{
    amw_slice_t *s = a->begin;
    while (s) {
        amw_slice_t *s0 = s;
        s = s->next;
        amw_slice_free(s0);
    }
    a->begin = NULL;
    a->end = NULL;
}

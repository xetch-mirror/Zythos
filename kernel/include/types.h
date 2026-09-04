#ifndef _TYPES_H
#define _TYPES_H

/* integer types */
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

/* X64 type integers */
#if defined(__x86_64__) || defined(__aarch64__)
    typedef uint64_t       size_t;
    typedef int64_t        ssize_t;
    typedef uint64_t       uintptr_t;
#else
    typedef uint32_t       size_t;
    typedef int32_t        ssize_t;
    typedef uint32_t       uintptr_t;
#endif

/* boolean types */
typedef int              bool;
#define true               1
#define false              0

/* Null pointer */
#ifndef NULL
    #define NULL           ((void *)0)
#endif

/* Utility macros */
#define UNUSED(x)          ((void)(x))
#define ARRAY_SIZE(arr)    (sizeof(arr) / sizeof((arr)[0]))

#endif /* _TYPES_H */

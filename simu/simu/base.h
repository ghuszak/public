#if !defined(__BASE_H__)
#define __BASE_H__

#include <string.h>
#include <stdarg.h>

#if defined(__GNUC__)
// __USE_MINGW_ANSI_STDIO: to make %llu build with gcc (32-bit)
// Note: this must be defined before including <stdio.h>
#define __USE_MINGW_ANSI_STDIO 1
#endif // if defined(__GNUC__)
#include <stdio.h>

#define RAND_SEED          0x5cb49f39u

#define ELEMS_OF(v)        (SIZE_OF(v) / SIZE_OF(v[0]))
#define SIZE_OF(s)         sizeof(s)
#define CAST_TO(e, t)      ((t)(e))
#define ADDR_OF(v)         (&(v))

#if (defined(_WIN32) || defined(_WIN64))
#if defined(UNICODE)
#error "UNICODE not supported"
#endif // if defined(UNICODE)

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;
typedef unsigned int       uint;

#if defined(true)
#undef true
#endif // if defined(true)

#if defined(false)
#undef false
#endif // if defined(false)

#define false              (0u == 1u)
#define true               (!false)
#else // platform-check
#include <stdint.h>

typedef uint8_t            uint8;
typedef uint16_t           uint16;
typedef uint32_t           uint32;
typedef uint64_t           uint64;
typedef unsigned int       uint;
#endif // else platform-check

// If the type/size of clock changes, 
// l_print() may also need revision
typedef uint64             clock;
typedef uint               sym5b;
typedef uint               sym4b;

#endif // if !defined(__BASE_H__)

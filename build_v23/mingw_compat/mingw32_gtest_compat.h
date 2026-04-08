/* MinGW GCC 6.3.0 compatibility shim for GTest release-1.10.0.
 * Force-included via -include compiler flag for gtest/gtest_main targets. */
#ifndef MINGW32_GTEST_COMPAT_H_
#define MINGW32_GTEST_COMPAT_H_

/* Pull in windows.h + CRT headers first so macros like MAX_PATH are defined */
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#include <strings.h>   /* strcasecmp */

/* _MAX_PATH: MinGW stdlib.h maps it to MAX_PATH; ensure it's defined */
#ifndef _MAX_PATH
#  ifndef MAX_PATH
#    define MAX_PATH 260
#  endif
#  define _MAX_PATH MAX_PATH
#endif

/* String comparison — MinGW32 may not expose the underscore versions in C++ */
#ifndef _stricmp
#  define _stricmp strcasecmp
#endif
#ifndef _strncasecmp
#  define _strncasecmp strncasecmp
#endif

/* _strdup: POSIX strdup may be hidden in MinGW C++17 mode; implement directly */
#ifndef _strdup
#  ifdef __cplusplus
     static inline char* _strdup(const char* s) {
         size_t n = ::strlen(s) + 1;
         char* p = static_cast<char*>(::malloc(n));
         if (p) ::memcpy(p, s, n);
         return p;
     }
#  else
#    define _strdup(s) ((char*)memcpy(malloc(strlen(s)+1),(s),strlen(s)+1))
#  endif
#endif

/* _fileno: forward-declare so it is always visible in C++ mode */
#ifndef _fileno
#  ifdef __cplusplus
     extern "C" int fileno(FILE*);
#  endif
#  define _fileno fileno
#endif

/* fdopen: POSIX; MinGW has it in <stdio.h> under C mode but it may be hidden
 * in C++ mode.  Forward-declare it to ensure it is visible. */
#ifdef __cplusplus
extern "C" FILE* fdopen(int, const char*);
#endif

/* _wcsicmp: wide-char case-insensitive compare.
 * Old MinGW may not have wcscasecmp; provide a simple fallback. */
#ifndef _wcsicmp
#  ifdef wcscasecmp
#    define _wcsicmp wcscasecmp
#  else
#    include <wctype.h>
     static inline int _wcsicmp(const wchar_t* a, const wchar_t* b) {
         while (*a && *b && towlower(*a) == towlower(*b)) { ++a; ++b; }
         return (int)towlower(*a) - (int)towlower(*b);
     }
#  endif
#endif

/* _exit: POSIX; available in <process.h>; forward-declare as safety net */
#ifndef _exit
#  ifdef __cplusplus
     extern "C" void _exit(int);
#  else
     extern void _exit(int);
#  endif
#endif

/* MSVC-only CRT debug API — stub out; never called in normal test runs */
#define _OUT_TO_STDERR          2
#define _set_error_mode(x)      ((void)(x))
#define _CRT_ASSERT             2
#define _CRTDBG_MODE_FILE       1
#define _CRTDBG_MODE_DEBUG      4
#define _CrtSetReportMode(t,m)  0
#define _CRTDBG_FILE_STDERR     ((void*)2)
#define _CrtSetReportFile(t,f)  NULL

#endif /* MINGW32_GTEST_COMPAT_H_ */

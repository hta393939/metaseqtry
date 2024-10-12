#pragma once

#ifdef WIN32

#ifdef MLIBS_STATIC_LIB
#define MLIBS_API
#elif defined(MLIBS_EXPORTS)
#define MLIBS_API __declspec(dllexport)
#else
#define MLIBS_API __declspec(dllimport)
#endif

typedef unsigned __int64 __uint64;

#endif // WIN32

#if __APPLE__

#include <cstdint>
#include <cstring>
#include <string>
#include <errno.h>
#include <sys/_types/_errno_t.h>

#define MLIBS_API

#ifndef OBJC_BOOL_DEFINED
typedef char BOOL;
#endif
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t LONG; // Compatible with Windows
typedef int64_t __int64;
typedef uint64_t __uint64;
typedef short SHORT;
typedef unsigned short USHORT;
typedef int INT;
typedef unsigned int UINT;
typedef int32_t INT32;
typedef uint32_t UINT32;
typedef int64_t INT64;
typedef uint64_t UINT64;
typedef float FLOAT;
typedef void VOID, *LPVOID;
typedef const void *LPCVOID;
typedef BYTE *LPBYTE;
typedef WORD *LPWORD;
typedef DWORD *LPDWORD;
typedef unsigned long ULONG;
typedef char CHAR, *LPSTR;
typedef const char *LPCSTR;
typedef wchar_t *LPWSTR;
typedef const wchar_t *LPCWSTR;

errno_t strcpy_s(char *dst, size_t num, const char *src);
template<size_t size> inline errno_t strcpy_s(char (&dst)[size], const char *src) { return strcpy_s(dst, size, src); }
errno_t memcpy_s(void *dst, size_t nelems, const void *src, size_t n);
#define _stricmp strcasecmp
#define sscanf_s sscanf

int _wtoi(const wchar_t *s);
double _wtof(const wchar_t *s);
int64_t _atoi64(const char *s);
int64_t _wtoi64(const wchar_t *s);
int64_t _strtoi64(const char *s, size_t *idx, int base);
uint64_t _strtoui64(const char *s, size_t *idx, int base);
int64_t _wcstoi64(const wchar_t *s, size_t *idx, int base);
int64_t _wcstoui64(const wchar_t *s, size_t *idx, int base);

errno_t fopen_s(FILE** fh, const char *filename, const char *mode);
errno_t _wfopen_s(FILE** fh, const wchar_t *filename, const wchar_t *mode);
#define _ftelli64 ftell
#define _fseeki64 fseek

BOOL IsDBCSLeadByte(BYTE ch);

#endif // __APPLE__

#if defined(_DEBUG) || defined(DEBUG)
#include <assert.h>
#define MLIBS_ASSERT(_x) assert(_x)
#else
#define MLIBS_ASSERT(_x)
#endif

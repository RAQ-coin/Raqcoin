/* This is the build config file.
 *
 * With this you can setup what to inlcude/exclude automatically during any build.  Just comment
 * out the line that #define's the word for the thing you want to remove.  phew!
 */

#ifndef RAQCOIN_PQCRYPT_CFG_H
#define RAQCOIN_PQCRYPT_CFG_H


#if defined(_WIN32) || defined(_MSC_VER)
   #define PQC_CALL __cdecl
#elif !defined(PQC_CALL)
   #define PQC_CALL
#endif

#ifndef PQC_EXPORT
   #define PQC_EXPORT
#endif

/* certain platforms use macros for these, making the prototypes broken */
#ifndef PQC_NO_PROTOTYPES

/* you can change how memory allocation works ... */
PQC_EXPORT void * PQC_CALL XMALLOC(size_t n);
PQC_EXPORT void * PQC_CALL XREALLOC(void *p, size_t n);
PQC_EXPORT void * PQC_CALL XCALLOC(size_t n, size_t s);
PQC_EXPORT void PQC_CALL XFREE(void *p);

PQC_EXPORT void PQC_CALL XQSORT(void *base, size_t nmemb, size_t size, int(*compar)(const void *, const void *));


/* change the clock function too */
PQC_EXPORT clock_t PQC_CALL XCLOCK(void);

/* various other functions */
PQC_EXPORT void * PQC_CALL XMEMCPY(void *dest, const void *src, size_t n);
PQC_EXPORT int   PQC_CALL XMEMCMP(const void *s1, const void *s2, size_t n);
PQC_EXPORT void * PQC_CALL XMEMSET(void *s, int c, size_t n);

PQC_EXPORT int   PQC_CALL XSTRCMP(const char *s1, const char *s2);

#endif

/* some compilers do not like "inline" */
#if defined(__HP_cc)
   #define PQC_INLINE
#elif defined(_MSC_VER)
   #define PQC_INLINE __inline
#else
   #define PQC_INLINE inline
#endif

/* type of argument checking, 0=default, 1=fatal and 2=error+continue, 3=nothing */
#ifndef ARGTYPE
   #define ARGTYPE  2
#endif

/* Controls endianess and size of registers.  Leave uncommented to get platform neutral [slower] code
 *
 * Note: in order to use the optimized macros your platform must support unaligned 32 and 64 bit read/writes.
 * The x86 platforms allow this but some others [ARM for instance] do not.  On those platforms you **MUST**
 * use the portable [slower] macros.
 */
/* detect x86/i386 32bit */
#if defined(__i386__) || defined(__i386) || defined(_M_IX86)
   #define ENDIAN_LITTLE
   #define ENDIAN_32BITWORD
   #define PQC_FAST
#endif

/* detect amd64/x64 */
#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
   #define ENDIAN_LITTLE
   #define ENDIAN_64BITWORD
   #define PQC_FAST
#endif

/* detect PPC32 */
#if defined(PQC_PPC32)
   #define ENDIAN_BIG
   #define ENDIAN_32BITWORD
   #define PQC_FAST
#endif

/* detects MIPS R5900 processors (PS2) */
#if (defined(__R5900) || defined(R5900) || defined(__R5900__)) && (defined(_mips) || defined(__mips__) || defined(mips))
   #define ENDIAN_64BITWORD
   #if defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__)
     #define ENDIAN_BIG
   #endif
     #define ENDIAN_LITTLE
   #endif
#endif

/* detect AIX */
#if defined(_AIX) && defined(_BIG_ENDIAN)
  #define ENDIAN_BIG
  #if defined(__LP64__) || defined(_ARCH_PPC64)
    #define ENDIAN_64BITWORD
  #else
    #define ENDIAN_32BITWORD
  #endif
#endif

/* detect HP-UX */
#if defined(__hpux) || defined(__hpux__)
  #define ENDIAN_BIG
  #if defined(__ia64) || defined(__ia64__) || defined(__LP64__)
    #define ENDIAN_64BITWORD
  #else
    #define ENDIAN_32BITWORD
  #endif
#endif

/* detect Apple OS X */
#if defined(__APPLE__) && defined(__MACH__)
  #if defined(__LITTLE_ENDIAN__) || defined(__x86_64__)
    #define ENDIAN_LITTLE
  #else
    #define ENDIAN_BIG
  #endif
  #if defined(__LP64__) || defined(__x86_64__)
    #define ENDIAN_64BITWORD
  #else
    #define ENDIAN_32BITWORD
  #endif
#endif

/* detect SPARC and SPARC64 */
#if defined(__sparc__) || defined(__sparc)
  #define ENDIAN_BIG
  #if defined(__arch64__) || defined(__sparcv9) || defined(__sparc_v9__)
    #define ENDIAN_64BITWORD
  #else
    #define ENDIAN_32BITWORD
  #endif
#endif

/* detect IBM S390(x) */
#if defined(__s390x__) || defined(__s390__)
  #define ENDIAN_BIG
  #if defined(__s390x__)
    #define ENDIAN_64BITWORD
  #else
    #define ENDIAN_32BITWORD
  #endif
#endif

/* detect PPC64 */
#if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__)
   #define ENDIAN_64BITWORD
   #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      #define ENDIAN_BIG
   #elif  __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      #define ENDIAN_LITTLE
   #endif
   #define PQC_FAST
#endif

/* endianness fallback */
#if !defined(ENDIAN_BIG) && !defined(ENDIAN_LITTLE)
  #if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
      defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ || \
      defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN) || \
      defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || \
      defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__)
    #define ENDIAN_BIG
  #elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
      defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || \
      defined(__LITTLE_ENDIAN__) || defined(_LITTLE_ENDIAN) || \
      defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) || \
      defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
    #define ENDIAN_LITTLE
  #else
    #error Cannot detect endianness
  #endif
#endif

/* ulong64: 64-bit data type */
#ifdef _MSC_VER
   #define CONST64(n) n ## ui64
   typedef unsigned __int64 ulong64;
#else
   #define CONST64(n) n ## ULL
   typedef unsigned long long ulong64;
#endif

/* ulong32: "32-bit at least" data type */
#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64) || \
    defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || \
    defined(__s390x__) || defined(__arch64__) || defined(__aarch64__) || \
    defined(__sparcv9) || defined(__sparc_v9__) || defined(__sparc64__) || \
    defined(__ia64) || defined(__ia64__) || defined(__itanium__) || defined(_M_IA64) || \
    defined(__LP64__) || defined(_LP64) || defined(__64BIT__)
   typedef unsigned ulong32;
   #if !defined(ENDIAN_64BITWORD) && !defined(ENDIAN_32BITWORD)
     #define ENDIAN_64BITWORD
   #endif
#else
   typedef unsigned long ulong32;
   #if !defined(ENDIAN_64BITWORD) && !defined(ENDIAN_32BITWORD)
     #define ENDIAN_32BITWORD
   #endif
#endif

/* No PQC_FAST if: explicitly disabled OR non-gcc/non-clang compiler OR old gcc OR using -ansi -std=c99 */
#if defined(PQC_NO_FAST) || (__GNUC__ < 4) || defined(__STRICT_ANSI__)
   #undef PQC_FAST
#endif

#ifdef PQC_FAST
   #define PQC_FAST_TYPE_PTR_CAST(x) ((PQC_FAST_TYPE*)(void*)(x))
   #ifdef ENDIAN_64BITWORD
   typedef ulong64 __attribute__((__may_alias__)) PQC_FAST_TYPE;
   #else
   typedef ulong32 __attribute__((__may_alias__)) PQC_FAST_TYPE;
   #endif
#endif

#ifdef ENDIAN_64BITWORD
typedef ulong64 pqc_mp_digit;
#else
typedef ulong32 pqc_mp_digit;
#endif

/* No asm is a quick way to disable anything "not portable" */
#ifdef PQC_NO_ASM
   #define ENDIAN_NEUTRAL
   #undef ENDIAN_32BITWORD
   #undef ENDIAN_64BITWORD
   #undef PQC_FAST
   #undef PQC_FAST_TYPE
   #define PQC_NO_ROLC
   #define PQC_NO_BSWAP
#endif

#if !defined(ENDIAN_NEUTRAL) && (defined(ENDIAN_BIG) || defined(ENDIAN_LITTLE)) && !(defined(ENDIAN_32BITWORD) || defined(ENDIAN_64BITWORD))
    #error You must specify a word size as well as endianess in pqccrypt_cfg.h
#endif

#if !(defined(ENDIAN_BIG) || defined(ENDIAN_LITTLE))
   #define ENDIAN_NEUTRAL
#endif

#if (defined(ENDIAN_32BITWORD) && defined(ENDIAN_64BITWORD))
    #error Cannot be 32 and 64 bit words...
#endif

/* gcc 4.3 and up has a bswap builtin; detect it by gcc version.
 * clang also supports the bswap builtin, and although clang pretends
 * to be gcc (macro-wise, anyway), clang pretends to be a version
 * prior to gcc 4.3, so we can't detect bswap that way.  Instead,
 * clang has a __has_builtin mechanism that can be used to check
 * for builtins:
 * http://clang.llvm.org/docs/LanguageExtensions.html#feature_check */
#ifndef __has_builtin
   #define __has_builtin(x) 0
#endif
#if !defined(PQC_NO_BSWAP) && defined(__GNUC__) &&                      \
   ((__GNUC__ * 100 + __GNUC_MINOR__ >= 403) ||                         \
    (__has_builtin(__builtin_bswap32) && __has_builtin(__builtin_bswap64)))
   #define PQC_HAVE_BSWAP_BUILTIN
#endif


/* $Source$ */
/* $Revision$ */
/* $Date$ */


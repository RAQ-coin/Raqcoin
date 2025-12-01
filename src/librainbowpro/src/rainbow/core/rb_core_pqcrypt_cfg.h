
#ifndef rb_PQC_PQCRYPT_CFG_H
#define rb_PQC_PQCRYPT_CFG_H

#include "rb_core_config.h"


#if 1
#if defined(__i386__) || defined(__i386) || defined(_M_IX86)
   #define rb_ENDIAN_LITTLE
   #define rb_ENDIAN_32BITWORD
   #define rb_PQC_FAST
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
   #define rb_ENDIAN_LITTLE
   #define rb_ENDIAN_64BITWORD
   #define rb_PQC_FAST
#endif
#endif


#if defined(PQC_PPC32)
   #define rb_ENDIAN_BIG
   #define rb_ENDIAN_32BITWORD
   #define rb_PQC_FAST
#endif


#if (defined(__R5900) || defined(R5900) || defined(__R5900__)) && (defined(_mips) || defined(__mips__) || defined(mips))
   #define rb_ENDIAN_64BITWORD
   #if defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__)
     #define rb_ENDIAN_BIG
   #endif
     #define rb_ENDIAN_LITTLE
   #endif
#endif


#if defined(_AIX) && defined(_BIG_ENDIAN)
  #define rb_ENDIAN_BIG
  #if defined(__LP64__) || defined(_ARCH_PPC64)
    #define rb_ENDIAN_64BITWORD
  #else
    #define rb_ENDIAN_32BITWORD
  #endif
#endif


#if defined(__hpux) || defined(__hpux__)
  #define rb_ENDIAN_BIG
  #if defined(__ia64) || defined(__ia64__) || defined(__LP64__)
    #define rb_ENDIAN_64BITWORD
  #else
    #define rb_ENDIAN_32BITWORD
  #endif
#endif


#if defined(__APPLE__) && defined(__MACH__)
  #if defined(__LITTLE_ENDIAN__) || defined(__x86_64__)
    #define rb_ENDIAN_LITTLE
  #else
    #define rb_ENDIAN_BIG
  #endif
  #if defined(__LP64__) || defined(__x86_64__)
    #define rb_ENDIAN_64BITWORD
  #else
    #define rb_ENDIAN_32BITWORD
  #endif
#endif


#if defined(__sparc__) || defined(__sparc)
  #define rb_ENDIAN_BIG
  #if defined(__arch64__) || defined(__sparcv9) || defined(__sparc_v9__)
    #define rb_ENDIAN_64BITWORD
  #else
    #define rb_ENDIAN_32BITWORD
  #endif
#endif


#if defined(__s390x__) || defined(__s390__)
  #define rb_ENDIAN_BIG
  #if defined(__s390x__)
    #define rb_ENDIAN_64BITWORD
  #else
    #define rb_ENDIAN_32BITWORD
  #endif
#endif


#if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__)
   #define rb_ENDIAN_64BITWORD
   #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      #define rb_ENDIAN_BIG
   #elif  __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      #define rb_ENDIAN_LITTLE
   #endif
   #define rb_PQC_FAST
#endif


#if !defined(rb_ENDIAN_BIG) && !defined(rb_ENDIAN_LITTLE)
  #if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
      defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ || \
      defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN) || \
      defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || \
      defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__)
    #define rb_ENDIAN_BIG
  #elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
      defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || \
      defined(__LITTLE_ENDIAN__) || defined(_LITTLE_ENDIAN) || \
      defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) || \
      defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
    #define rb_ENDIAN_LITTLE
  #else
    #error Cannot detect endianness
  #endif
#endif


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
   #if !defined(rb_ENDIAN_64BITWORD) && !defined(rb_ENDIAN_32BITWORD)
     #define rb_ENDIAN_64BITWORD
   #endif
#else
   typedef unsigned int ulong32;
   #if !defined(rb_ENDIAN_64BITWORD) && !defined(rb_ENDIAN_32BITWORD)
     #define rb_ENDIAN_32BITWORD
   #endif
#endif


#if defined(PQC_NO_FAST) || (__GNUC__ < 4) || defined(__STRICT_ANSI__)
   #undef rb_PQC_FAST
#endif



#ifdef rb_ENDIAN_64BITWORD
typedef ulong64 pqc_mp_digit;
#else
typedef ulong32 pqc_mp_digit;
#endif


#ifdef rb_PQC_NO_ASM
   #define rb_ENDIAN_NEUTRAL
   #undef rb_ENDIAN_32BITWORD
   #undef rb_ENDIAN_64BITWORD
   #undef rb_PQC_FAST
   #undef PQC_FAST_TYPE
   #define PQC_NO_ROLC
   #define PQC_NO_BSWAP
#endif

#if !defined(rb_ENDIAN_NEUTRAL) && (defined(rb_ENDIAN_BIG) || defined(rb_ENDIAN_LITTLE)) && !(defined(rb_ENDIAN_32BITWORD) || defined(rb_ENDIAN_64BITWORD))
    #error You must specify a word size as well as endianess in pqccrypt_cfg.h
#endif

#if !(defined(rb_ENDIAN_BIG) || defined(rb_ENDIAN_LITTLE))
   #define rb_ENDIAN_NEUTRAL
#endif

#if (defined(rb_ENDIAN_32BITWORD) && defined(rb_ENDIAN_64BITWORD))
    #error Cannot be 32 and 64 bit words...
#endif


#ifndef __has_builtin
   #define __has_builtin(x) 0
#endif
#if !defined(PQC_NO_BSWAP) && defined(__GNUC__) &&                      \
   ((__GNUC__ * 100 + __GNUC_MINOR__ >= 403) ||                         \
    (__has_builtin(__builtin_bswap32) && __has_builtin(__builtin_bswap64)))
   #define PQC_HAVE_BSWAP_BUILTIN
#endif


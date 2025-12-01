#ifndef RAQCOIN_PQCRYPT_UTIL_H
#define RAQCOIN_PQCRYPT_UTIL_H

/* macros for various libc functions you can change for embedded targets */
#ifndef XMALLOC
#define XMALLOC  malloc
#endif
#ifndef XREALLOC
#define XREALLOC realloc
#endif
#ifndef XCALLOC
#define XCALLOC  calloc
#endif
#ifndef XFREE
#define XFREE    free
#endif

#ifndef XMEMSET
#define XMEMSET  memset
#endif
#ifndef XMEMCPY
#define XMEMCPY  memcpy
#endif
#ifndef XMEMCMP
#define XMEMCMP  memcmp
#endif
#ifndef XMEM_NEQ
#define XMEM_NEQ  mem_neq
#endif
#ifndef XSTRCMP
#define XSTRCMP strcmp
#endif

#ifndef XCLOCK
#define XCLOCK   clock
#endif

#ifndef XQSORT
#define XQSORT qsort
#endif

#if ( defined(malloc) || defined(realloc) || defined(calloc) || defined(free) || \
      defined(memset) || defined(memcpy) || defined(memcmp) || defined(strcmp) || \
      defined(clock) || defined(qsort) ) && !defined(PQC_NO_PROTOTYPES)
#define PQC_NO_PROTOTYPES
#endif

/* shortcut to disable automatic inclusion */
#if defined PQC_NOTHING && !defined PQC_EASY
  #define PQC_NO_CIPHERS
  #define PQC_NO_MODES
  #define PQC_NO_HASHES
  #define PQC_NO_MACS
  #define PQC_NO_PRNGS
  #define PQC_NO_FILE
#endif /* PQC_NOTHING */

/* Easy button? */
#ifdef PQC_EASY
   #define PQC_NO_CIPHERS
   #define PQC_RIJNDAEL

   #define PQC_NO_MODES
   #define PQC_ECB_MODE
   #define PQC_CBC_MODE
   #define PQC_CTR_MODE

   #define PQC_NO_HASHES
   #define PQC_SHA1
   #define PQC_SHA512
   #define PQC_SHA384
   #define PQC_SHA256
   #define PQC_SHA224
   #define PQC_HASH_HELPERS

   #define PQC_NO_MACS
   #define PQC_HMAC
   #define PQC_OMAC
   #define PQC_CCM_MODE

   #define PQC_NO_PRNGS
   #define PQC_SPRNG
   #define PQC_FORTUNA
   #define PQC_DEVRANDOM
   #define PQC_TRY_URANDOM_FIRST
   #define PQC_RNG_GET_BYTES
   #define PQC_RNG_MAKE_PRNG

#endif

/* The minimal set of functionality to run the tests */
#ifdef PQC_MINIMAL
   #define PQC_RIJNDAEL
   #define PQC_SHA256
   #define PQC_FORTUNA
   #define PQC_CTR_MODE

   #define PQC_RNG_MAKE_PRNG
   #define PQC_RNG_GET_BYTES
   #define PQC_DEVRANDOM
   #define PQC_TRY_URANDOM_FIRST

   #undef PQC_NO_FILE
#endif

/* Enable self-test test vector checking */
#ifndef PQC_NO_TEST
   #define PQC_TEST
#endif
/* Enable extended self-tests */
/* #define PQC_TEST_EXT */

/* Use small code where possible */
/* #define PQC_SMALL_CODE */

/* clean the stack of functions which put private information on stack */
/* #define PQC_CLEAN_STACK */

/* disable all file related functions */
/* #define PQC_NO_FILE */

/* disable all forms of ASM */
/* #define PQC_NO_ASM */

/* disable FAST mode */
/* #define PQC_NO_FAST */

/* disable BSWAP on x86 */
/* #define PQC_NO_BSWAP */

/* ---> Symmetric Block Ciphers <--- */
#ifndef PQC_NO_CIPHERS

#define PQC_RIJNDAEL

#endif /* PQC_NO_CIPHERS */


/* ---> Block Cipher Modes of Operation <--- */
#ifndef PQC_NO_MODES

#define PQC_CFB_MODE
#define PQC_OFB_MODE
#define PQC_ECB_MODE
#define PQC_CBC_MODE
#define PQC_CTR_MODE

/* F8 chaining mode */
#define PQC_F8_MODE

/* LRW mode */
#define PQC_LRW_MODE
#ifndef PQC_NO_TABLES
   /* like GCM mode this will enable 16 8x128 tables [64KB] that make
    * seeking very fast.
    */
   #define PQC_LRW_TABLES
#endif

/* XTS mode */
#define PQC_XTS_MODE

#endif /* PQC_NO_MODES */

/* ---> One-Way Hash Functions <--- */
#ifndef PQC_NO_HASHES


#define PQC_SHA512
#define PQC_SHA512_256
#define PQC_SHA512_224
#define PQC_SHA384
#define PQC_SHA256
#define PQC_SHA224
#define PQC_SHA1
#define PQC_RIPEMD128
#define PQC_RIPEMD160
#define PQC_RIPEMD256
#define PQC_RIPEMD320

#define PQC_HASH_HELPERS

#endif /* PQC_NO_HASHES */


/* ---> MAC functions <--- */
#ifndef PQC_NO_MACS

#define PQC_HMAC
#define PQC_OMAC
#define PQC_PMAC
#define PQC_XCBC
#define PQC_F9_MODE
#define PQC_PELICAN

/* ---> Encrypt + Authenticate Modes <--- */

#define PQC_EAX_MODE

#define PQC_OCB_MODE
#define PQC_OCB3_MODE
#define PQC_CCM_MODE
#define PQC_GCM_MODE

/* Use 64KiB tables */
#ifndef PQC_NO_TABLES
   #define PQC_GCM_TABLES
#endif

/* USE SSE2? requires GCC works on x86_32 and x86_64*/
#ifdef PQC_GCM_TABLES
/* #define PQC_GCM_TABLES_SSE2 */
#endif

#endif /* PQC_NO_MACS */


/* --> Pseudo Random Number Generators <--- */
#ifndef PQC_NO_PRNGS


/* a PRNG that simply reads from an available system source */
#define PQC_SPRNG

/* Fortuna PRNG */
#define PQC_FORTUNA

/* the *nix style /dev/random device */
#define PQC_DEVRANDOM
/* try /dev/urandom before trying /dev/random
 * are you sure you want to disable this? http://www.2uo.de/myths-about-urandom/ */
#define PQC_TRY_URANDOM_FIRST
/* rng_get_bytes() */
#define PQC_RNG_GET_BYTES
/* rng_make_prng() */
#define PQC_RNG_MAKE_PRNG

/* enable the ltc_rng hook to integrate e.g. embedded hardware RNG's easily */
/* #define PQC_PRNG_ENABLE_PQC_RNG */

#endif /* PQC_NO_PRNGS */

#ifdef PQC_FORTUNA

#ifndef PQC_FORTUNA_WD
/* reseed every N calls to the read function */
#define PQC_FORTUNA_WD    10
#endif

#ifndef PQC_FORTUNA_POOLS
/* number of pools (4..32) can save a bit of ram by lowering the count */
#define PQC_FORTUNA_POOLS 32
#endif

#endif /* PQC_FORTUNA */


#if defined(PQC_PELICAN) && !defined(PQC_RIJNDAEL)
   #error Pelican-MAC requires PQC_RIJNDAEL
#endif

#if defined(PQC_EAX_MODE) && !(defined(PQC_CTR_MODE) && defined(PQC_OMAC))
   #error PQC_EAX_MODE requires CTR and PQC_OMAC mode
#endif


/* THREAD management */
#ifdef PQC_PTHREAD

#include <pthread.h>

#define PQC_MUTEX_GLOBAL(x)   pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER;
#define PQC_MUTEX_PROTO(x)    extern pthread_mutex_t x;
#define PQC_MUTEX_TYPE(x)     pthread_mutex_t x;
#define PQC_MUTEX_INIT(x)     pthread_mutex_init(x, NULL);
#define PQC_MUTEX_LOCK(x)     pthread_mutex_lock(x);
#define PQC_MUTEX_UNLOCK(x)   pthread_mutex_unlock(x);

#else

/* default no functions */
#define PQC_MUTEX_GLOBAL(x)
#define PQC_MUTEX_PROTO(x)
#define PQC_MUTEX_TYPE(x)
#define PQC_MUTEX_INIT(x)
#define PQC_MUTEX_LOCK(x)
#define PQC_MUTEX_UNLOCK(x)

#endif

/* Debuggers */

/* define this if you use Valgrind, note: it CHANGES the way SOBER-128 and PQC_RC4 work (see the code) */
/* #define PQC_VALGRIND */

/**
   Zero a block of memory
   @param out    The destination of the area to zero
   @param outlen The length of the area to zero (octets)
*/
inline void zeromem(volatile void *out, size_t outlen) {
   volatile char *mem = (volatile char *)out;
   while (outlen-- > 0) {
      *mem++ = '\0';
   }
}

/**
   Burn some stack memory
   @param len amount of stack to burn in bytes
*/
inline void burn_stack(unsigned long len) {
   unsigned char buf[32];
   zeromem(buf, sizeof(buf));
   if (len > (unsigned long)sizeof(buf))
      burn_stack(len - sizeof(buf));
}


#endif

/* $Source$ */
/* $Revision$ */
/* $Date$ */


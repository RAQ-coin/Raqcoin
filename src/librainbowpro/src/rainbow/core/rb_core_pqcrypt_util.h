#ifndef rb_PQC_PQCRYPT_UTIL_H
#define rb_PQC_PQCRYPT_UTIL_H

#include "rb_core_config.h"

#if ( defined(malloc) || defined(realloc) || defined(calloc) || defined(free) || \
      defined(memset) || defined(memcpy) || defined(memcmp) || defined(strcmp) || \
      defined(clock) || defined(qsort) ) && !defined(rb_PQC_NO_PROTOTYPES)
#define rb_PQC_NO_PROTOTYPES
#endif


#if defined rb_PQC_NOTHING && !defined rb_PQC_EASY
  #define rb_PQC_NO_CIPHERS
  #define rb_PQC_NO_MODES
  #define rb_PQC_NO_HASHES
  #define rb_PQC_NO_MACS
  #define rb_PQC_NO_PRNGS
  #define rb_PQC_NO_FILE
#endif /* rb_PQC_NOTHING */


#ifdef rb_PQC_EASY
   #define rb_PQC_NO_CIPHERS
   #define rb_PQC_RIJNDAEL

   #define rb_PQC_NO_MODES
   #define rb_PQC_ECB_MODE
   #define rb_PQC_CBC_MODE
   #define rb_PQC_CTR_MODE

   #define rb_PQC_NO_HASHES
   #define rb_PQC_SHA1
   #define rb_PQC_SHA512
   #define rb_PQC_SHA384
   #define rb_PQC_SHA256
   #define rb_PQC_SHA224
   #define rb_PQC_HASH_HELPERS

   #define rb_PQC_NO_MACS
   #define rb_PQC_HMAC
   #define rb_PQC_OMAC
   #define rb_PQC_CCM_MODE

   #define rb_PQC_NO_PRNGS
   #define rb_PQC_SPRNG
   #define rb_PQC_FORTUNA
   #define rb_PQC_DEVRANDOM
   #define rb_PQC_TRY_URANDOM_FIRST
   #define rb_PQC_RNG_GET_BYTES
   #define rb_PQC_RNG_MAKE_PRNG

#endif


#ifdef PQC_MINIMAL
   #define rb_PQC_RIJNDAEL
   #define rb_PQC_SHA256
   #define rb_PQC_FORTUNA
   #define rb_PQC_CTR_MODE

   #define rb_PQC_RNG_MAKE_PRNG
   #define rb_PQC_RNG_GET_BYTES
   #define rb_PQC_DEVRANDOM
   #define rb_PQC_TRY_URANDOM_FIRST

   #undef rb_PQC_NO_FILE
#endif


#ifndef rb_PQC_NO_TEST
   #define rb_PQC_TEST
#endif

/* #define PQC_TEST_EXT */


/* #define rb_PQC_SMALL_CODE */


/* #define PQC_CLEAN_STACK */


/* #define rb_PQC_NO_FILE */


/* #define rb_PQC_NO_ASM */


/* #define PQC_NO_FAST */


/* #define PQC_NO_BSWAP */


#ifndef rb_PQC_NO_CIPHERS

#define rb_PQC_RIJNDAEL

#endif /* rb_PQC_NO_CIPHERS */



#ifndef rb_PQC_NO_MODES

#define rb_PQC_CFB_MODE
#define rb_PQC_OFB_MODE
#define rb_PQC_ECB_MODE
#define rb_PQC_CBC_MODE
#define rb_PQC_CTR_MODE


#define rb_PQC_F8_MODE


#define rb_PQC_LRW_MODE
#ifndef rb_PQC_NO_TABLES
   #define rb_PQC_LRW_TABLES
#endif


#define rb_PQC_XTS_MODE

#endif /* rb_PQC_NO_MODES */


#ifndef rb_PQC_NO_HASHES


#define rb_PQC_SHA512
#define rb_PQC_SHA512_256
#define rb_PQC_SHA512_224
#define rb_PQC_SHA384
#define rb_PQC_SHA256
#define rb_PQC_SHA224
#define rb_PQC_SHA1
#define rb_PQC_RIPEMD128
#define rb_PQC_RIPEMD160
#define rb_PQC_RIPEMD256
#define rb_PQC_RIPEMD320

#define rb_PQC_HASH_HELPERS

#endif /* rb_PQC_NO_HASHES */



#ifndef rb_PQC_NO_MACS

#define rb_PQC_HMAC
#define rb_PQC_OMAC
#define rb_PQC_PMAC
#define rb_PQC_XCBC
#define rb_PQC_F9_MODE
#define rb_PQC_PELICAN



#define rb_PQC_EAX_MODE

#define rb_PQC_OCB_MODE
#define rb_PQC_OCB3_MODE
#define rb_PQC_CCM_MODE
#define rb_PQC_GCM_MODE


#ifndef rb_PQC_NO_TABLES
   #define rb_PQC_GCM_TABLES
#endif


#ifdef rb_PQC_GCM_TABLES
/* #define PQC_GCM_TABLES_SSE2 */
#endif

#endif /* rb_PQC_NO_MACS */



#ifndef rb_PQC_NO_PRNGS



#define rb_PQC_SPRNG


#define rb_PQC_FORTUNA


#define rb_PQC_DEVRANDOM

#define rb_PQC_TRY_URANDOM_FIRST

#define rb_PQC_RNG_GET_BYTES

#define rb_PQC_RNG_MAKE_PRNG


/* #define PQC_PRNG_ENABLE_PQC_RNG */

#endif /* rb_PQC_NO_PRNGS */

#ifdef rb_PQC_FORTUNA

#ifndef PQC_FORTUNA_WD
#define PQC_FORTUNA_WD    10
#endif

#ifndef rb_PQC_FORTUNA_POOLS
#define rb_PQC_FORTUNA_POOLS 32
#endif

#endif /* rb_PQC_FORTUNA */


#if defined(rb_PQC_PELICAN) && !defined(rb_PQC_RIJNDAEL)
   #error Pelican-MAC requires rb_PQC_RIJNDAEL
#endif

#if defined(rb_PQC_EAX_MODE) && !(defined(rb_PQC_CTR_MODE) && defined(rb_PQC_OMAC))
   #error rb_PQC_EAX_MODE requires CTR and rb_PQC_OMAC mode
#endif


#ifdef PQC_PTHREAD

#include <pthread.h>

#define rb_PQC_MUTEX_GLOBAL(x)   pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER;
#define rb_PQC_MUTEX_PROTO(x)    extern pthread_mutex_t x;
#define rb_PQC_MUTEX_TYPE(x)     pthread_mutex_t x;
#define rb_PQC_MUTEX_INIT(x)     pthread_mutex_init(x, NULL);
#define rb_PQC_MUTEX_LOCK(x)     pthread_mutex_lock(x);
#define rb_PQC_MUTEX_UNLOCK(x)   pthread_mutex_unlock(x);

#else

/* default no functions */
#define rb_PQC_MUTEX_GLOBAL(x)
#define rb_PQC_MUTEX_PROTO(x)
#define rb_PQC_MUTEX_TYPE(x)
#define rb_PQC_MUTEX_INIT(x)
#define rb_PQC_MUTEX_LOCK(x)
#define rb_PQC_MUTEX_UNLOCK(x)

#endif


#endif

/* $Source$ */
/* $Revision$ */
/* $Date$ */


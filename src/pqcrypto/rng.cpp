#include <time.h>
#include <stdio.h>

#include "pqcrypto.h"


#ifdef PQC_DEVRANDOM
/* on *NIX read /dev/random */
static unsigned long rngNix(unsigned char *buf, unsigned long len,
                             void (*callback)(void)) {
#ifdef PQC_NO_FILE
    PQC_UNUSED_PARAM(callback);
    PQC_UNUSED_PARAM(buf);
    PQC_UNUSED_PARAM(len);
    return 0;
#else
    FILE *f;
    unsigned long x;
    PQC_UNUSED_PARAM(callback);
#ifdef PQC_TRY_URANDOM_FIRST
    f = fopen("/dev/urandom", "rb");
    if (f == NULL)
#endif /* PQC_TRY_URANDOM_FIRST */
       f = fopen("/dev/random", "rb");

    if (f == NULL) {
       return 0;
    }

    /* disable buffering */
    if (setvbuf(f, NULL, _IONBF, 0) != 0) {
       fclose(f);
       return 0;
    }

    x = (unsigned long)fread(buf, 1, (size_t)len, f);
    fclose(f);
    return x;
#endif /* PQC_NO_FILE */
}

#endif /* PQC_DEVRANDOM */


#if !defined(_WIN32_WCE)

#define ANSI_RNG

static unsigned long rngAnsic(unsigned char *buf, unsigned long len,
                               void (*callback)(void))
{
   clock_t t1;
   int l, acc, bits, a, b;

   l = len;
   bits = 8;
   acc  = a = b = 0;
   while (len--) {
       if (callback != NULL) callback();
       while (bits--) {
          do {
             t1 = clock(); while (t1 == clock()) a ^= 1;
             t1 = clock(); while (t1 == clock()) b ^= 1;
          } while (a == b);
          acc = (acc << 1) | a;
       }
       *buf++ = acc;
       acc  = 0;
       bits = 8;
   }
   return l;
}

#endif

/* Try the Microsoft CSP */
#if defined(_WIN32) || defined(_WIN32_WCE)
#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0400
#endif
#ifdef _WIN32_WCE
   #define UNDER_CE
   #define ARM
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>

static unsigned long rngWin32(unsigned char *buf, unsigned long len,
                               void (*callback)(void))
{
   HCRYPTPROV hProv = 0;
   PQC_UNUSED_PARAM(callback);
   if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
                            (CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET)) &&
       !CryptAcquireContext (&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET))
      return 0;

   if (CryptGenRandom(hProv, len, buf) == TRUE) {
      CryptReleaseContext(hProv, 0);
      return len;
   } else {
      CryptReleaseContext(hProv, 0);
      return 0;
   }
}

#endif /* WIN32 */

/**
  Read the system RNG
  @param out       Destination
  @param outlen    Length desired (octets)
  @param callback  Pointer to void function to act as "callback" when RNG is slow.  This can be NULL
  @return Number of octets read
*/
unsigned long getRngBytes(unsigned char *out, unsigned long outlen,
                            void (*callback)(void))
{
   unsigned long x;

  // PQC_ARGCHK(out != NULL);
#if 1
#ifdef PQC_PRNG_ENABLE_PQC_RNG
   if (pqc_rng) {
      x = pqc_rng(out, outlen, callback);
      if (x != 0) {
         return x;
      }
   }
#endif
#endif
#if defined(_WIN32) || defined(_WIN32_WCE)
   x = rngWin32(out, outlen, callback); if (x != 0) { return x; }
#elif defined(PQC_DEVRANDOM)
   x = rngNix(out, outlen, callback);   if (x != 0) { return x; }
#endif
#ifdef ANSI_RNG
   x = rngAnsic(out, outlen, callback); if (x != 0) { return x; }
#endif
   return 0;
}



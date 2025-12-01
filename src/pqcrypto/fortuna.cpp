#include "pqcrypto.h"
//#include "rng.h"

/**
  @file fortuna.c
  Fortuna PRNG, Tom St Denis
*/

/* 
We deviate slightly here for reasons of simplicity [and to fit in the API].  First all "sources"
in the AddEntropy function are fixed to 0.  Second since no reliable timer is provided
we reseed automatically when len(pool0) >= 64 or every PQC_FORTUNA_WD calls to the read function */


/* requries PQC_SHA256 and AES  */
#if !(defined(PQC_RIJNDAEL) && defined(PQC_SHA256))
   #error PQC_FORTUNA requires PQC_SHA256 and PQC_RIJNDAEL (AES)
#endif

#ifndef PQC_FORTUNA_POOLS
   #warning PQC_FORTUNA_POOLS was not previously defined (old headers?)
   #define PQC_FORTUNA_POOLS 32
#endif

#if PQC_FORTUNA_POOLS < 4 || PQC_FORTUNA_POOLS > 32
   #error PQC_FORTUNA_POOLS must be in [4..32]
#endif


/* update the IV */
static void fortuna_update_iv(prng_state *prng) {
   int            x;
   unsigned char *IV;
   /* update IV */
   IV = prng->fortuna.IV;
   for (x = 0; x < 16; x++) {
      IV[x] = (IV[x] + 1) & 255;
      if (IV[x] != 0) break;
   }
}

/* reseed the PRNG */
static int fortuna_reseed(prng_state *prng) {
   unsigned char tmp[MAXBLOCKSIZE];
   Sha256    md;
   int           err, x;

   ++prng->fortuna.reset_cnt;

   /* new K == PQC_SHA256(K || s) where s == PQC_SHA256(P0) || PQC_SHA256(P1) ... */
   sha256Init(&md);
   if ((err = sha256Process(&md, prng->fortuna.K, 32)) != PQCRYPT_OK) {
      sha256Done(&md, tmp);
      return err;
   }

   for (x = 0; x < PQC_FORTUNA_POOLS; x++) {
       if (x == 0 || ((prng->fortuna.reset_cnt >> (x-1)) & 1) == 0) {
          /* terminate this hash */
          if ((err = sha256Done(&prng->fortuna.pool[x], tmp)) != PQCRYPT_OK) {
             sha256Done(&md, tmp);
             return err;
          }
          /* add it to the string */
          if ((err = sha256Process(&md, tmp, 32)) != PQCRYPT_OK) {
             sha256Done(&md, tmp);
             return err;
          }
          /* reset this pool */
          if ((err = sha256Init(&prng->fortuna.pool[x])) != PQCRYPT_OK) {
             sha256Done(&md, tmp);
             return err;
          }
       } else {
          break;
       }
   }

   /* finish key */
   if ((err = sha256Done(&md, prng->fortuna.K)) != PQCRYPT_OK) {
      return err;
   }
   if ((err = rijndael_setup(prng->fortuna.K, 32, 0, &prng->fortuna.skey)) != PQCRYPT_OK) {
      return err;
   }
   fortuna_update_iv(prng);

   /* reset pool len */
   prng->fortuna.pool0_len = 0;
   prng->fortuna.wd        = 0;

   zeromem(&md, sizeof(md));
   zeromem(tmp, sizeof(tmp));

   return PQCRYPT_OK;
}

/**
  Start the PRNG
  @param prng     [out] The PRNG state to initialize
  @return PQCRYPT_OK if successful
*/
int fortuna_start(prng_state *prng) {
   int err, x, y;
   unsigned char tmp[MAXBLOCKSIZE];

   PQC_ARGCHK(prng != NULL);

   /* initialize the pools */
   for (x = 0; x < PQC_FORTUNA_POOLS; x++) {
       if ((err = sha256Init(&prng->fortuna.pool[x])) != PQCRYPT_OK) {
          for (y = 0; y < x; y++) {
              sha256Done(&prng->fortuna.pool[y], tmp);
          }
          return err;
       }
   }
   prng->fortuna.pool_idx = prng->fortuna.pool0_len = prng->fortuna.wd = 0;
   prng->fortuna.reset_cnt = 0;

   /* reset bufs */
   zeromem(prng->fortuna.K, 32);
   if ((err = rijndael_setup(prng->fortuna.K, 32, 0, &prng->fortuna.skey)) != PQCRYPT_OK) {
      for (x = 0; x < PQC_FORTUNA_POOLS; x++) {
          sha256Done(&prng->fortuna.pool[x], tmp);
      }
      return err;
   }
   zeromem(prng->fortuna.IV, 16);

   PQC_MUTEX_INIT(&prng->fortuna.prng_lock)

   return PQCRYPT_OK;
}

/**
  Add entropy to the PRNG state
  @param in       The data to add
  @param inlen    Length of the data to add
  @param prng     PRNG state to update
  @return PQCRYPT_OK if successful
*/
int fortuna_add_entropy(const unsigned char *in, unsigned long inlen, prng_state *prng) {
   unsigned char tmp[2];
   int           err;

   PQC_ARGCHK(in  != NULL);
   PQC_ARGCHK(prng != NULL);

   PQC_MUTEX_LOCK(&prng->fortuna.prng_lock);

   /* ensure inlen <= 32 */
   if (inlen > 32) {
      PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return PQCRYPT_INVALID_ARG;
   }

   /* add s || length(in) || in to pool[pool_idx] */
   tmp[0] = 0;
   tmp[1] = (unsigned char)inlen;
   if ((err = sha256Process(&prng->fortuna.pool[prng->fortuna.pool_idx], tmp, 2)) != PQCRYPT_OK) {
      PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return err;
   }
   if ((err = sha256Process(&prng->fortuna.pool[prng->fortuna.pool_idx], in, inlen)) != PQCRYPT_OK) {
      PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return err;
   }
   if (prng->fortuna.pool_idx == 0) {
      prng->fortuna.pool0_len += inlen;
   }
   if (++(prng->fortuna.pool_idx) == PQC_FORTUNA_POOLS) {
      prng->fortuna.pool_idx = 0;
   }

   PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
   return PQCRYPT_OK;
}

/**
  Make the PRNG ready to read from
  @param prng   The PRNG to make active
  @return PQCRYPT_OK if successful
*/
int fortuna_ready(prng_state *prng) {
   return fortuna_reseed(prng);
}

/**
  Read from the PRNG
  @param out      Destination
  @param outlen   Length of output
  @param prng     The active PRNG to read from
  @return Number of octets read
*/
unsigned long fortuna_read(unsigned char *out, unsigned long outlen, prng_state *prng) {
   unsigned char tmp[16];
   unsigned long tlen;

   PQC_ARGCHK(out  != NULL);
   PQC_ARGCHK(prng != NULL);

   PQC_MUTEX_LOCK(&prng->fortuna.prng_lock);

   /* do we have to reseed? */
   if (++prng->fortuna.wd == PQC_FORTUNA_WD || prng->fortuna.pool0_len >= 64) {
      if (fortuna_reseed(prng) != PQCRYPT_OK) {
         PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
         return 0;
      }
   }

   /* now generate the blocks required */
   tlen = outlen;

   /* handle whole blocks without the extra XMEMCPY */
   while (outlen >= 16) {
      /* encrypt the IV and store it */
      rijndael_ecb_encrypt(prng->fortuna.IV, out, &prng->fortuna.skey);
      out += 16;
      outlen -= 16;
      fortuna_update_iv(prng);
   }

   /* left over bytes? */
   if (outlen > 0) {
      rijndael_ecb_encrypt(prng->fortuna.IV, tmp, &prng->fortuna.skey);
      XMEMCPY(out, tmp, outlen);
      fortuna_update_iv(prng);
   }

   /* generate new key */
   rijndael_ecb_encrypt(prng->fortuna.IV, prng->fortuna.K   , &prng->fortuna.skey);
   fortuna_update_iv(prng);

   rijndael_ecb_encrypt(prng->fortuna.IV, prng->fortuna.K+16, &prng->fortuna.skey);
   fortuna_update_iv(prng);

   if (rijndael_setup(prng->fortuna.K, 32, 0, &prng->fortuna.skey) != PQCRYPT_OK) {
      PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return 0;
   }
   zeromem(tmp, sizeof(tmp));
   PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
   return tlen;
}

/**
  Terminate the PRNG
  @param prng   The PRNG to terminate
  @return PQCRYPT_OK if successful
*/
int fortuna_done(prng_state *prng) {
   int           err, x;
   unsigned char tmp[32];

   PQC_ARGCHK(prng != NULL);
   PQC_MUTEX_LOCK(&prng->fortuna.prng_lock);

   /* terminate all the hashes */
   for (x = 0; x < PQC_FORTUNA_POOLS; x++) {
       if ((err = sha256Done(&(prng->fortuna.pool[x]), tmp)) != PQCRYPT_OK) {
          PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
          return err;
       }
   }
   /* call cipher done when we invent one ;-) */

   zeromem(tmp, sizeof(tmp));

   PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
   return PQCRYPT_OK;
}

/**
  Export the PRNG state
  @param out       [out] Destination
  @param outlen    [in/out] Max size and resulting size of the state
  @param prng      The PRNG to export
  @return PQCRYPT_OK if successful
*/
int fortuna_export(unsigned char *out, unsigned long *outlen, prng_state *prng) {
   int         x, err;
   Sha256 *md;

   PQC_ARGCHK(out    != NULL);
   PQC_ARGCHK(outlen != NULL);
   PQC_ARGCHK(prng   != NULL);

   PQC_MUTEX_LOCK(&prng->fortuna.prng_lock);

   /* we'll write bytes for s&g's */
   if (*outlen < 32*PQC_FORTUNA_POOLS) {
      PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      *outlen = 32*PQC_FORTUNA_POOLS;
      return PQCRYPT_BUFFER_OVERFLOW;
   }

   md = (Sha256 *)XMALLOC(sizeof(Sha256));
   if (md == NULL) {
      PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return PQCRYPT_MEM;
   }

   /* to emit the state we copy each pool, terminate it then hash it again so
    * an attacker who sees the state can't determine the current state of the PRNG
    */
   for (x = 0; x < PQC_FORTUNA_POOLS; x++) {
      /* copy the PRNG */
      XMEMCPY(md, &(prng->fortuna.pool[x]), sizeof(*md));

      /* terminate it */
      if ((err = sha256Done(md, out+x*32)) != PQCRYPT_OK) {
         goto LBL_ERR;
      }

      /* now hash it */
      if ((err = sha256Init(md)) != PQCRYPT_OK) {
         goto LBL_ERR;
      }
      if ((err = sha256Process(md, out+x*32, 32)) != PQCRYPT_OK) {
         goto LBL_ERR;
      }
      if ((err = sha256Done(md, out+x*32)) != PQCRYPT_OK) {
         goto LBL_ERR;
      }
   }
   *outlen = 32*PQC_FORTUNA_POOLS;
   err = PQCRYPT_OK;

LBL_ERR:
   zeromem(md, sizeof(*md));
   XFREE(md);
   PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
   return err;
}

/**
  Import a PRNG state
  @param in       The PRNG state
  @param inlen    Size of the state
  @param prng     The PRNG to import
  @return PQCRYPT_OK if successful
*/
int fortuna_import(const unsigned char *in, unsigned long inlen, prng_state *prng) {
   int err, x;

   PQC_ARGCHK(in   != NULL);
   PQC_ARGCHK(prng != NULL);

   if (inlen != 32*PQC_FORTUNA_POOLS) {
      return PQCRYPT_INVALID_ARG;
   }

   if ((err = fortuna_start(prng)) != PQCRYPT_OK) {
      return err;
   }
   for (x = 0; x < PQC_FORTUNA_POOLS; x++) {
      if ((err = fortuna_add_entropy(in+x*32, 32, prng)) != PQCRYPT_OK) {
         return err;
      }
   }
   return err;
}

/**
  portable way to get secure random bits to feed a PRNG  (Tom St Denis)
*/

/**
  Create a PRNG from a RNG
  @param bits     Number of bits of entropy desired (64 ... 1024)
  @param prng     [out] PRNG state to initialize
  @param callback A pointer to a void function for when the RNG is slow, this can be NULL
  @return PQCRYPT_OK if successful
*/
int rng_make_prng(int bits, prng_state *prng, void (*callback)(void)) {
   unsigned char buf[256];
   int err;

   PQC_ARGCHK(prng != NULL);


   if (bits < 64 || bits > 1024) {
      return PQCRYPT_INVALID_PRNGSIZE;
   }

   if ((err = fortuna_start(prng)) != PQCRYPT_OK) {
      return err;
   }

   bits = ((bits/8)+((bits&7)!=0?1:0)) * 2;
   if (getRngBytes(buf, (unsigned long)bits, callback) != (unsigned long)bits) {
      return PQCRYPT_ERROR_READPRNG;
   }

   if ((err = fortuna_add_entropy(buf, (unsigned long)bits, prng)) != PQCRYPT_OK) {
      return err;
   }

   if ((err = fortuna_ready(prng)) != PQCRYPT_OK) {
      return err;
   }

   zeromem(buf, sizeof(buf));
   return PQCRYPT_OK;
}



/* $Source$ */
/* $Revision$ */
/* $Date$ */


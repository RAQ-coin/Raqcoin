#include "rb_core_config.h"
#include "rb_core_RAINBOW_PRO_API.h"
#include "rb_core_pqcrypto.h"

#if !(defined(rb_PQC_RIJNDAEL) && defined(rb_PQC_SHA256))
   #error rb_PQC_FORTUNA requires rb_PQC_SHA256 and rb_PQC_RIJNDAEL (AES)
#endif

#ifndef rb_PQC_FORTUNA_POOLS
   #warning rb_PQC_FORTUNA_POOLS was not previously defined (old headers?)
   #define rb_PQC_FORTUNA_POOLS 32
#endif

#if rb_PQC_FORTUNA_POOLS < 4 || rb_PQC_FORTUNA_POOLS > 32
   #error rb_PQC_FORTUNA_POOLS must be in [4..32]
#endif

   void zeromem(volatile void* out, size_t outlen) 
   {
       volatile char* mem = (volatile char*)out;
       while (outlen-- > 0) {
           *mem++ = '\0';
       }
   }


static void rb_fortuna_update_iv(rb_prng_state *prng) {
   int            x;
   unsigned char *IV;
   /* update IV */
   IV = prng->fortuna.IV;
   for (x = 0; x < 16; x++) {
      IV[x] = (IV[x] + 1) & 255;
      if (IV[x] != 0) break;
   }
}


static int rb_fortuna_reseed(rb_prng_state *prng) {
   unsigned char tmp[rb_MAXBLOCKSIZE];
   rb_Sha256    md;
   int           err, x;

   ++prng->fortuna.reset_cnt;

  
   rb_sha256Init(&md);
   if ((err = rb_sha256Process(&md, prng->fortuna.K, 32)) != PQCRYPT_OK) {
       rb_sha256Done(&md, tmp);
      return err;
   }

   for (x = 0; x < rb_PQC_FORTUNA_POOLS; x++) {
       if (x == 0 || ((prng->fortuna.reset_cnt >> (x-1)) & 1) == 0) {
          /* terminate this hash */
          if ((err = rb_sha256Done(&prng->fortuna.pool[x], tmp)) != PQCRYPT_OK) {
              rb_sha256Done(&md, tmp);
             return err;
          }
          /* add it to the string */
          if ((err = rb_sha256Process(&md, tmp, 32)) != PQCRYPT_OK) {
              rb_sha256Done(&md, tmp);
             return err;
          }
          /* reset this pool */
          if ((err = rb_sha256Init(&prng->fortuna.pool[x])) != PQCRYPT_OK) {
              rb_sha256Done(&md, tmp);
             return err;
          }
       } else {
          break;
       }
   }

   /* finish key */
   if ((err = rb_sha256Done(&md, prng->fortuna.K)) != PQCRYPT_OK) {
      return err;
   }
   if ((err = rb_rijndael_setup(prng->fortuna.K, 32, 0, &prng->fortuna.skey)) != PQCRYPT_OK) {
      return err;
   }
   rb_fortuna_update_iv(prng);

   /* reset pool len */
   prng->fortuna.pool0_len = 0;
   prng->fortuna.wd        = 0;

   zeromem(&md, sizeof(rb_Sha256));
   zeromem(tmp, sizeof(tmp));

   return PQCRYPT_OK;
}

int rb_fortuna_start(rb_prng_state *prng) {
   int err, x, y;
   unsigned char tmp[rb_MAXBLOCKSIZE];

   rb_PQC_ARGCHK(prng != NULL);

   /* initialize the pools */
   for (x = 0; x < rb_PQC_FORTUNA_POOLS; x++) {
       if ((err = rb_sha256Init(&prng->fortuna.pool[x])) != PQCRYPT_OK) {
          for (y = 0; y < x; y++) {
              rb_sha256Done(&prng->fortuna.pool[y], tmp);
          }
          return err;
       }
   }
   prng->fortuna.pool_idx = prng->fortuna.pool0_len = prng->fortuna.wd = 0;
   prng->fortuna.reset_cnt = 0;

   /* reset bufs */
   zeromem(prng->fortuna.K, 32);
   if ((err = rb_rijndael_setup(prng->fortuna.K, 32, 0, &prng->fortuna.skey)) != PQCRYPT_OK) {
      for (x = 0; x < rb_PQC_FORTUNA_POOLS; x++) {
          rb_sha256Done(&prng->fortuna.pool[x], tmp);
      }
      return err;
   }
   zeromem(prng->fortuna.IV, 16);

   rb_PQC_MUTEX_INIT(&prng->fortuna.prng_lock)

   return PQCRYPT_OK;
}

int rb_fortuna_add_entropy(const unsigned char *in, unsigned int inlen, rb_prng_state *prng) {
   unsigned char tmp[2];
   int           err;

   rb_PQC_ARGCHK(in  != NULL);
   rb_PQC_ARGCHK(prng != NULL);

   rb_PQC_MUTEX_LOCK(&prng->fortuna.prng_lock);

   /* ensure inlen <= 32 */
   if (inlen > 32) {
       rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return PQCRYPT_INVALID_ARG;
   }

   /* add s || length(in) || in to pool[pool_idx] */
   tmp[0] = 0;
   tmp[1] = (unsigned char)inlen;
   if ((err = rb_sha256Process(&prng->fortuna.pool[prng->fortuna.pool_idx], tmp, 2)) != PQCRYPT_OK) {
       rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return err;
   }
   if ((err = rb_sha256Process(&prng->fortuna.pool[prng->fortuna.pool_idx], in, inlen)) != PQCRYPT_OK) {
       rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return err;
   }
   if (prng->fortuna.pool_idx == 0) {
      prng->fortuna.pool0_len += inlen;
   }
   if (++(prng->fortuna.pool_idx) == rb_PQC_FORTUNA_POOLS) {
      prng->fortuna.pool_idx = 0;
   }

   rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
   return PQCRYPT_OK;
}

int rb_fortuna_ready(rb_prng_state *prng) {
   return rb_fortuna_reseed(prng);
}

unsigned int rb_fortuna_read(unsigned char *out, unsigned int outlen, rb_prng_state *prng) {
   unsigned char tmp[16];
   unsigned int tlen;

   rb_PQC_ARGCHK(out  != NULL);
   rb_PQC_ARGCHK(prng != NULL);

   rb_PQC_MUTEX_LOCK(&prng->fortuna.prng_lock);

   /* do we have to reseed? */
   if (++prng->fortuna.wd == PQC_FORTUNA_WD || prng->fortuna.pool0_len >= 64) {
      if (rb_fortuna_reseed(prng) != PQCRYPT_OK) {
          rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
         return 0;
      }
   }

   /* now generate the blocks required */
   tlen = outlen;

   /* handle whole blocks without the extra XMEMCPY */
   while (outlen >= 16) {
      /* encrypt the IV and store it */
       rb_rijndael_ecb_encrypt(prng->fortuna.IV, out, &prng->fortuna.skey);
      out += 16;
      outlen -= 16;
      rb_fortuna_update_iv(prng);
   }

   /* left over bytes? */
   if (outlen > 0) {
       rb_rijndael_ecb_encrypt(prng->fortuna.IV, tmp, &prng->fortuna.skey);
      memcpy(out, tmp, outlen);
      rb_fortuna_update_iv(prng);
   }

   /* generate new key */
   rb_rijndael_ecb_encrypt(prng->fortuna.IV, prng->fortuna.K   , &prng->fortuna.skey);
   rb_fortuna_update_iv(prng);

   rb_rijndael_ecb_encrypt(prng->fortuna.IV, prng->fortuna.K+16, &prng->fortuna.skey);
   rb_fortuna_update_iv(prng);

   if (rb_rijndael_setup(prng->fortuna.K, 32, 0, &prng->fortuna.skey) != PQCRYPT_OK) {
       rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return 0;
   }
   zeromem(tmp, sizeof(tmp));
   rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
   return tlen;
}

int rb_fortuna_done(rb_prng_state *prng) {
   int           err, x;
   unsigned char tmp[32];

   rb_PQC_ARGCHK(prng != NULL);
   rb_PQC_MUTEX_LOCK(&prng->fortuna.prng_lock);

   /* terminate all the hashes */
   for (x = 0; x < rb_PQC_FORTUNA_POOLS; x++) {
       if ((err = rb_sha256Done(&(prng->fortuna.pool[x]), tmp)) != PQCRYPT_OK) {
           rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
          return err;
       }
   }
   /* call cipher done when we invent one ;-) */

   zeromem(tmp, sizeof(tmp));

   rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
   return PQCRYPT_OK;
}

int rb_fortuna_export(unsigned char *out, unsigned int*outlen, rb_prng_state *prng) {
   int         x, err;
   rb_Sha256 *md;

   rb_PQC_ARGCHK(out    != NULL);
   rb_PQC_ARGCHK(outlen != NULL);
   rb_PQC_ARGCHK(prng   != NULL);

   rb_PQC_MUTEX_LOCK(&prng->fortuna.prng_lock);

   /* we'll write bytes for s&g's */
   if (*outlen < 32* rb_PQC_FORTUNA_POOLS) {
       rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      *outlen = 32* rb_PQC_FORTUNA_POOLS;
      return PQCRYPT_BUFFER_OVERFLOW;
   }

   md = (rb_Sha256 *)rb_safe_calloc(1,sizeof(rb_Sha256));
   if (md == NULL) {
       rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
      return PQCRYPT_MEM;
   }

   /* to emit the state we copy each pool, terminate it then hash it again so
    * an attacker who sees the state can't determine the current state of the PRNG
    */
   for (x = 0; x < rb_PQC_FORTUNA_POOLS; x++) {
      /* copy the PRNG */
       memcpy(md, &(prng->fortuna.pool[x]), sizeof(rb_Sha256));

      /* terminate it */
      if ((err = rb_sha256Done(md, out+x*32)) != PQCRYPT_OK) {
         goto LBL_ERR;
      }

      /* now hash it */
      if ((err = rb_sha256Init(md)) != PQCRYPT_OK) {
         goto LBL_ERR;
      }
      if ((err = rb_sha256Process(md, out+x*32, 32)) != PQCRYPT_OK) {
         goto LBL_ERR;
      }
      if ((err = rb_sha256Done(md, out+x*32)) != PQCRYPT_OK) {
         goto LBL_ERR;
      }
   }
   *outlen = 32* rb_PQC_FORTUNA_POOLS;
   err = PQCRYPT_OK;

LBL_ERR:
   zeromem(md, sizeof(*md));
   rb_safe_free(md);
   rb_PQC_MUTEX_UNLOCK(&prng->fortuna.prng_lock);
   return err;
}


int rb_fortuna_import(const unsigned char *in, unsigned int inlen, rb_prng_state *prng) {
   int err, x;

   rb_PQC_ARGCHK(in   != NULL);
   rb_PQC_ARGCHK(prng != NULL);

   if (inlen != 32* rb_PQC_FORTUNA_POOLS) {
      return PQCRYPT_INVALID_ARG;
   }

   if ((err = rb_fortuna_start(prng)) != PQCRYPT_OK) {
      return err;
   }
   for (x = 0; x < rb_PQC_FORTUNA_POOLS; x++) {
      if ((err = rb_fortuna_add_entropy(in+x*32, 32, prng)) != PQCRYPT_OK) {
         return err;
      }
   }
   return err;
}

int rb_rng_make_prng(int bits, rb_prng_state *prng, void (*callback)(void)) {
   unsigned char buf[256];
   int err;

   rb_PQC_ARGCHK(prng != NULL);


   if (bits < 64 || bits > 1024) {
      return PQCRYPT_INVALID_PRNGSIZE;
   }

   if ((err = rb_fortuna_start(prng)) != PQCRYPT_OK) {
      return err;
   }

   bits = ((bits/8)+((bits&7)!=0?1:0)) * 2;
   if (rb_getRngBytes(buf, (unsigned int)bits, callback) != (unsigned int)bits) {
      return PQCRYPT_ERROR_READPRNG;
   }

   if ((err = rb_fortuna_add_entropy(buf, (unsigned int)bits, prng)) != PQCRYPT_OK) {
      return err;
   }

   if ((err = rb_fortuna_ready(prng)) != PQCRYPT_OK) {
      return err;
   }

   zeromem(buf, sizeof(buf));
   return PQCRYPT_OK;
}


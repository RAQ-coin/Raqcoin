#include "rb_core_pqcrypto.h"

#define rb___PQC_AES_TAB_C__
#include "rb_core_rijndael_tab.h"



#ifndef rb_ENCRYPT_ONLY

#define  rb_SETUP  rb_rijndael_setup  
#define  rb_ECB_ENC  rb_rijndael_ecb_encrypt
#define  rb_ECB_DEC  rb_rijndael_ecb_decrypt
#define  rb_ECB_DONE rb_rijndael_done
#define  rb_ECB_KS   rb_rijndael_keysize

#else

#define rb_SETUP    rb_rijndael_enc_setup
#define rb_ECB_ENC  rb_rijndael_enc_ecb_encrypt
#define rb_ECB_KS   rb_rijndael_enc_keysize
#define rb_ECB_DONE rb_rijndael_enc_done

#endif

static ulong32 rb_setup_mix(ulong32 temp)
{
   return (rb_Te4_3[rb_byte(temp, 2)]) ^
          (rb_Te4_2[rb_byte(temp, 1)]) ^
          (rb_Te4_1[rb_byte(temp, 0)]) ^
          (rb_Te4_0[rb_byte(temp, 3)]);
}

#ifndef rb_ENCRYPT_ONLY
#ifdef rb_PQC_SMALL_CODE
static ulong32 rb_setup_mix2(ulong32 temp)
{
   return rb_Td0(255 & rb_Te4[rb_byte(temp, 3)]) ^
       rb_Td1(255 & rb_Te4[rb_byte(temp, 2)]) ^
       rb_Td2(255 & rb_Te4[rb_byte(temp, 1)]) ^
       rb_Td3(255 & rb_Te4[rb_byte(temp, 0)]);
}
#endif
#endif


int rb_SETUP(const unsigned char *key, int keylen, int num_rounds, rb_symmetric_key *skey)
{
    int i;
    ulong32 temp, *rk;
#ifndef rb_ENCRYPT_ONLY
    ulong32 *rrk;
#endif
    rb_PQC_ARGCHK(key  != NULL);
    rb_PQC_ARGCHK(skey != NULL);

    if (keylen != 16 && keylen != 24 && keylen != 32) {
       return PQCRYPT_INVALID_KEYSIZE;
    }

    if (num_rounds != 0 && num_rounds != (10 + ((keylen/8)-2)*2)) {
       return PQCRYPT_INVALID_ROUNDS;
    }

    skey->rijndael.Nr = 10 + ((keylen/8)-2)*2;

    /* setup the forward key */
    i                 = 0;
    rk                = skey->rijndael.eK;
    rb_LOAD32H(rk[0], key     );
    rb_LOAD32H(rk[1], key +  4);
    rb_LOAD32H(rk[2], key +  8);
    rb_LOAD32H(rk[3], key + 12);
    if (keylen == 16) {
        for (;;) {
            temp  = rk[3];
            rk[4] = rk[0] ^ rb_setup_mix(temp) ^ rb_rcon_00[i];
            rk[5] = rk[1] ^ rk[4];
            rk[6] = rk[2] ^ rk[5];
            rk[7] = rk[3] ^ rk[6];
            if (++i == 10) {
               break;
            }
            rk += 4;
        }
    } else if (keylen == 24) {
        rb_LOAD32H(rk[4], key + 16);
        rb_LOAD32H(rk[5], key + 20);
        for (;;) {
        #ifdef _MSC_VER
            temp = skey->rijndael.eK[rk - skey->rijndael.eK + 5];
        #else
            temp = rk[5];
        #endif
            rk[ 6] = rk[ 0] ^ rb_setup_mix(temp) ^ rb_rcon_00[i];
            rk[ 7] = rk[ 1] ^ rk[ 6];
            rk[ 8] = rk[ 2] ^ rk[ 7];
            rk[ 9] = rk[ 3] ^ rk[ 8];
            if (++i == 8) {
                break;
            }
            rk[10] = rk[ 4] ^ rk[ 9];
            rk[11] = rk[ 5] ^ rk[10];
            rk += 6;
        }
    } else if (keylen == 32) {
        rb_LOAD32H(rk[4], key + 16);
        rb_LOAD32H(rk[5], key + 20);
        rb_LOAD32H(rk[6], key + 24);
        rb_LOAD32H(rk[7], key + 28);
        for (;;) {
        #ifdef _MSC_VER
            temp = skey->rijndael.eK[rk - skey->rijndael.eK + 7];
        #else
            temp = rk[7];
        #endif
            rk[ 8] = rk[ 0] ^ rb_setup_mix(temp) ^ rb_rcon_00[i];
            rk[ 9] = rk[ 1] ^ rk[ 8];
            rk[10] = rk[ 2] ^ rk[ 9];
            rk[11] = rk[ 3] ^ rk[10];
            if (++i == 7) {
                break;
            }
            temp = rk[11];
            rk[12] = rk[ 4] ^ rb_setup_mix(rb_RORc(temp, 8));
            rk[13] = rk[ 5] ^ rk[12];
            rk[14] = rk[ 6] ^ rk[13];
            rk[15] = rk[ 7] ^ rk[14];
            rk += 8;
        }
    } else {
       /* this can't happen */
       /* coverity[dead_error_line] */
       return PQCRYPT_ERROR;
    }

#ifndef rb_ENCRYPT_ONLY
    /* setup the inverse key now */
    rk   = skey->rijndael.dK;
    rrk  = skey->rijndael.eK + (28 + keylen) - 4;

    /* apply the inverse MixColumn transform to all round keys but the first and the last: */
    /* copy first */
    *rk++ = *rrk++;
    *rk++ = *rrk++;
    *rk++ = *rrk++;
    *rk   = *rrk;
    rk -= 3; rrk -= 3;

    for (i = 1; i < skey->rijndael.Nr; i++) {
        rrk -= 4;
        rk  += 4;
    #ifdef rb_PQC_SMALL_CODE
        temp = rrk[0];
        rk[0] = rb_setup_mix2(temp);
        temp = rrk[1];
        rk[1] = rb_setup_mix2(temp);
        temp = rrk[2];
        rk[2] = rb_setup_mix2(temp);
        temp = rrk[3];
        rk[3] = rb_setup_mix2(temp);
     #else
        temp = rrk[0];
        rk[0] =
            rb_Tks0[rb_byte(temp, 3)] ^
            rb_Tks1[rb_byte(temp, 2)] ^
            rb_Tks2[rb_byte(temp, 1)] ^
            rb_Tks3[rb_byte(temp, 0)];
        temp = rrk[1];
        rk[1] =
            rb_Tks0[rb_byte(temp, 3)] ^
            rb_Tks1[rb_byte(temp, 2)] ^
            rb_Tks2[rb_byte(temp, 1)] ^
            rb_Tks3[rb_byte(temp, 0)];
        temp = rrk[2];
        rk[2] =
            rb_Tks0[rb_byte(temp, 3)] ^
            rb_Tks1[rb_byte(temp, 2)] ^
            rb_Tks2[rb_byte(temp, 1)] ^
            rb_Tks3[rb_byte(temp, 0)];
        temp = rrk[3];
        rk[3] =
            rb_Tks0[rb_byte(temp, 3)] ^
            rb_Tks1[rb_byte(temp, 2)] ^
            rb_Tks2[rb_byte(temp, 1)] ^
            rb_Tks3[rb_byte(temp, 0)];
      #endif

    }

    /* copy last */
    rrk -= 4;
    rk  += 4;
    *rk++ = *rrk++;
    *rk++ = *rrk++;
    *rk++ = *rrk++;
    *rk   = *rrk;
#endif /* rb_ENCRYPT_ONLY */

    return PQCRYPT_OK;
}


#ifdef PQC_CLEAN_STACK
static int _rijndael_ecb_encrypt(const unsigned char *pt, unsigned char *ct, rb_symmetric_key *skey)
#else
int rb_ECB_ENC(const unsigned char *pt, unsigned char *ct, rb_symmetric_key *skey)
#endif
{
    ulong32 s0, s1, s2, s3, t0, t1, t2, t3, *rk;
    int Nr, r;

    rb_PQC_ARGCHK(pt != NULL);
    rb_PQC_ARGCHK(ct != NULL);
    rb_PQC_ARGCHK(skey != NULL);

    Nr = skey->rijndael.Nr;
    rk = skey->rijndael.eK;


    rb_LOAD32H(s0, pt      ); s0 ^= rk[0];
    rb_LOAD32H(s1, pt  +  4); s1 ^= rk[1];
    rb_LOAD32H(s2, pt  +  8); s2 ^= rk[2];
    rb_LOAD32H(s3, pt  + 12); s3 ^= rk[3];

#ifdef rb_PQC_SMALL_CODE

    for (r = 0; ; r++) {
        rk += 4;
        t0 =
            rb_Te0(rb_byte(s0, 3)) ^
            rb_Te1(rb_byte(s1, 2)) ^
            rb_Te2(rb_byte(s2, 1)) ^
            rb_Te3(rb_byte(s3, 0)) ^
            rk[0];
        t1 =
            rb_Te0(rb_byte(s1, 3)) ^
            rb_Te1(rb_byte(s2, 2)) ^
            rb_Te2(rb_byte(s3, 1)) ^
            rb_Te3(rb_byte(s0, 0)) ^
            rk[1];
        t2 =
            rb_Te0(rb_byte(s2, 3)) ^
            rb_Te1(rb_byte(s3, 2)) ^
            rb_Te2(rb_byte(s0, 1)) ^
            rb_Te3(rb_byte(s1, 0)) ^
            rk[2];
        t3 =
            rb_Te0(rb_byte(s3, 3)) ^
            rb_Te1(rb_byte(s0, 2)) ^
            rb_Te2(rb_byte(s1, 1)) ^
            rb_Te3(rb_byte(s2, 0)) ^
            rk[3];
        if (r == Nr-2) {
           break;
        }
        s0 = t0; s1 = t1; s2 = t2; s3 = t3;
    }
    rk += 4;

#else


    r = Nr >> 1;
    for (;;) {
        t0 =
            rb_Te0(rb_byte(s0, 3)) ^
            rb_Te1(rb_byte(s1, 2)) ^
            rb_Te2(rb_byte(s2, 1)) ^
            rb_Te3(rb_byte(s3, 0)) ^
            rk[4];
        t1 =
            rb_Te0(rb_byte(s1, 3)) ^
            rb_Te1(rb_byte(s2, 2)) ^
            rb_Te2(rb_byte(s3, 1)) ^
            rb_Te3(rb_byte(s0, 0)) ^
            rk[5];
        t2 =
            rb_Te0(rb_byte(s2, 3)) ^
            rb_Te1(rb_byte(s3, 2)) ^
            rb_Te2(rb_byte(s0, 1)) ^
            rb_Te3(rb_byte(s1, 0)) ^
            rk[6];
        t3 =
            rb_Te0(rb_byte(s3, 3)) ^
            rb_Te1(rb_byte(s0, 2)) ^
            rb_Te2(rb_byte(s1, 1)) ^
            rb_Te3(rb_byte(s2, 0)) ^
            rk[7];

        rk += 8;
        if (--r == 0) {
            break;
        }

        s0 =
            rb_Te0(rb_byte(t0, 3)) ^
            rb_Te1(rb_byte(t1, 2)) ^
            rb_Te2(rb_byte(t2, 1)) ^
            rb_Te3(rb_byte(t3, 0)) ^
            rk[0];
        s1 =
            rb_Te0(rb_byte(t1, 3)) ^
            rb_Te1(rb_byte(t2, 2)) ^
            rb_Te2(rb_byte(t3, 1)) ^
            rb_Te3(rb_byte(t0, 0)) ^
            rk[1];
        s2 =
            rb_Te0(rb_byte(t2, 3)) ^
            rb_Te1(rb_byte(t3, 2)) ^
            rb_Te2(rb_byte(t0, 1)) ^
            rb_Te3(rb_byte(t1, 0)) ^
            rk[2];
        s3 =
            rb_Te0(rb_byte(t3, 3)) ^
            rb_Te1(rb_byte(t0, 2)) ^
            rb_Te2(rb_byte(t1, 1)) ^
            rb_Te3(rb_byte(t2, 0)) ^
            rk[3];
    }

#endif


    s0 =
        (rb_Te4_3[rb_byte(t0, 3)]) ^
        (rb_Te4_2[rb_byte(t1, 2)]) ^
        (rb_Te4_1[rb_byte(t2, 1)]) ^
        (rb_Te4_0[rb_byte(t3, 0)]) ^
        rk[0];
    rb_STORE32H(s0, ct);
    s1 =
        (rb_Te4_3[rb_byte(t1, 3)]) ^
        (rb_Te4_2[rb_byte(t2, 2)]) ^
        (rb_Te4_1[rb_byte(t3, 1)]) ^
        (rb_Te4_0[rb_byte(t0, 0)]) ^
        rk[1];
    rb_STORE32H(s1, ct+4);
    s2 =
        (rb_Te4_3[rb_byte(t2, 3)]) ^
        (rb_Te4_2[rb_byte(t3, 2)]) ^
        (rb_Te4_1[rb_byte(t0, 1)]) ^
        (rb_Te4_0[rb_byte(t1, 0)]) ^
        rk[2];
    rb_STORE32H(s2, ct+8);
    s3 =
        (rb_Te4_3[rb_byte(t3, 3)]) ^
        (rb_Te4_2[rb_byte(t0, 2)]) ^
        (rb_Te4_1[rb_byte(t1, 1)]) ^
        (rb_Te4_0[rb_byte(t2, 0)]) ^
        rk[3];
    rb_STORE32H(s3, ct+12);

    return PQCRYPT_OK;
}



#ifndef rb_ENCRYPT_ONLY


#ifdef PQC_CLEAN_STACK
static int _rijndael_ecb_decrypt(const unsigned char *ct, unsigned char *pt, rb_symmetric_key *skey)
#else
int rb_ECB_DEC(const unsigned char *ct, unsigned char *pt, rb_symmetric_key *skey)
#endif
{
    ulong32 s0, s1, s2, s3, t0, t1, t2, t3, *rk;
    int Nr, r;

    rb_PQC_ARGCHK(pt != NULL);
    rb_PQC_ARGCHK(ct != NULL);
    rb_PQC_ARGCHK(skey != NULL);

    Nr = skey->rijndael.Nr;
    rk = skey->rijndael.dK;

    /*
     * map byte array block to cipher state
     * and add initial round key:
     */
    rb_LOAD32H(s0, ct      ); s0 ^= rk[0];
    rb_LOAD32H(s1, ct  +  4); s1 ^= rk[1];
    rb_LOAD32H(s2, ct  +  8); s2 ^= rk[2];
    rb_LOAD32H(s3, ct  + 12); s3 ^= rk[3];

#ifdef rb_PQC_SMALL_CODE
    for (r = 0; ; r++) {
        rk += 4;
        t0 =
            rb_Td0(rb_byte(s0, 3)) ^
            rb_Td1(rb_byte(s3, 2)) ^
            rb_Td2(rb_byte(s2, 1)) ^
            rb_Td3(rb_byte(s1, 0)) ^
            rk[0];
        t1 =
            rb_Td0(rb_byte(s1, 3)) ^
            rb_Td1(rb_byte(s0, 2)) ^
            rb_Td2(rb_byte(s3, 1)) ^
            rb_Td3(rb_byte(s2, 0)) ^
            rk[1];
        t2 =
            rb_Td0(rb_byte(s2, 3)) ^
            rb_Td1(rb_byte(s1, 2)) ^
            rb_Td2(rb_byte(s0, 1)) ^
            rb_Td3(rb_byte(s3, 0)) ^
            rk[2];
        t3 =
            rb_Td0(rb_byte(s3, 3)) ^
            rb_Td1(rb_byte(s2, 2)) ^
            rb_Td2(rb_byte(s1, 1)) ^
            rb_Td3(rb_byte(s0, 0)) ^
            rk[3];
        if (r == Nr-2) {
           break;
        }
        s0 = t0; s1 = t1; s2 = t2; s3 = t3;
    }
    rk += 4;

#else

    /*
     * Nr - 1 full rounds:
     */
    r = Nr >> 1;
    for (;;) {

        t0 =
            rb_Td0(rb_byte(s0, 3)) ^
            rb_Td1(rb_byte(s3, 2)) ^
            rb_Td2(rb_byte(s2, 1)) ^
            rb_Td3(rb_byte(s1, 0)) ^
            rk[4];
        t1 =
            rb_Td0(rb_byte(s1, 3)) ^
            rb_Td1(rb_byte(s0, 2)) ^
            rb_Td2(rb_byte(s3, 1)) ^
            rb_Td3(rb_byte(s2, 0)) ^
            rk[5];
        t2 =
            rb_Td0(rb_byte(s2, 3)) ^
            rb_Td1(rb_byte(s1, 2)) ^
            rb_Td2(rb_byte(s0, 1)) ^
            rb_Td3(rb_byte(s3, 0)) ^
            rk[6];
        t3 =
            rb_Td0(rb_byte(s3, 3)) ^
            rb_Td1(rb_byte(s2, 2)) ^
            rb_Td2(rb_byte(s1, 1)) ^
            rb_Td3(rb_byte(s0, 0)) ^
            rk[7];

        rk += 8;
        if (--r == 0) {
            break;
        }


        s0 =
            rb_Td0(rb_byte(t0, 3)) ^
            rb_Td1(rb_byte(t3, 2)) ^
            rb_Td2(rb_byte(t2, 1)) ^
            rb_Td3(rb_byte(t1, 0)) ^
            rk[0];
        s1 =
            rb_Td0(rb_byte(t1, 3)) ^
            rb_Td1(rb_byte(t0, 2)) ^
            rb_Td2(rb_byte(t3, 1)) ^
            rb_Td3(rb_byte(t2, 0)) ^
            rk[1];
        s2 =
            rb_Td0(rb_byte(t2, 3)) ^
            rb_Td1(rb_byte(t1, 2)) ^
            rb_Td2(rb_byte(t0, 1)) ^
            rb_Td3(rb_byte(t3, 0)) ^
            rk[2];
        s3 =
            rb_Td0(rb_byte(t3, 3)) ^
            rb_Td1(rb_byte(t2, 2)) ^
            rb_Td2(rb_byte(t1, 1)) ^
            rb_Td3(rb_byte(t0, 0)) ^
            rk[3];
    }
#endif


    s0 =
        (rb_Td4[rb_byte(t0, 3)] & 0xff000000) ^
        (rb_Td4[rb_byte(t3, 2)] & 0x00ff0000) ^
        (rb_Td4[rb_byte(t2, 1)] & 0x0000ff00) ^
        (rb_Td4[rb_byte(t1, 0)] & 0x000000ff) ^
        rk[0];
    rb_STORE32H(s0, pt);
    s1 =
        (rb_Td4[rb_byte(t1, 3)] & 0xff000000) ^
        (rb_Td4[rb_byte(t0, 2)] & 0x00ff0000) ^
        (rb_Td4[rb_byte(t3, 1)] & 0x0000ff00) ^
        (rb_Td4[rb_byte(t2, 0)] & 0x000000ff) ^
        rk[1];
    rb_STORE32H(s1, pt+4);
    s2 =
        (rb_Td4[rb_byte(t2, 3)] & 0xff000000) ^
        (rb_Td4[rb_byte(t1, 2)] & 0x00ff0000) ^
        (rb_Td4[rb_byte(t0, 1)] & 0x0000ff00) ^
        (rb_Td4[rb_byte(t3, 0)] & 0x000000ff) ^
        rk[2];
    rb_STORE32H(s2, pt+8);
    s3 =
        (rb_Td4[rb_byte(t3, 3)] & 0xff000000) ^
        (rb_Td4[rb_byte(t2, 2)] & 0x00ff0000) ^
        (rb_Td4[rb_byte(t1, 1)] & 0x0000ff00) ^
        (rb_Td4[rb_byte(t0, 0)] & 0x000000ff) ^
        rk[3];
    rb_STORE32H(s3, pt+12);

    return PQCRYPT_OK;
}




#endif 



void rb_ECB_DONE(rb_symmetric_key *skey)
{
    rb_PQC_UNUSED_PARAM(skey);
}



int rb_ECB_KS(int *keysize)
{
   rb_PQC_ARGCHK(keysize != NULL);

   if (*keysize < 16)
      return PQCRYPT_INVALID_KEYSIZE;
   if (*keysize < 24) {
      *keysize = 16;
      return PQCRYPT_OK;
   } else if (*keysize < 32) {
      *keysize = 24;
      return PQCRYPT_OK;
   } else {
      *keysize = 32;
      return PQCRYPT_OK;
   }
}


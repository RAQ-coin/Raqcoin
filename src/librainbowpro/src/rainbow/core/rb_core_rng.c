#include <string.h>
#include "rb_core_rng.h"
#include "rb_core_aes256_ecb.h"
#include "rb_core_rainbow_handle.h"
#include "rb_core_utils_hash.h"



void rb_AES256_ECB(unsigned char *key, unsigned char *ctr, unsigned char *buffer)
{

    rb_aes256_context ctx;
    unsigned char temp[16];

    memcpy(temp, ctr, 16);
    rb_aes256_init(&ctx, key);
    rb_aes256_encrypt_ecb(&ctx, temp);
    memcpy(buffer, temp, 16);

    rb_aes256_done(&ctx);


}



void rb_randombytes_init(unsigned long handle, unsigned char *entropy_input,unsigned char *personalization_string)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned char   seed_material[48]; 
    
    memcpy(seed_material, entropy_input, 48);

    for (int i=0; i<48; i++)
            seed_material[i] ^= personalization_string[i];

    memset(HD->DRBG_ctx.Key, 0x00, 32);
    memset(HD->DRBG_ctx.V, 0x00, 16);

    rb_AES256_CTR_DRBG_Update(seed_material, HD->DRBG_ctx.Key, HD->DRBG_ctx.V);
    HD->DRBG_ctx.reseed_counter = 1;
}


int rb_randombytes(unsigned long handle, unsigned char *x, unsigned int xlen)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned char   block[16];
    int             i = 0;
    
    while ( xlen > 0 ) {
        //increment V
        for (int j=15; j>=0; j--) {
            if (HD->DRBG_ctx.V[j] == 0xff )
                HD->DRBG_ctx.V[j] = 0x00;
            else {
                HD->DRBG_ctx.V[j]++;
                break;
            }
        }
        rb_AES256_ECB(HD->DRBG_ctx.Key, HD->DRBG_ctx.V, block);
        if ( xlen > 15 ) {
            memcpy(x+i, block, 16);
            i += 16;
            xlen -= 16;
        }
        else {
            memcpy(x+i, block, (size_t)xlen);
            xlen = 0;
        }
    }
    rb_AES256_CTR_DRBG_Update(NULL, HD->DRBG_ctx.Key, HD->DRBG_ctx.V);
    HD->DRBG_ctx.reseed_counter++;
    
    return RNG_SUCCESS;
}


void rb_AES256_CTR_DRBG_Update(unsigned char *provided_data,
                       unsigned char *Key,
                       unsigned char *V)
{
    unsigned char   temp[48];
    
    for (int i=0; i<3; i++) 
    {
        //increment V
        for (int j=15; j>=0; j--) 
        {
            if ( V[j] == 0xff )
                V[j] = 0x00;
            else 
            {
                V[j]++;
                break;
            }
        }
        rb_AES256_ECB(Key, V, temp+16*i);
    }
    if ( provided_data != NULL )
        for (int i=0; i<48; i++)
            temp[i] ^= provided_data[i];
    memcpy(Key, temp, 32);
    memcpy(V, temp+32, 16);
}








#include "rb_core_pqcrypto.h"




#define rb_ANSI_RNG

static unsigned int rb_rngAnsic(unsigned char* buf, unsigned int len, void (*callback)(void))
{

    clock_t t1;
    clock_t t2;
    int L, acc, bits, a, b;

    L = len;
    bits = 8;
    acc = a = b = 0;
    while (len--)
    {
        if (callback != NULL) callback();
        while (bits--) {
            do {
                t1 = clock(); 
                while (t1 == clock()) 
                    a ^= 1;
                t2 = clock(); 
                while (t2 == clock()) 
                    b ^= 1;
            } while (a == b);
            acc = (acc << 1) | a;
        }
        *buf++ = (unsigned char)acc;
        acc = 0;
        bits = 8;
    }

    return L;
}


static unsigned int rb_rngAnsic2(unsigned char* buf, unsigned int len, void (*callback)(void))
{
    int a = 0;
    int L = len;

    srand((unsigned int)clock());
    while (len--)
    {
        if (callback != NULL) callback();

        a = rand();
        *buf++ = (unsigned char)a;
        //printf("rand:%2x\n", (unsigned char)a);
    }

    return L;
}


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

static unsigned int rb_rngWin32(unsigned char* buf, unsigned int len,
    void (*callback)(void))
{
    HCRYPTPROV hProv = 0;
    rb_PQC_UNUSED_PARAM(callback);
    if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
        (CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET)) &&
        !CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET))
        return 0;

    if (CryptGenRandom(hProv, len, buf) == TRUE) {
        CryptReleaseContext(hProv, 0);
        return len;
    }
    else {
        CryptReleaseContext(hProv, 0);
        return 0;
    }
}

#endif /* WIN32 */


unsigned int rb_getRngBytes(unsigned char* out, unsigned int outlen,void (*callback)(void))
{
    unsigned int x;



#ifdef PQC_PRNG_ENABLE_PQC_RNG
    if (rb_pqc_rng) {
        x = rb_pqc_rng(out, outlen, callback);
        if (x != 0) {
            return x;
        }
    }
#endif


#if 1
#if defined(WIN32_VERSION) 
    x = rb_rngWin32(out, outlen, callback); if (x != 0) { return x; }
#endif

#ifdef rb_ANSI_RNG
    x = rb_rngAnsic2(out, outlen, callback); if (x != 0) { return x; }
#endif

#else

    x = rb_rngAnsic2(out, outlen, callback); if (x != 0) { return x; }
#endif
    return 0;
}


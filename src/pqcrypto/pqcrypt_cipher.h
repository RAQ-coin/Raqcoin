/* ---- SYMMETRIC KEY STUFF -----
 *
 * We put each of the ciphers scheduled keys in their own structs then we put all of
 * the key formats in one union.  This makes the function prototypes easier to use.
 */

#ifndef RAQCOIN_PQCRYPT_CIPHER_H
#define RAQCOIN_PQCRYPT_CIPHER_H


struct rijndael_key {
   ulong32 eK[60], dK[60];
   int Nr;
};

typedef union Symmetric_key {
   struct rijndael_key rijndael;
   void   *data;
} symmetric_key;

/** A block cipher ECB structure */
typedef struct {
   /** The index of the cipher chosen */
   int                 cipher,
   /** The block size of the given cipher */
                       blocklen;
   /** The scheduled key */
   symmetric_key       key;
} symmetric_ECB;

/** A block cipher CFB structure */
typedef struct {
   /** The index of the cipher chosen */
   int                 cipher,
   /** The block size of the given cipher */
                       blocklen,
   /** The padding offset */
                       padlen;
   /** The current IV */
   unsigned char       IV[MAXBLOCKSIZE],
   /** The pad used to encrypt/decrypt */
                       pad[MAXBLOCKSIZE];
   /** The scheduled key */
   symmetric_key       key;
} symmetric_CFB;

/** A block cipher OFB structure */
typedef struct {
   /** The index of the cipher chosen */
   int                 cipher,
   /** The block size of the given cipher */
                       blocklen,
   /** The padding offset */
                       padlen;
   /** The current IV */
   unsigned char       IV[MAXBLOCKSIZE];
   /** The scheduled key */
   symmetric_key       key;
} symmetric_OFB;

/** A block cipher CBC structure */
typedef struct {
   /** The index of the cipher chosen */
   int                 cipher,
   /** The block size of the given cipher */
                       blocklen;
   /** The current IV */
   unsigned char       IV[MAXBLOCKSIZE];
   /** The scheduled key */
   symmetric_key       key;
} symmetric_CBC;


/** A block cipher CTR structure */
typedef struct {
   /** The index of the cipher chosen */
   int                 cipher,
   /** The block size of the given cipher */
                       blocklen,
   /** The padding offset */
                       padlen,
   /** The mode (endianess) of the CTR, 0==little, 1==big */
                       mode,
   /** counter width */
                       ctrlen;

   /** The counter */
   unsigned char       ctr[MAXBLOCKSIZE],
   /** The pad used to encrypt/decrypt */
                       pad[MAXBLOCKSIZE];
   /** The scheduled key */
   symmetric_key       key;
} symmetric_CTR;


/* make aes an alias */
#define aes_setup           rijndael_setup
#define aes_ecb_encrypt     rijndael_ecb_encrypt
#define aes_ecb_decrypt     rijndael_ecb_decrypt
#define aes_test            rijndael_test
#define aes_done            rijndael_done
#define aes_keysize         rijndael_keysize

#define aes_enc_setup           rijndael_enc_setup
#define aes_enc_ecb_encrypt     rijndael_enc_ecb_encrypt
#define aes_enc_keysize         rijndael_enc_keysize

int rijndael_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int rijndael_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *skey);
int rijndael_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *skey);
int rijndael_test(void);
void rijndael_done(symmetric_key *skey);
int rijndael_keysize(int *keysize);
int rijndael_enc_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int rijndael_enc_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *skey);
void rijndael_enc_done(symmetric_key *skey);
int rijndael_enc_keysize(int *keysize);


int ecb_start(int cipher, const unsigned char *key,
              int keylen, int num_rounds, symmetric_ECB *ecb);
int ecb_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_ECB *ecb);
int ecb_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_ECB *ecb);
int ecb_done(symmetric_ECB *ecb);

int cfb_start(int cipher, const unsigned char *IV, const unsigned char *key,
              int keylen, int num_rounds, symmetric_CFB *cfb);
int cfb_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_CFB *cfb);
int cfb_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_CFB *cfb);
int cfb_getiv(unsigned char *IV, unsigned long *len, symmetric_CFB *cfb);
int cfb_setiv(const unsigned char *IV, unsigned long len, symmetric_CFB *cfb);
int cfb_done(symmetric_CFB *cfb);

int ofb_start(int cipher, const unsigned char *IV, const unsigned char *key,
              int keylen, int num_rounds, symmetric_OFB *ofb);
int ofb_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_OFB *ofb);
int ofb_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_OFB *ofb);
int ofb_getiv(unsigned char *IV, unsigned long *len, symmetric_OFB *ofb);
int ofb_setiv(const unsigned char *IV, unsigned long len, symmetric_OFB *ofb);
int ofb_done(symmetric_OFB *ofb);

int cbc_start(int cipher, const unsigned char *IV, const unsigned char *key,
               int keylen, int num_rounds, symmetric_CBC *cbc);
int cbc_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_CBC *cbc);
int cbc_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_CBC *cbc);
int cbc_getiv(unsigned char *IV, unsigned long *len, symmetric_CBC *cbc);
int cbc_setiv(const unsigned char *IV, unsigned long len, symmetric_CBC *cbc);
int cbc_done(symmetric_CBC *cbc);


#define CTR_COUNTER_LITTLE_ENDIAN    0x0000
#define CTR_COUNTER_BIG_ENDIAN       0x1000
#define PQC_CTR_RFC3686              0x2000

int ctr_start(               int   cipher,
              const unsigned char *IV,
              const unsigned char *key,       int keylen,
                             int  num_rounds, int ctr_mode,
                   symmetric_CTR *ctr);
int ctr_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_CTR *ctr);
int ctr_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_CTR *ctr);
int ctr_getiv(unsigned char *IV, unsigned long *len, symmetric_CTR *ctr);
int ctr_setiv(const unsigned char *IV, unsigned long len, symmetric_CTR *ctr);
int ctr_done(symmetric_CTR *ctr);
int ctr_test(void);


#endif
/* $Source$ */
/* $Revision$ */
/* $Date$ */

#ifndef rb_PQC_PQCRYPT_CIPHER_H
#define rb_PQC_PQCRYPT_CIPHER_H

#include "rb_core_config.h"

struct rb_rijndael_key {
   ulong32 eK[60], dK[60];
   int Nr;
};

typedef union Symmetric_key {
   struct rb_rijndael_key rijndael;
   void   *data;
} rb_symmetric_key;



int rb_rijndael_setup(const unsigned char *key, int keylen, int num_rounds, rb_symmetric_key *skey);
int rb_rijndael_ecb_encrypt(const unsigned char *pt, unsigned char *ct, rb_symmetric_key *skey);
int rb_rijndael_ecb_decrypt(const unsigned char *ct, unsigned char *pt, rb_symmetric_key *skey);

void rb_rijndael_done(rb_symmetric_key *skey);
int rb_rijndael_keysize(int *keysize);
int rb_rijndael_enc_setup(const unsigned char *key, int keylen, int num_rounds, rb_symmetric_key *skey);
int rb_rijndael_enc_ecb_encrypt(const unsigned char *pt, unsigned char *ct, rb_symmetric_key *skey);
void rb_rijndael_enc_done(rb_symmetric_key *skey);
int rb_rijndael_enc_keysize(int *keysize);




#endif

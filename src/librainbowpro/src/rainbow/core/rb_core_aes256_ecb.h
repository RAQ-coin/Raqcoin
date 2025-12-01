#include "rb_core_config.h"


#ifdef __cplusplus
extern "C" { 
#endif

typedef struct {
    uint8_t key[32]; 
    uint8_t enckey[32]; 
    uint8_t deckey[32];
} rb_aes256_context;


void rb_aes256_init(rb_aes256_context *, uint8_t * /* key */);
void rb_aes256_done(rb_aes256_context *);
void rb_aes256_encrypt_ecb(rb_aes256_context *, uint8_t * /* plaintext */);
void rb_aes256_decrypt_ecb(rb_aes256_context *, uint8_t * /* cipertext */);

#ifdef __cplusplus
}
#endif
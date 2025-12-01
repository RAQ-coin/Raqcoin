#ifndef rb__UTILS_HASH_H_
#define rb__UTILS_HASH_H_
#include "rb_core_config.h"
#ifdef  __cplusplus
extern  "C" {
#endif


int rb_hash_msg( unsigned char * digest, unsigned int len_digest, const unsigned char* m , unsigned int mlen, unsigned int P_HASH_LEN);


#ifdef  __cplusplus
}
#endif



#endif // rb__UTILS_HASH_H_


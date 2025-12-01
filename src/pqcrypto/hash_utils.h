#ifndef _SHA256_UTILS_H_
#define _SHA256_UTILS_H_

#include <stdio.h>
#include <stdlib.h>


/// for the definition of _HASH_LEN.
#include "hash_len_config.h"


#ifdef  __cplusplus
extern  "C" {
#endif



int sha2_file( unsigned char * digest , FILE * fp );

int sha2_chain( unsigned char * d2 , const unsigned char * d1 );

int sha2_chain_file( unsigned char * digest , unsigned n_digest , const char * f_name );

int sha2_chain_msg( unsigned char * digest , unsigned n_digest , const unsigned char * m , unsigned long long mlen );

int sha2_chain_byte( unsigned char * output_bytes , unsigned output_size , unsigned * chain_status , unsigned char * chain );



#ifdef  __cplusplus
}
#endif



#endif


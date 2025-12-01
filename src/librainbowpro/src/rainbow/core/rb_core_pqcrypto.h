#ifndef rb_PQC_PQCRYPT_H
#define rb_PQC_PQCRYPT_H



#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>


#include "rb_core_pqcrypt_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define rb_PQC_ARGCHK(x) if (!(x)) { fprintf(stderr, "\nwarning: ARGCHK failed at %s:%d\n", __FILE__, __LINE__); }
#define rb_PQC_ARGCHKVD(x) rb_PQC_ARGCHK(x)

#define rb_PQCRYPT   0x0001
#define rb_SPQCRYPT  "0.01"


#define rb_MAXBLOCKSIZE  128


#define rb_TAB_SIZE      32


enum {
   PQCRYPT_OK=0,             
   PQCRYPT_ERROR,            
   PQCRYPT_NOP,              

   PQCRYPT_INVALID_KEYSIZE,  
   PQCRYPT_INVALID_ROUNDS,   
   PQCRYPT_FAIL_TESTVECTOR,  

   PQCRYPT_BUFFER_OVERFLOW,  
   PQCRYPT_INVALID_PACKET,   

   PQCRYPT_INVALID_PRNGSIZE, 
   PQCRYPT_ERROR_READPRNG,   

   PQCRYPT_INVALID_CIPHER,   
   PQCRYPT_INVALID_HASH,     
   PQCRYPT_INVALID_PRNG,     

   PQCRYPT_MEM,              

   PQCRYPT_PK_TYPE_MISMATCH, 
   PQCRYPT_PK_NOT_PRIVATE,   

   PQCRYPT_INVALID_ARG,      
   PQCRYPT_FILE_NOTFOUND,    

   PQCRYPT_PK_INVALID_TYPE,  
   PQCRYPT_PK_INVALID_SYSTEM,
   PQCRYPT_PK_DUP,           
   PQCRYPT_PK_NOT_FOUND,     
   PQCRYPT_PK_INVALID_SIZE,  

   PQCRYPT_INVALID_PRIME_SIZE,
   PQCRYPT_PK_INVALID_PADDING,

   PQCRYPT_HASH_OVERFLOW      
};



#include "rb_core_pqcrypt_cfg.h"
#include "rb_core_pqcrypt_macros.h"
#include "rb_core_pqcrypt_cipher.h"
#include "rb_core_sha256_pqc.h"
#include "rb_core_sha512.h"
#include "rb_core_rng.h"
#include "rb_core_pqcrypt_prng.h"






#ifdef __cplusplus
   }
#endif

#endif /* PQCRYPT_H_ */



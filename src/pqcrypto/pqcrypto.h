#ifndef RAQCOIN_PQCRYPT_H
#define RAQCOIN_PQCRYPT_H

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

/* use configuration data */
#include "pqcrypt_util.h"

#ifdef __cplusplus
extern "C" {
#endif


/* version */
#define PQCRYPT   0x0001
#define SPQCRYPT  "0.01"

/* max size of either a cipher/hash block or symmetric key [largest of the two] */
#define MAXBLOCKSIZE  128

/* descriptor table size */
#define TAB_SIZE      32

/* error codes [will be expanded in future releases] */
enum {
   PQCRYPT_OK=0,             /* Result OK */
   PQCRYPT_ERROR,            /* Generic Error */
   PQCRYPT_NOP,              /* Not a failure but no operation was performed */

   PQCRYPT_INVALID_KEYSIZE,  /* Invalid key size given */
   PQCRYPT_INVALID_ROUNDS,   /* Invalid number of rounds */
   PQCRYPT_FAIL_TESTVECTOR,  /* Algorithm failed test vectors */

   PQCRYPT_BUFFER_OVERFLOW,  /* Not enough space for output */
   PQCRYPT_INVALID_PACKET,   /* Invalid input packet given */

   PQCRYPT_INVALID_PRNGSIZE, /* Invalid number of bits for a PRNG */
   PQCRYPT_ERROR_READPRNG,   /* Could not read enough from PRNG */

   PQCRYPT_INVALID_CIPHER,   /* Invalid cipher specified */
   PQCRYPT_INVALID_HASH,     /* Invalid hash specified */
   PQCRYPT_INVALID_PRNG,     /* Invalid PRNG specified */

   PQCRYPT_MEM,              /* Out of memory */

   PQCRYPT_PK_TYPE_MISMATCH, /* Not equivalent types of PK keys */
   PQCRYPT_PK_NOT_PRIVATE,   /* Requires a private PK key */

   PQCRYPT_INVALID_ARG,      /* Generic invalid argument */
   PQCRYPT_FILE_NOTFOUND,    /* File Not Found */

   PQCRYPT_PK_INVALID_TYPE,  /* Invalid type of PK key */
   PQCRYPT_PK_INVALID_SYSTEM,/* Invalid PK system specified */
   PQCRYPT_PK_DUP,           /* Duplicate key already in key ring */
   PQCRYPT_PK_NOT_FOUND,     /* Key not found in keyring */
   PQCRYPT_PK_INVALID_SIZE,  /* Invalid size input for PK parameters */

   PQCRYPT_INVALID_PRIME_SIZE,/* Invalid size of prime requested */
   PQCRYPT_PK_INVALID_PADDING, /* Invalid padding on input */

   PQCRYPT_HASH_OVERFLOW      /* Hash applied to too many bits */
};


#include "pqcrypt_cfg.h"
#include "pqcrypt_macros.h"
#include "pqcrypt_cipher.h"
//#include "rmd128.h"
//#include "rmd160.h"
//#include "rmd256.h"
//#include "rmd320.h"
#include "sha256.h"
//#include "sha224.h"
#include "sha512.h"
#include "sha384.h"
//#include "sha1.h"
#include "rng.h"
#include "aes.h"
//#include "gf_256.h"
//#include "rainbow.h"

//#include "pqcrypt_hash.h"
//#include "pqcrypt_mac.h"
#include "pqcrypt_prng.h"
//#include <pqcrypt_pk.h>
//#include <pqcrypt_math.h>
//#include "pqcrypt_misc.h"
#include "pqcrypt_argchk.h"
//#include <pqcrypt_pkcs.h>


#ifdef __cplusplus
   }
#endif

#endif /* PQCRYPT_H_ */

/* $Source$ */
/* $Revision$ */
/* $Date$ */


#include "pqcrypto.h"

/**
  @file crypt_argchk.c
*/


#if (ARGTYPE == 0)
void crypt_argchk(char *v, char *s, int d)
{
 fprintf(stderr, "PQC_ARGCHK '%s' failure on line %d of file %s\n",
         v, d, s);
 abort();
}
#endif



/* $Source$ */
/* $Revision$ */
/* $Date$ */


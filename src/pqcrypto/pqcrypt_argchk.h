/* Defines the PQC_ARGCHK macro used within the library */
/* ARGTYPE is defined in pqcrypt_cfg.h */

#ifndef RAQCOIN_PQCRYPT_ARGCHK_H
#define RAQCOIN_PQCRYPT_ARGCHK_H

#include <signal.h>



#if ARGTYPE == 0


/* this is the default PQCrypt macro  */
#if defined(__clang__) || defined(__GNUC_MINOR__)
#define NORETURN __attribute__ ((noreturn))
#else
#define NORETURN 
#endif

void crypt_argchk(char *v, char *s, int d) NORETURN;
#define PQC_ARGCHK(x) do { if (!(x)) { crypt_argchk(#x, __FILE__, __LINE__); } }while(0)
#define PQC_ARGCHKVD(x) do { if (!(x)) { crypt_argchk(#x, __FILE__, __LINE__); } }while(0)

#elif ARGTYPE == 1

/* fatal type of error */
#define PQC_ARGCHK(x) assert((x))
#define PQC_ARGCHKVD(x) PQC_ARGCHK(x)

#elif ARGTYPE == 2

#define PQC_ARGCHK(x) if (!(x)) { fprintf(stderr, "\nwarning: ARGCHK failed at %s:%d\n", __FILE__, __LINE__); }
#define PQC_ARGCHKVD(x) PQC_ARGCHK(x)

#elif ARGTYPE == 3

#define PQC_ARGCHK(x)
#define PQC_ARGCHKVD(x) PQC_ARGCHK(x)

#elif ARGTYPE == 4

#define PQC_ARGCHK(x)   if (!(x)) return PQCRYPT_INVALID_ARG;
#define PQC_ARGCHKVD(x) if (!(x)) return;

#endif



#endif


/* $Source$ */
/* $Revision$ */
/* $Date$ */


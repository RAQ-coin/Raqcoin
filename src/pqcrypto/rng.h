#ifndef RAQCOIN_PQCRYPT_RNG_H
#define RAQCOIN_PQCRYPT_RNG_H

/* Slow RNG you **might** be able to use to seed a PRNG with.  Be careful as this
 * might not work on all platforms as planned
 */

/**
  Read the system RNG
  @param out       Destination
  @param outlen    Length desired (octets)
  @param callback  Pointer to void function to act as "callback" when RNG is slow.  This can be NULL
  @return Number of octets read
*/
unsigned long getRngBytes(unsigned char *out, unsigned long outlen, void (*callback)(void));
extern unsigned long (*pqc_rng)(unsigned char *out, unsigned long outlen,
      void (*callback)(void));


#endif


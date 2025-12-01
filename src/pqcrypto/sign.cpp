
#include <stdlib.h>
#include <string.h>

#include "rainbow_config.h"
#include "rainbow_16.h"

#include "hash_utils.h"


int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
	rainbow_genkey(pk, sk);
	return 0;
}

int
crypto_sign(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk)
{
	unsigned char digest[_HASH_LEN];

	sha2_chain_msg( digest , _HASH_LEN , m , mlen );

	memcpy( sm , m , mlen );
	smlen[0] = mlen + _SIGNATURE_BYTE;
	return rainbow_sign( sm + mlen , sk , digest );
}

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,const unsigned char *sm, unsigned long long smlen,const unsigned char *pk)
{
	if( _SIGNATURE_BYTE > smlen ) return -1;
	memcpy( m , sm , smlen-_SIGNATURE_BYTE );
	mlen[0] = smlen-_SIGNATURE_BYTE;

	unsigned char digest[_HASH_LEN];
	sha2_chain_msg( digest , _HASH_LEN , m , *mlen );

	return rainbow_verify( digest , sm + mlen[0] , pk );
}



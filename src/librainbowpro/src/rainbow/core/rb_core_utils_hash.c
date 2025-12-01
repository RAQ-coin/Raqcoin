#include "rb_core_config.h"
#include "rb_core_RAINBOW_PRO_API.h"
#include "rb_core_utils_hash.h"
#include "rb_core_sha256.h"
#include "rb_core_sha512.h"
#include "rb_core_rng.h"



static inline int rb__hash( unsigned char * digest , const unsigned char * m , unsigned int mlen, unsigned int P_HASH_LEN)
{
	int ret_len = 0;
	if (P_HASH_LEN == 28)
	{
		rb_SHA512_CTX sha512;
		rb_SHA512_224_Init(&sha512);
		rb_SHA512_224_Update(&sha512, m, mlen);
		rb_SHA512_224_Final(&sha512, digest);
	
		ret_len = 28;
	}
	else
	if (P_HASH_LEN == 32)
	{
#ifdef HASH_APIs_Be_Used_IN_Inner
		unsigned long sha256_handle = 0;
		rainbow_pro_sha256_init(&sha256_handle);
		rainbow_pro_sha256_update(sha256_handle, (unsigned char *)m, (size_t)mlen);
		rainbow_pro_sha256_final(sha256_handle, digest);
#else
		rb_SHA256_CTX sha256;
		rb_SHA256Init(&sha256);
		rb_SHA256Update(&sha256, m, mlen);
		rb_SHA256Final(&sha256, digest);

#endif
		ret_len = 32;
	}
	else
		if (P_HASH_LEN == 48)
		{
#ifdef HASH_APIs_Be_Used_IN_Inner
			unsigned long sha384_handle = 0;
			rainbow_pro_sha384_init(&sha384_handle);
			rainbow_pro_sha384_update(sha384_handle, (unsigned char*)m, (size_t)mlen);
			rainbow_pro_sha384_final(sha384_handle, digest);
#else
			rb_SHA512_CTX sha384;
			rb_SHA384_Init(&sha384);
			rb_SHA384_Update(&sha384, m, mlen);
			rb_SHA384_Final(&sha384, digest);

#endif
			ret_len = 48;

		}
		else
			if (P_HASH_LEN == 64)
			{
#ifdef HASH_APIs_Be_Used_IN_Inner
				unsigned long sha512_handle = 0;
				rainbow_pro_sha512_init(&sha512_handle);
				rainbow_pro_sha512_update(sha512_handle, (unsigned char*)m, (size_t)mlen);
				rainbow_pro_sha512_final(sha512_handle, digest);
#else
				rb_SHA512_CTX sha512;
				rb_SHA512_Init(&sha512);
				rb_SHA512_Update(&sha512, m, mlen);
				rb_SHA512_Final( &sha512, digest);
#endif
				ret_len = 64;

			}
			else
			{
				rb_SHA256_CTX sha256;
				rb_SHA256Init(&sha256);
				rb_SHA256Update(&sha256, m, mlen);
				rb_SHA256Final(&sha256, digest);

				ret_len = 32;
			}

	return ret_len;
}





static inline int rb_expand_hash( unsigned char * digest , unsigned int n_digest , const unsigned char * hash, unsigned int P_HASH_LEN)
{
	if(P_HASH_LEN >= n_digest ) 
	{
		for (unsigned int i = 0; i < n_digest; i++)
		{
			digest[i] = hash[i];
		}
		return 0;
	} 
	else 
	{
		for (unsigned int i = 0; i < P_HASH_LEN; i++)
		{
			digest[i] = hash[i];
		}
		n_digest -= P_HASH_LEN;
	}

	while(P_HASH_LEN <= n_digest ) 
	{
		rb__hash( digest+ P_HASH_LEN, digest , P_HASH_LEN, P_HASH_LEN);

		n_digest -= P_HASH_LEN;
		digest += P_HASH_LEN;
	}
	unsigned char *temp=(unsigned char *)rb_safe_malloc(P_HASH_LEN);

	if( n_digest )
	{
		rb__hash( temp , digest , P_HASH_LEN, P_HASH_LEN);
		for(unsigned int i=0;i<n_digest;i++) 
			digest[P_HASH_LEN +i] = temp[i];
	}
	rb_safe_free(temp);
	return 0;
}




int rb_hash_msg( unsigned char * digest , unsigned int len_digest , const unsigned char * m , unsigned int mlen, unsigned int P_HASH_LEN)
{
	int ret = 0;
	int ret_len = 0;
	unsigned char *buf = (unsigned char *)rb_safe_malloc(P_HASH_LEN);
	
	ret_len = rb__hash( buf , m , mlen, P_HASH_LEN);

	ret= rb_expand_hash(digest, len_digest, buf, ret_len);
	rb_safe_free(buf);
	return ret;
}


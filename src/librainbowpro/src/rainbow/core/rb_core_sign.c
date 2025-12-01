#include <stdlib.h>
#include <string.h>
#include "rb_core_config.h"
#include "rb_core_rainbow_keypair.h"
#include "rb_core_rainbow.h"
#include "rb_core_api.h"
#include "rb_core_utils_hash.h"
#include "rb_core_rng.h"

extern unsigned int rainbow_pro_savebuffer_tofile(unsigned long handle, char* filename, unsigned char* buffer, unsigned int buffer_size, char* dataname, int mode, int format);
extern void printf_debuginfo(char* s, unsigned char* buf, unsigned int len);

int rb_crypto_generate_keypair(unsigned long handle,unsigned char *pk, unsigned char *sk)
{

	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
	int r = 0;

	unsigned char *sk_seed=(unsigned char *)rb_safe_malloc(HD->P_LEN_SKSEED);
	if (sk_seed == NULL)
		return -1;
	rb_randombytes(handle,sk_seed, HD->P_LEN_SKSEED);

	unsigned char* pk_seed = (unsigned char*)rb_safe_malloc(HD->P_LEN_PKSEED);
	if (pk_seed == NULL)
		return -1;
	rb_randombytes(handle,pk_seed, HD->P_LEN_PKSEED);
	if (HD->set_display_debuginfo >= 1)
	{
		printf_debuginfo("sk_seed", sk_seed, HD->P_LEN_SKSEED);
		printf_debuginfo("pk_seed", pk_seed, HD->P_LEN_SKSEED);
	}


	unsigned int sys[8];
	sys[0] = HD->set_use_gf16_gf256_par_id;
	sys[1] = HD->P_GFSIZE;
	sys[2] = HD->P_V1;
	sys[3] = HD->P_O1;
	sys[4] = HD->P_O2;
	sys[5] = HD->P_HASH_LEN;
	sys[6] = HD->set_type;
	sys[7] = HD->set_subtype;
	unsigned char total_seed_len = HD->P_LEN_SKSEED * 2 + 8 * 4;
	unsigned char *total_seed= (unsigned char*)rb_safe_malloc(total_seed_len);
	if (total_seed == NULL)
		return -1;

	unsigned char total_seed2_len = HD->P_LEN_SKSEED * 2;
	unsigned char *total_seed2 = (unsigned char*)rb_safe_malloc(total_seed2_len);
	if (total_seed2 == NULL)
		return -1;

	if (HD->set_display_debuginfo >= 1)
	{
		printf_debuginfo("sys", (unsigned char *)sys, 8 * 4);
	}

	memcpy(total_seed, sk_seed, HD->P_LEN_SKSEED);
	memcpy(total_seed+ HD->P_LEN_SKSEED, pk_seed, HD->P_LEN_SKSEED);
	memcpy(total_seed + HD->P_LEN_SKSEED*2, (unsigned char *)sys, 8*4);
	if (HD->set_display_debuginfo >= 1)
	{
		printf_debuginfo("total_seed", total_seed, total_seed_len);
	}

	rb_hash_msg(total_seed2, total_seed2_len, total_seed, total_seed_len, HD->P_HASH_LEN);

	if (HD->set_display_debuginfo >= 1)
	{
		printf_debuginfo("total_seed2", total_seed2, total_seed2_len);
	}

	memcpy(sk_seed, total_seed2, HD->P_LEN_SKSEED);
	memcpy(pk_seed, total_seed2 + HD->P_LEN_SKSEED, HD->P_LEN_SKSEED);

	if (HD->set_display_debuginfo >= 1)
	{
		printf_debuginfo("new_sk_seed", sk_seed, HD->P_LEN_SKSEED);
		printf_debuginfo("new_pk_seed", pk_seed, HD->P_LEN_SKSEED);
	}

	rb_safe_free(total_seed);
	rb_safe_free(total_seed2);


	if (HD->set_display_debuginfo >= 1)
	{
		rainbow_pro_savebuffer_tofile((unsigned long)HD, NULL, sk_seed, HD->P_LEN_SKSEED, " sk_seed", 1, 0);
		rainbow_pro_savebuffer_tofile((unsigned long)HD, NULL, pk_seed, HD->P_LEN_PKSEED, " pk_seed", 1, 0);
	}



	if (HD->set_rainbow_type_id == _RAINBOW_COMPRESSED_TYPE)
	{
		rb_cpk_t _cpk_t;
		unsigned char* p_pk_buf = pk;

		_cpk_t.pk_seed	= p_pk_buf; p_pk_buf += HD->member_len_of_cpk_t[0];			_cpk_t.pk_seed_len	= HD->member_len_of_cpk_t[0];
		_cpk_t.l1_Q3	= p_pk_buf; p_pk_buf += HD->member_len_of_cpk_t[1];			_cpk_t.l1_Q3_len	= HD->member_len_of_cpk_t[1];
		_cpk_t.l1_Q5	= p_pk_buf; p_pk_buf += HD->member_len_of_cpk_t[2];			_cpk_t.l1_Q5_len	= HD->member_len_of_cpk_t[2];
		_cpk_t.l1_Q6	= p_pk_buf; p_pk_buf += HD->member_len_of_cpk_t[3];			_cpk_t.l1_Q6_len	= HD->member_len_of_cpk_t[3];
		_cpk_t.l1_Q9	= p_pk_buf; p_pk_buf += HD->member_len_of_cpk_t[4];			_cpk_t.l1_Q9_len	= HD->member_len_of_cpk_t[4];
		_cpk_t.l2_Q9	= p_pk_buf;													_cpk_t.l2_Q9_len	= HD->member_len_of_cpk_t[5];

		rb_csk_t _csk_t;
		unsigned char* p_sk_buf = sk;

		_csk_t.pk_seed = p_sk_buf; p_sk_buf += HD->member_len_of_csk_t[0];			_csk_t.pk_seed_len	= HD->member_len_of_csk_t[0];
		_csk_t.sk_seed = p_sk_buf;													_csk_t.sk_seed_len	= HD->member_len_of_csk_t[1];
		
		r = rb_generate_compact_keypair_cyclic(handle,&_cpk_t, &_csk_t, pk_seed, sk_seed);
	}


	rb_safe_free(sk_seed);
	rb_safe_free(pk_seed);

    return r;
}





int rb_crypto_sign(unsigned long handle, unsigned char* sm, unsigned int* smlen, unsigned char* digest_hash, unsigned int digest_hash_len, unsigned char* sk)
{
	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
	int r = -1;


	if (HD->P_HASH_LEN != digest_hash_len)
		return 0;

	
	if (HD->set_rainbow_type_id == _RAINBOW_COMPRESSED_TYPE)
	{
		rb_csk_t _csk_t;
		rb_csk_t *csk0=&_csk_t;
		unsigned char* temp_sk = sk;

		csk0->pk_seed = temp_sk; temp_sk += HD->member_len_of_csk_t[0];			csk0->pk_seed_len = HD->member_len_of_csk_t[0];
		csk0->sk_seed = temp_sk;												csk0->sk_seed_len = HD->member_len_of_csk_t[1];

										
		r = rb_rainbow_sign_cyclic(handle, sm, csk0, digest_hash);

	}
	
	smlen[0] = HD->mlen + HD->P_SIGNATURE_BYTE;


	return r;
}




#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "rb_core_config.h"
#include "rb_core_rainbow.h"
#include "rb_core_utils.h"
#include "rb_core_utils_hash.h"

#include "rb_core_api.h"
#include "rb_core_RAINBOW_PRO_API.h"



extern unsigned int rainbow_pro_reset(unsigned long *handle, unsigned int  index, unsigned int set_display_debuginfo, unsigned int set_no_salt);

unsigned int rainbow_pro_verify(
	unsigned long handle, 
	unsigned int set_display_debuginfo, 
	unsigned char* m, unsigned int mlen, 
	unsigned char* sign_buf, unsigned int sign_size, 
	unsigned char* pk_buf, 
	unsigned int pk_size)
{

	unsigned int ret = 0;
	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

	HD->set_display_debuginfo = set_display_debuginfo;
	

	if ((HD->set_type == _PQC_RAINBOWPRO) && (HD->set_subtype <= RAINBOWPRO_NUM))
	{
		if (HD->P_CRYPTO_PUBLICKEYBYTES != pk_size)
			return 0;



		unsigned char* digest = rb_safe_malloc(HD->P_HASH_LEN);
		rb_hash_msg(digest, HD->P_HASH_LEN, m, mlen, HD->P_HASH_LEN);

					
		if (HD->set_rainbow_type_id == _RAINBOW_COMPRESSED_TYPE)
		{
			rb_cpk_t _cpk_t;
			unsigned char* temp_pk = pk_buf;

			_cpk_t.pk_seed = temp_pk;	temp_pk += HD->member_len_of_cpk_t[0];				_cpk_t.pk_seed_len = HD->member_len_of_cpk_t[0];
			_cpk_t.l1_Q3 = temp_pk;		temp_pk += HD->member_len_of_cpk_t[1];				_cpk_t.l1_Q3_len = HD->member_len_of_cpk_t[1];
			_cpk_t.l1_Q5 = temp_pk;		temp_pk += HD->member_len_of_cpk_t[2];				_cpk_t.l1_Q5_len = HD->member_len_of_cpk_t[2];
			_cpk_t.l1_Q6 = temp_pk;		temp_pk += HD->member_len_of_cpk_t[3];				_cpk_t.l1_Q6_len = HD->member_len_of_cpk_t[3];
			_cpk_t.l1_Q9 = temp_pk;		temp_pk += HD->member_len_of_cpk_t[4];				_cpk_t.l1_Q9_len = HD->member_len_of_cpk_t[4];
			_cpk_t.l2_Q9 = temp_pk;															_cpk_t.l2_Q9_len = HD->member_len_of_cpk_t[5];
			ret = rb_rainbow_verify_cyclic(handle, digest, sign_buf + sign_size - HD->P_SIGNATURE_BYTE, &_cpk_t);

		}

		rb_safe_free(digest);

		if (ret == 0)
			ret = 1;
		else
			ret = 0;
	}
	else
		ret = 0;

	return ret;
}
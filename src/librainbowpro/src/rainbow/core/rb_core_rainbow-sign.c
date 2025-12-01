#include <stdio.h>
#include <stdint.h>

#include "rb_core_config.h"
#include "rb_core_utils.h"
#include "rb_core_api.h"
#include "rb_core_RAINBOW_PRO_API.h"
#include "rb_core_utils_hash.h"


void printf_debuginfo(char* s, unsigned char* buf, unsigned int len)
{
	printf("%s: len:%d\n", s, len);
	for (unsigned int i = 0; i < len; i++)
	{
		printf("%02x ", buf[i]);
		if ((i + 1) % 96 == 0)
			printf("\n");
	}
	printf("\n\n");
}
unsigned int rainbowplus_get_size_inner(unsigned int config_value, unsigned int set_display_debuginfo, unsigned int* sk_size, unsigned int* pk_size, unsigned int* hash_size, unsigned int* sign_size);
unsigned int get_mix_config_size(unsigned int config_value, unsigned int set_display_debuginfo, unsigned int* sk_size, unsigned int* pk_size, unsigned int* hash_size, unsigned int* sign_size);
extern unsigned int rainbow_pro_signconfig_tobuffer(unsigned long handle);
unsigned int rainbow_pro_sign(unsigned long handle, unsigned int set_display_debuginfo, unsigned char**sm, unsigned int* smlen,  unsigned char* m, unsigned int mlen, unsigned char* sk_buf, unsigned int sk_size, unsigned char** hash_of_sign)
{
	unsigned int ret = 0;
	int r = 0;
	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
	HD->set_display_debuginfo = set_display_debuginfo;


	if ((HD->set_type == _PQC_RAINBOWPRO) && (HD->set_subtype <= RAINBOWPRO_NUM))
	{
		if (HD->set_display_debuginfo == 1)
		{
			printf("\nStart Sign------>\n");
		}
		if (HD->P_CRYPTO_SECRETKEYBYTES != sk_size)
			return 0;


		rainbow_pro_signconfig_tobuffer(handle);

		HD->mlen = mlen;
		HD->msg = m;

		unsigned int smlen_temp = 0;
		unsigned char* digest = (unsigned char*)rb_safe_malloc(HD->P_HASH_LEN);
		rb_hash_msg(digest, HD->P_HASH_LEN, m, mlen, HD->P_HASH_LEN);



		HD->signature = (unsigned char*)rb_safe_malloc((size_t)HD->mlen + HD->P_CRYPTO_BYTES);
		if (NULL == HD->signature)
		{
			printf("alloc memory for signature buffer fail.\n");
			ret = 0;
			return ret;
		}

		r = rb_crypto_sign(handle, HD->signature + mlen, &smlen_temp, digest, HD->P_HASH_LEN, sk_buf);

		memcpy(HD->signature, HD->msg, (size_t)HD->mlen);


		rb_safe_free(digest);

		if (0 != r)
		{
			printf("sign() fail.\n");
			ret = 0;
			return ret;
		}
#ifdef ONLY_SIGN
		* smlen = HD->P_SIGNATURE_BYTE;
		*sm = HD->signature + HD->mlen;
#else
		* smlen = HD->P_SIGNATURE_BYTE + HD->P_HASH_LEN;
		*sm = HD->signature;
#endif

		if (hash_of_sign)
		{
			rb_hash_msg(HD->hash_of_sig, 32, HD->signature, *smlen, 32);
			*hash_of_sign = HD->hash_of_sig;

		}
		if ((HD->set_display_debuginfo == 1) || (HD->set_display_debuginfo == 2))
		{
			rainbow_pro_savebuffer_tofile(handle, NULL, HD->msg, (unsigned int)HD->mlen, " msg", 1, 0);
			rainbow_pro_savebuffer_tofile(handle, NULL, HD->signature + HD->mlen, HD->P_CRYPTO_BYTES, " signature", 1, 0);

			if (hash_of_sign)
				rainbow_pro_savebuffer_tofile(handle, NULL, HD->hash_of_sig, 32, " hash_of_sig", 1, 0);
		}




		ret = 1;
	}
	else
		ret = 0;


		return ret;
}



unsigned int rainbowplus_get_size(unsigned int config_value, unsigned int set_display_debuginfo,  unsigned int* sk_size, unsigned int* pk_size, unsigned int* hash_size, unsigned int* sign_size)
{
	unsigned int ret = 0;
	unsigned int type = config_value >> 8;
	unsigned int subtype = config_value & 0xff;

#ifdef  CHECK_IF_THE_CHOISED_CONFIG
	unsigned int if_ret = rainbowplus_if_the_choised_configvalue(config_value);
	if ((if_ret != 1)&&(config_value!=0))
		return ret;
#endif


	if ((type == _PQC_MIX) && (subtype == 0x01))
	{
		ret = get_mix_config_size(config_value, set_display_debuginfo, sk_size, pk_size, hash_size, sign_size);
	}
	else
	{
		ret = rainbowplus_get_size_inner(config_value, set_display_debuginfo, sk_size, pk_size, hash_size, sign_size);
	}
	return ret;
}



typedef struct _sign_type_par
{
	unsigned int type;
	unsigned int subtype;
	char         config_name[64];
	char		 choised_sign_name[32];
	unsigned int sk_size;
	unsigned int pk_size;
	unsigned int hash_size;
	unsigned int sign_size;
	char		 choised_sign_discription[160];

}sign_type_par;


sign_type_par SIGN_CONFIG_RBOW[RAINBOWPRO_NUM + 1] =
{
{	0,  0,     "L1_RAINBOW_CLASSIC---------RAINBOW16_32_32_32  ",	"PQC(Rainbow_SL0.8)",		         100209,      152097,   32,      64, "PQC Sign, Using Rainbow Security Level0.8 signature algorithm."},

{	1,  1,     "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
{	1,  2,     "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
{	1,  3,     "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
										    
{	1,  4,     "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
{	1,  5,     "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
{	1,  6,     "L1_UOV_COMPRESSED----------RAINBOW256_112_44_00",	"PQC(UOV_SL1)",		                     70,	   43598,   44,     172, "PQC Sign Sign, Using UOV Security Level1 signature algorithm."},
																							    
{	1,  7,     "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  8,     "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  9,     "L3_UOV_COMPRESSED----------RAINBOW256_184_72_00",	"PQC(UOV_SL3)",		                     70,	  189254,   72,     272, "PQC Sign, Using UOV Security Level3 signature algorithm."},
																							    
{	1,  10,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  11,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  12,    "L5_UOV_COMPRESSED----------RAINBOW256_244_96_00",	"PQC(UOV_SL5)",		                     70,	  447014,   96,     356, "PQC Sign, Using UOV Security Level5 signature algorithm."},
																							    
{	1,  13,    "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
{	1,  14,    "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
{	1,  15,    "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
																							    
{	1,  16,    "-----------------------------------------------",	"----------------------------",	          0,	       0,    0,       0, "--------------------------"},
{	1,  17,    "-----------------------------------------------",	"----------------------------",	          0,	       0,    0,       0, "--------------------------"},
{	1,  18,    "L1_RAINBOW_COMPRESSED------RAINBOW16_72_8_48   ",	"PQC(Rainbow_SL1)",		                 70,	   48470,   28,      80, "PQC Sign, Using Rainbow Security Level1 signature algorithm."},
																							    
{	1,  19,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  20,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  21,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
																							    
{	1,  22,    "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
{	1,  23,    "-----------------------------------------------",	"----------------------------",		      0,	       0,    0,       0, "--------------------------"},
{	1,  24,    "L3_RAINBOW_COMPRESSED------RAINBOW256_104_8_64 ",	"PQC(Rainbow_SL3)",		                 70,	  207430,   72,     192, "PQC Sign, Using Rainbow Security Level3 signature algorithm."},
																							    
{	1,  25,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  26,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  27,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
																							    
{	1,  28,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  29,    "-----------------------------------------------",	"----------------------------",		     0,	           0,    0,       0, "--------------------------"},
{	1,  30,    "L5_RAINBOW_COMPRESSED------RAINBOW256_148_8_96 ",	"PQC(Rainbow_SL5)",		                 70,	  604358,   96,     268, "PQC Sign, Using Rainbow Security Level5 signature algorithm."},

};






sign_type_par SIGN_CONFIG_MIX[MIX_NUM + 1] =
{
{	0,   0,    "MIX_SIGN                          CONFIG	   ",   "----------------------------",             0,             0,    0,       0, "--------------------------"},
{	2,	 1,    "MIX_SIGN---------------------------------------",   "PQC(Mix:Rainbow_SL5|UOV_SL5)",           146,       1051378,  192,     624, "PQC Sign, Using Mixing signature algorithm by UOV Security Level5 signature algorithm and Rainbow Security Level5 signature algorithm."}

};




#define DEFAULT_CONFIG    4
#define PUBLIC_CONFIG_NUM 7
unsigned int PUBLIC_CONFIG[PUBLIC_CONFIG_NUM+1][2] = {
	{0, 0},
	{1, 6},
	{1, 9},
	{1, 12},
	{1, 18},
	{1, 24},
	{1, 30},
	{2,  1},
};






#define PUBLIC_MIX_CONFIG_NUM 2
unsigned int PUBLIC_MIX_CONFIG[PUBLIC_MIX_CONFIG_NUM + 1][2] = {
	{0, 0},
	{1, 12},
	{1, 30}
};





unsigned int rainbowplus_get_size_inner(unsigned int config_value, unsigned int set_display_debuginfo, unsigned int* sk_size, unsigned int* pk_size, unsigned int* hash_size, unsigned int* sign_size)
{
	unsigned int ret = 1;
	unsigned int sub_config_value = 0;
	unsigned int sub_sk_size = 0;
	unsigned int sub_pk_size = 0;
	unsigned int sub_hash_size = 0;
	unsigned int sub_sign_size = 0;

	unsigned int type = config_value>>8;
	unsigned int subtype = config_value & 0xff;

	if (type == 0)
	{
		sub_sk_size = SIGN_CONFIG_RBOW[subtype].sk_size;
		sub_pk_size = SIGN_CONFIG_RBOW[subtype].pk_size;
		sub_hash_size = SIGN_CONFIG_RBOW[subtype].hash_size;
		sub_sign_size = SIGN_CONFIG_RBOW[subtype].sign_size;

	}
	else
	if (type == _PQC_RAINBOWPRO)
	{
		sub_sk_size = SIGN_CONFIG_RBOW[subtype].sk_size;
		sub_pk_size = SIGN_CONFIG_RBOW[subtype].pk_size;
		sub_hash_size = SIGN_CONFIG_RBOW[subtype].hash_size;
		sub_sign_size = SIGN_CONFIG_RBOW[subtype].sign_size;

	}
	else
	{
		ret = 0;
	}


	if (ret != 0)
	{
		if(sk_size)
			*sk_size = sub_sk_size;
		if(pk_size)
			*pk_size = sub_pk_size;
		if(hash_size)
			*hash_size = sub_hash_size;
		if(sign_size)
			*sign_size = sub_sign_size;
	}

	return ret;
}
unsigned int get_config_value(unsigned int type, unsigned int subtype)
{
	return type << 8 | subtype;
}

unsigned int get_mix_config_size(unsigned int config_value, unsigned int set_display_debuginfo, unsigned int* sk_size, unsigned int* pk_size, unsigned int* hash_size, unsigned int* sign_size)
{
	unsigned int ret = 1;
	unsigned int ret2 = 0;
	unsigned int sub_config_value = 0;
	unsigned int sub_sk_size = 0;
	unsigned int sub_pk_size = 0;
	unsigned int sub_hash_size = 0;
	unsigned int sub_sign_size = 0;

	unsigned int t_sk_size = 0;
	unsigned int t_pk_size = 0;
	unsigned int t_hash_size = 0;
	unsigned int t_sign_size = 0;

	for (int i = 1; i <= PUBLIC_MIX_CONFIG_NUM; i++)
	{
		unsigned int type = PUBLIC_MIX_CONFIG[i][0];
		unsigned int subtype= PUBLIC_MIX_CONFIG[i][1];
		sub_config_value = get_config_value(type,subtype);

		ret2 += rainbowplus_get_size_inner(sub_config_value, set_display_debuginfo, &sub_sk_size, &sub_pk_size, &sub_hash_size, &sub_sign_size);

		if (set_display_debuginfo)
		{
			printf("config_value:%04x, type: %02x, subtype: %02x, sk_size:%8d, pk_size:%8d, hash_size:%8d, sign_size:%8d\n",
				sub_config_value,
				sub_config_value >> 8,
				sub_config_value & 0xff,
				sub_sk_size,
				sub_pk_size,
				sub_hash_size,
				sub_sign_size);
		}

		t_sk_size += sub_sk_size;
		t_pk_size += sub_pk_size;
		t_hash_size += sub_hash_size;
		t_sign_size += sub_sign_size;

	}

	if (ret2 == PUBLIC_MIX_CONFIG_NUM)
		ret = 1;
	else
		ret = 0;

	if(sk_size)
		*sk_size = t_sk_size+6;
	if(pk_size)
		*pk_size = t_pk_size+6;
	if(hash_size)
		*hash_size = t_hash_size;
	if(sign_size)
		*sign_size = t_sign_size;

	return ret;
}
unsigned int rainbowplus_get_number_of_choised_total_config(unsigned int* choised_total_config_num)
{
	unsigned int ret = 0;
	if (choised_total_config_num)
	{
		*choised_total_config_num = PUBLIC_CONFIG_NUM;
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	return ret;
}
unsigned int rainbowplus_get_choised_config_value_list(
	unsigned int* choised_config_value_list,
	unsigned int  choised_total_config_num)
{
	unsigned int ret = 1;
	unsigned int num = 0;
	unsigned int type = 0;
	unsigned int subtype = 0;
	unsigned int config_value = 0;
	rainbowplus_get_number_of_choised_total_config(&num);

	if (choised_config_value_list)
	{
		if (num == choised_total_config_num)
		{
			for (unsigned int index = 0; index < choised_total_config_num; index++)
			{
				type = PUBLIC_CONFIG[index + 1][0];
				subtype = PUBLIC_CONFIG[index + 1][1];

				config_value = get_config_value(type, subtype);

				choised_config_value_list[index] = config_value;

			}
		}
		else
			ret = 0;
	}
	else
	{
		ret = 0;
	}

	return ret;
}

unsigned int get_config_names_inner(unsigned int type, unsigned int subtype, char* config_name, char* choised_sign_name, char* choised_sign_discription)
{
	unsigned int ret = 1;
	if (config_name)
	{
		memset(config_name, 0, sizeof(SIGN_CONFIG_RBOW[subtype].config_name));

		if (type == 0)
			strcpy(config_name, SIGN_CONFIG_RBOW[subtype].config_name);
		else
			if (type == _PQC_RAINBOWPRO)
				strcpy(config_name, SIGN_CONFIG_RBOW[subtype].config_name);
							else
								if (type == _PQC_MIX)
									strcpy(config_name, SIGN_CONFIG_MIX[subtype].config_name);
								else
									strcpy(config_name, "error !!!");
	}
	if (choised_sign_name)
	{
		memset(choised_sign_name, 0, sizeof(SIGN_CONFIG_RBOW[subtype].choised_sign_name));

		if (type == 0)
			strcpy(choised_sign_name, SIGN_CONFIG_RBOW[subtype].choised_sign_name);
		else
			if (type == _PQC_RAINBOWPRO)
				strcpy(choised_sign_name, SIGN_CONFIG_RBOW[subtype].choised_sign_name);
							else
								if (type == _PQC_MIX)
									strcpy(choised_sign_name, SIGN_CONFIG_MIX[subtype].choised_sign_name);
								else
									strcpy(choised_sign_name, "error !!!");
	}

	if (choised_sign_discription)
	{
		memset(choised_sign_discription, 0, sizeof(SIGN_CONFIG_RBOW[subtype].choised_sign_discription));

		if (type == 0)
			strcpy(choised_sign_discription, SIGN_CONFIG_RBOW[subtype].choised_sign_discription);
		else
			if (type == _PQC_RAINBOWPRO)
				strcpy(choised_sign_discription, SIGN_CONFIG_RBOW[subtype].choised_sign_discription);
							else
								if (type == _PQC_MIX)
									strcpy(choised_sign_discription, SIGN_CONFIG_MIX[subtype].choised_sign_discription);
								else
									strcpy(choised_sign_discription, "error !!!");
	}

	return ret;
}

unsigned int rainbowplus_get_choised_config(unsigned int choised_index, unsigned int* out_config_value, unsigned int* out_type, unsigned int* out_subtype, char* config_name, char* choised_sign_name, char* choised_sign_discription)
{
	unsigned int ret = 1;
	unsigned int config_value = 0;
	unsigned int type = 0;
	unsigned int subtype = 0; 

	type = PUBLIC_CONFIG[choised_index][0];
	subtype = PUBLIC_CONFIG[choised_index][1];

	config_value = get_config_value(type, subtype);

	if (out_config_value)
		*out_config_value = config_value;
	if(out_type)
		*out_type = type;
	if(out_subtype)
		*out_subtype = subtype;


	get_config_names_inner(type, subtype, config_name, choised_sign_name, choised_sign_discription);


	return ret;
}
unsigned int rainbowplus_get_choised_config_from_config_value(unsigned int config_value, unsigned int* out_type, unsigned int* out_subtype, char* config_name, char* choised_sign_name, char* choised_sign_discription)
{
	unsigned int ret = 1;

#ifdef  CHECK_IF_THE_CHOISED_CONFIG
	unsigned int if_ret = rainbowplus_if_the_choised_configvalue(config_value);
	if( (if_ret != 1) && (config_value!=0) )
		return ret;
#endif

	unsigned int type = config_value >> 8;
	unsigned int subtype = config_value & 0xff;

	*out_type = type;
	*out_subtype = subtype;

	
	get_config_names_inner(type, subtype, config_name, choised_sign_name, choised_sign_discription);

	return ret;
}
void get_max_min_of_array(unsigned int* a, unsigned int a_len, unsigned int* max, unsigned int* min)
{
	unsigned int imaxp = 0;
	unsigned int iminp = 0;
	unsigned int imin = a[0];
	unsigned int imax = a[0];

	for (unsigned int i = 1; i < a_len; i++)
	{

		if (a[i] > imax)
		{
			imax = a[i];
			imaxp = i;
		}
		if (a[i] < imin)
		{
			imin = a[i];
			iminp = i;
		}

	}
	if(max)
		*max = imax;
	if(min)
		*min = imin;
	
}
unsigned int rainbowplus_choised_help()
{

	unsigned ret = 1;
	unsigned int sksize=0;
	unsigned int pksize=0;
	unsigned int hashsize=0;
	unsigned int signsize=0;



	for (unsigned int i = 1; i <= PUBLIC_CONFIG_NUM; i++)
	{
		unsigned int type = 0;
		unsigned int subtype = 0;
		type = PUBLIC_CONFIG[i][0];
		subtype = PUBLIC_CONFIG[i][1];

		if (type == _PQC_RAINBOWPRO)
			printf("List:%2d, type:%2d, index:%2d, par: %s,sk_size:%7d, pk_size:%7d, hash_size:%7d, sign_size:%7d \n", i, SIGN_CONFIG_RBOW[subtype].type, SIGN_CONFIG_RBOW[subtype].subtype, SIGN_CONFIG_RBOW[subtype].config_name, SIGN_CONFIG_RBOW[subtype].sk_size, SIGN_CONFIG_RBOW[subtype].pk_size, SIGN_CONFIG_RBOW[subtype].hash_size, SIGN_CONFIG_RBOW[subtype].sign_size);
		else
		if (type == _PQC_MIX)
		{
			unsigned int sk_size = 0;
			unsigned int pk_size = 0;
			unsigned int hash_size = 0;
			unsigned int sign_size = 0;
			unsigned int ret2 = get_mix_config_size(get_config_value(type, subtype), 0, &sk_size, &pk_size, &hash_size, &sign_size);
			printf("List:%2d, type:%2d, subtype:%2d, par: %s,sk_size:%7d, pk_size:%7d, hash_size:%7d, sign_size:%7d \n",
				i,type, subtype, SIGN_CONFIG_MIX[subtype].config_name,
				sk_size,
				pk_size,
				hash_size,
				sign_size);
		}
		else
			ret = 0;

	}
		
	return ret;
	
}

unsigned int rainbowplus_get_choised_info(
	unsigned int* max_byte_of_sk,
	unsigned int* min_byte_of_sk,
	unsigned int* max_byte_of_pk,
	unsigned int* min_byte_of_pk,
	unsigned int* max_byte_of_hash,
	unsigned int* min_byte_of_hash,
	unsigned int* max_byte_of_sign,
	unsigned int* min_byte_of_sign,
	unsigned int* default_choised_index,
	unsigned int* default_config_value,
	unsigned int* default_type,
	unsigned int* default_subtype
)
{
	unsigned ret = 1;
	unsigned int sksize[PUBLIC_CONFIG_NUM];
	unsigned int pksize[PUBLIC_CONFIG_NUM];
	unsigned int hashsize[PUBLIC_CONFIG_NUM];
	unsigned int signsize[PUBLIC_CONFIG_NUM];


		for(unsigned int i=1; i<= PUBLIC_CONFIG_NUM; i++)
		{
			unsigned int type = 0;
			unsigned int subtype = 0;
			type = PUBLIC_CONFIG[i][0]; 
			subtype = PUBLIC_CONFIG[i][1];

			if (type == _PQC_RAINBOWPRO)
			{
				sksize[i - 1] = SIGN_CONFIG_RBOW[subtype].sk_size;
				pksize[i - 1] = SIGN_CONFIG_RBOW[subtype].pk_size;
				hashsize[i - 1] = SIGN_CONFIG_RBOW[subtype].hash_size;
				signsize[i - 1] = SIGN_CONFIG_RBOW[subtype].sign_size;

			}
			else
			if (type == _PQC_MIX)
			{
				unsigned int ret2 = get_mix_config_size(get_config_value(type, subtype), 0, &sksize[i - 1], &pksize[i - 1], &hashsize[i - 1], &signsize[i - 1]);
			}
			else
				ret = 0;

		}
		get_max_min_of_array(sksize, PUBLIC_CONFIG_NUM, max_byte_of_sk, min_byte_of_sk);
		get_max_min_of_array(pksize, PUBLIC_CONFIG_NUM, max_byte_of_pk, min_byte_of_pk);
		get_max_min_of_array(hashsize, PUBLIC_CONFIG_NUM, max_byte_of_hash, min_byte_of_hash);
		get_max_min_of_array(signsize, PUBLIC_CONFIG_NUM, max_byte_of_sign, min_byte_of_sign);
		if(default_choised_index)
			*default_choised_index = DEFAULT_CONFIG;
		rainbowplus_get_choised_config(DEFAULT_CONFIG, default_config_value,default_type, default_subtype, NULL,NULL,NULL);

		return ret;
}

unsigned int rainbowplus_choised_check_len(unsigned int type,unsigned int len)
{
	unsigned ret = 0;
	unsigned int sksize[PUBLIC_CONFIG_NUM] = { 0 };
	unsigned int pksize[PUBLIC_CONFIG_NUM] = { 0 };
	unsigned int signsize[PUBLIC_CONFIG_NUM] = { 0 };
	unsigned int hashsize[PUBLIC_CONFIG_NUM] = { 0 };

	if (type == 0)
	{
		for (unsigned int i = 1; i <= PUBLIC_CONFIG_NUM; i++)
		{
			unsigned int type = 0;
			unsigned int subtype = 0;

			type = PUBLIC_CONFIG[i][0];
			subtype = PUBLIC_CONFIG[i][1];

			if (type == _PQC_RAINBOWPRO)
			{
				sksize[i - 1] = SIGN_CONFIG_RBOW[subtype].sk_size;
				pksize[i - 1] = SIGN_CONFIG_RBOW[subtype].pk_size;
				signsize[i - 1] = SIGN_CONFIG_RBOW[subtype].sign_size;
				hashsize[i - 1] = SIGN_CONFIG_RBOW[subtype].hash_size;
			}
			else
			if (type == _PQC_MIX)
			{

				unsigned int ret2 = get_mix_config_size(get_config_value(type, subtype), 0, &sksize[i - 1], &pksize[i - 1], &hashsize[i - 1], &signsize[i - 1]);

			}
			else
				ret = 0;

		}

		for (int j = 0; j < PUBLIC_CONFIG_NUM; j++)
		{
			if (sksize[j] == len)
			{
				ret = 1;
				break;
			}
			if (pksize[j] == len)
			{
				ret = 2;
				break;
			}
			if (signsize[j] == len)
			{
				ret = 3;
				break;
			}
			if (hashsize[j] == len)
			{
				ret = 4;
				break;
			}
		}
	}
	else
		if (type == 1)
		{
			for (unsigned int i = 1; i <= PUBLIC_CONFIG_NUM; i++)
			{
				unsigned int type = 0;
				unsigned int subtype = 0;

				type = PUBLIC_CONFIG[i][0];
				subtype = PUBLIC_CONFIG[i][1];

				if (type == _PQC_RAINBOWPRO)
				{
					sksize[i - 1] = SIGN_CONFIG_RBOW[subtype].sk_size;
				}
				else
				if (type == _PQC_MIX)
				{
					unsigned int ret2 = get_mix_config_size(get_config_value(type, subtype), 0, &sksize[i - 1], NULL,NULL, NULL);

				}
				else
					ret = 0;

			}

			for (int j = 0; j < PUBLIC_CONFIG_NUM; j++)
			{
				if (sksize[j] == len)
				{
					ret = 1;
					break;
				}
			}
		}
		else
			if (type == 2)
			{
				for (unsigned int i = 1; i <= PUBLIC_CONFIG_NUM; i++)
				{
					unsigned int type = 0;
					unsigned int subtype = 0;

					type = PUBLIC_CONFIG[i][0];
					subtype = PUBLIC_CONFIG[i][1];

					if (type == _PQC_RAINBOWPRO)
					{
						pksize[i - 1] = SIGN_CONFIG_RBOW[subtype].pk_size;
					}
					else
						if (type == _PQC_MIX)
						{
							unsigned int ret2 = get_mix_config_size(get_config_value(type, subtype), 0, NULL, &pksize[i - 1], NULL, NULL);

						}
						else
							ret = 0;
				}


				for (int j = 0; j < PUBLIC_CONFIG_NUM; j++)
				{
					if (pksize[j] == len)
					{
						ret = 2;
						break;
					}
				}
			}
			else
			if (type == 3)
			{
				for (unsigned int i = 1; i <= PUBLIC_CONFIG_NUM; i++)
				{
					unsigned int type = 0;
					unsigned int subtype = 0;

					type = PUBLIC_CONFIG[i][0];
					subtype = PUBLIC_CONFIG[i][1];

					if (type == _PQC_RAINBOWPRO)
					{
						signsize[i - 1] = SIGN_CONFIG_RBOW[subtype].sign_size;
					}
					else
					if (type == _PQC_MIX)
					{
						unsigned int ret2 = get_mix_config_size(get_config_value(type, subtype), 0, NULL,NULL, NULL, &signsize[i - 1]);

					}
					else
						ret = 0;

				}

				for (int j = 0; j < PUBLIC_CONFIG_NUM; j++)
				{
					if (signsize[j] == len)
					{
						ret = 3;
						break;
					}
				}
			}
			else
			if (type == 4)
			{
				for (unsigned int i = 1; i <= PUBLIC_CONFIG_NUM; i++)
				{
					unsigned int type = 0;
					unsigned int subtype = 0;

					type = PUBLIC_CONFIG[i][0];
					subtype = PUBLIC_CONFIG[i][1];

					if (type == _PQC_RAINBOWPRO)
					{
						hashsize[i - 1] = SIGN_CONFIG_RBOW[subtype].hash_size;
					}
					else
						if (type == _PQC_MIX)
						{
							unsigned int ret2 = get_mix_config_size(get_config_value(type,subtype), 0, NULL, NULL, &hashsize[i - 1], NULL);

						}
						else
							ret = 0;
				}


				for (int j = 0; j < PUBLIC_CONFIG_NUM; j++)
				{
					if (hashsize[j] == len)
					{
						ret = 4;
						break;
					}
				}
			}
	return ret;
}

unsigned int rainbowplus_get_number_of_type(unsigned int* typenum)
{
	unsigned int ret = 0;
	if(typenum!=NULL)
	{
		*typenum = 2;
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	
	return ret;
}
unsigned int rainbowplus_get_number_of_subtype(unsigned int* subtypenum)
{
	unsigned int ret = 0;
	if (subtypenum != NULL)
	{
		unsigned int num_of_rainbow = sizeof(SIGN_CONFIG_RBOW) / sizeof(sign_type_par) - 1;
		unsigned int num_of_mix = sizeof(SIGN_CONFIG_MIX) / sizeof(sign_type_par) - 1;
		

		*subtypenum = num_of_rainbow+ num_of_mix;;
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	return ret;
}
unsigned int rainbowplus_get_config_value_list(unsigned int *config_value_list, unsigned int total_subtypenum)
{
	unsigned int ret = 1;
	unsigned int num = 0;
	unsigned int type = 0;
	unsigned int typenum = 0;
	unsigned int subtype = 0;
	unsigned int subtypenum = 0;
	unsigned int time_index = 0;

	if (config_value_list != NULL)
	{
		rainbowplus_get_number_of_subtype(&num);
		if (num == total_subtypenum)
		{
			rainbowplus_get_number_of_type(&typenum);

			for (type = 1; type < typenum + 1; type++)
			{
				rainbowplus_get_subtype_number_of_onetype(type, &subtypenum);
				for (subtype = 1; subtype <= subtypenum; subtype++)
				{
					if (time_index == num)
						break;
					config_value_list[time_index] = get_config_value(type, subtype);
					printf("index:%d, valuse:0x%08x\n", time_index, config_value_list[time_index]);
					time_index += 1;
				}
			}
		}
		else
			ret = 0;
	}
	else
	{
		ret = 0;
	}

	return ret;
}
unsigned int rainbowplus_get_info_of_config_value(
	unsigned int  config_value,
	unsigned int  *i_type,
	unsigned int  *i_subtype,
	char * config_name)
{
	unsigned int ret = 1;
	unsigned int type = 0;
	unsigned int subtype = 0;
	char choised_sign_name[32];
	char choised_sign_discription[160];

	type = config_value >> 8;
	subtype = config_value & 0xff;
	if (i_type)
		*i_type = type;
	if (i_subtype)
		*i_subtype = subtype;


	get_config_names_inner(type, subtype, config_name, choised_sign_name, choised_sign_discription);

	return ret;

}
#ifdef RB_SIMD_X86
extern int rb_get_cpu_support();
#endif


unsigned int rainbowplus_help()
{
	unsigned int sk_size=0;
	unsigned int pk_size=0;
	unsigned int hash_size=0;
	unsigned int sign_size=0;

	unsigned int index_inner=0;
	unsigned int ret = 1;
	unsigned int typenum = 0;
	unsigned int subtypenum = 0;
	rainbowplus_get_number_of_type(&typenum);

#if defined(WIN32_VERSION) || defined(LINUX_VERSION_X86)
#ifdef RB_SIMD_X86
	rb_get_cpu_support();
#endif
#endif



	for (unsigned int type = 1; type <= typenum; type++)
	{
		rainbowplus_get_subtype_number_of_onetype(type, &subtypenum);

		for (unsigned int subtype = 1; subtype <= subtypenum; subtype++)
		{
			index_inner += 1;
			if (type == _PQC_RAINBOWPRO)
			printf("List:%2d, type:%2d, subtype:%2d, par: %s,sk_size:%7d, pk_size:%7d, hash_size:%7d, sign_size:%7d \n", index_inner, SIGN_CONFIG_RBOW[subtype].type, SIGN_CONFIG_RBOW[subtype].subtype, SIGN_CONFIG_RBOW[subtype].config_name, SIGN_CONFIG_RBOW[subtype].sk_size, SIGN_CONFIG_RBOW[subtype].pk_size, SIGN_CONFIG_RBOW[subtype].hash_size, SIGN_CONFIG_RBOW[subtype].sign_size);
			else
			if (type == _PQC_MIX)
			{
				unsigned int sk_size = 0;
				unsigned int pk_size = 0;
				unsigned int hash_size = 0;
				unsigned int sign_size = 0;
				unsigned int ret2 = get_mix_config_size(get_config_value(type, subtype), 0, &sk_size, &pk_size, &hash_size, &sign_size);
				printf("List:%2d, type:%2d, subtype:%2d, par: %s,sk_size:%7d, pk_size:%7d, hash_size:%7d, sign_size:%7d \n",
					index_inner, type, subtype, SIGN_CONFIG_MIX[subtype].config_name,
					sk_size,
					pk_size,
					hash_size,
					sign_size);
			}
			else
			ret = 0;
		}
					
	}




	return ret;

}


unsigned int rainbowplus_get_keypair_inner(
	unsigned int config_value,
	unsigned int set_display_debuginfo,
	unsigned char* seed,
	unsigned int seed_len,
	unsigned char* sk_buf,
	unsigned int sk_size,
	unsigned char* pk_buf,
	unsigned int pk_size,
	unsigned char* Hash_of_Private_Key,
	unsigned char* Hash_of_Public_Key)
{
	unsigned long handle = 0;
	unsigned int ret = 0;

	unsigned char* Private_Key = NULL;
	unsigned char* Public_Key = NULL;
	unsigned int    Private_Key_Len = 0;
	unsigned int    Public_Key_Len = 0;

	unsigned int type = config_value >> 8;
	unsigned int subtype = config_value & 0xff;

	ret += rainbow_pro_init(&handle, type, subtype, set_display_debuginfo, 0, NULL, NULL, NULL, NULL);
	ret += rainbow_pro_genkey(
		handle,
		set_display_debuginfo,
		seed, seed_len,
		&Private_Key, &Private_Key_Len,
		&Public_Key, &Public_Key_Len,
		NULL,
		NULL);
	if ((sk_size == Private_Key_Len + RAINBOWNPRO_HEADER_LEN) && (pk_size == Public_Key_Len + RAINBOWNPRO_HEADER_LEN))
	{
		memcpy(sk_buf + RAINBOWNPRO_HEADER_LEN, Private_Key, sk_size - RAINBOWNPRO_HEADER_LEN);
		*(sk_buf + 0) = 0xFF;
		*(sk_buf + 1) = 0xFF;
		*(sk_buf + 2) = 0xFF;
		*(sk_buf + 3) = 0xFF;
		*(sk_buf + 4) = type;
		*(sk_buf + 5) = subtype;
		memcpy(pk_buf + RAINBOWNPRO_HEADER_LEN, Public_Key, pk_size - RAINBOWNPRO_HEADER_LEN);
		*(pk_buf + 0) = 0xFF;
		*(pk_buf + 2) = 0xFF;
		*(pk_buf + 3) = 0xFF;
		*(pk_buf + 1) = 0xFF;
		*(pk_buf + 4) = type;
		*(pk_buf + 5) = subtype;
	}
	else
		ret = 0;

	if (Hash_of_Private_Key)
	{
		rb_hash_msg(Hash_of_Private_Key, 32, sk_buf, sk_size, 32);
	}

	if (Hash_of_Public_Key)
	{
		rb_hash_msg(Hash_of_Public_Key, 32, pk_buf, pk_size, 32);
	}

	if (set_display_debuginfo >= 1)
	{
		if (set_display_debuginfo == 2)
		{
			printf_debuginfo("sk_buf", sk_buf, sk_size);
			printf_debuginfo("pk_buf", pk_buf, pk_size);
		}
		if(Hash_of_Private_Key)
			printf_debuginfo("Hash_of_Private_Key", Hash_of_Private_Key, 32);
		if(Hash_of_Public_Key)
			printf_debuginfo("Hash_of_Public_Key", Hash_of_Public_Key, 32);
	}

	ret += rainbow_pro_uninit(handle);

	if (ret == 3)
		return 1;
	else
		return 0;
}

unsigned int rainbowplus_get_keypair(
	unsigned int config_value,
	unsigned int set_display_debuginfo,
	unsigned char* seed, 
	unsigned int seed_len, 
	unsigned char* sk_buf, 
	unsigned int sk_size, 
	unsigned char* pk_buf, 
	unsigned int pk_size, 
	unsigned char* Hash_of_Private_Key, 
	unsigned char* Hash_of_Public_Key)
{
	unsigned int ret = 0;
	unsigned int ret2 = 0;
	unsigned int type = config_value >> 8;
	unsigned int subtype = config_value & 0xff;

	if ((seed == NULL) || (sk_buf == NULL) || (pk_buf == NULL))
	{
		return ret;
	}

#ifdef  CHECK_IF_THE_CHOISED_CONFIG
	unsigned int if_ret = rainbowplus_if_the_choised_configvalue(config_value);
	if (if_ret != 1)
		return ret;
#endif

	if ((type == _PQC_MIX) && (subtype == 0x01))
	{
		unsigned int sub_config_value=0;
		unsigned int sub_sk_size = 0;
		unsigned int sub_pk_size = 0;
		unsigned int sub_hash_size = 0;
		unsigned int sub_sign_size = 0;
		unsigned char* sk_buf_temp = sk_buf+6;
		unsigned char* pk_buf_temp = pk_buf+6;

		sk_buf[0] = 0xff;
		sk_buf[1] = 0xff;
		sk_buf[2] = 0xff;
		sk_buf[3] = 0xff;
		sk_buf[4] = type;
		sk_buf[5] = subtype;

		pk_buf[0] = 0xff;
		pk_buf[1] = 0xff;
		pk_buf[2] = 0xff;
		pk_buf[3] = 0xff;
		pk_buf[4] = type;
		pk_buf[5] = subtype;
		for (int i = 1; i <= PUBLIC_MIX_CONFIG_NUM; i++)
		{
			unsigned int type = PUBLIC_MIX_CONFIG[i][0];
			unsigned int subtype = PUBLIC_MIX_CONFIG[i][1];
			sub_config_value = get_config_value(type, subtype);
			ret2 += rainbowplus_get_size_inner(sub_config_value, set_display_debuginfo, &sub_sk_size, &sub_pk_size, &sub_hash_size, &sub_sign_size);

			ret2 += rainbowplus_get_keypair_inner(
				sub_config_value,
				set_display_debuginfo,
				seed,
				seed_len,
				sk_buf_temp,
				sub_sk_size,
				pk_buf_temp,
				sub_pk_size,
				NULL,
				NULL);
			sk_buf_temp += sub_sk_size;
			pk_buf_temp += sub_pk_size;
		}

		if (Hash_of_Private_Key)
		{
			rb_hash_msg(Hash_of_Private_Key, 32, sk_buf, sk_size, 32);
		}

		if (Hash_of_Public_Key)
		{
			rb_hash_msg(Hash_of_Public_Key, 32, pk_buf, pk_size, 32);
		}


		if (set_display_debuginfo >= 1)
		{
			if (set_display_debuginfo == 2)
			{
				printf_debuginfo("mix_sk_buf", sk_buf, sk_size);
				printf_debuginfo("mix_pk_buf", pk_buf, pk_size);
			}
			if (Hash_of_Private_Key)
				printf_debuginfo("mix_Hash_of_Private_Key", Hash_of_Private_Key, 32);
			if (Hash_of_Public_Key)
				printf_debuginfo("mix_Hash_of_Public_Key", Hash_of_Public_Key, 32);
		}


		if (ret2 == PUBLIC_MIX_CONFIG_NUM*2)
			ret = 1;
		else
			ret = 0;

	}
	else
	{
		ret = rainbowplus_get_keypair_inner(
			config_value,
			set_display_debuginfo,
			seed,
			seed_len,
			sk_buf,
			sk_size,
			pk_buf,
			pk_size,
			Hash_of_Private_Key,
			Hash_of_Public_Key);
	}
	return ret;

}

unsigned int rainbowplus_hash_inner(unsigned int config_value, unsigned int set_display_debuginfo, unsigned char* m, unsigned int mlen, unsigned char* hash_buf, unsigned int hash_size)
{
	unsigned long handle = 0;
	unsigned int ret = 0;

	unsigned char* digest_hash = NULL;
	unsigned int digest_hash_len = 0;

	unsigned int type = config_value >> 8;
	unsigned int subtype = config_value & 0xff;

	ret += rainbow_pro_init(&handle, type, subtype, set_display_debuginfo, 0, NULL, NULL, NULL, NULL);
	ret += rainbow_pro_digest_hash(handle, set_display_debuginfo, m, mlen, &digest_hash, &digest_hash_len);

	if (hash_size == digest_hash_len)
	{
		memcpy(hash_buf, digest_hash, hash_size);
	}
	else
		ret = 0;

	ret += rainbow_pro_uninit(handle);

	if (set_display_debuginfo >= 1)
	{
			printf_debuginfo("hash_buf", hash_buf, hash_size);
	}

	if (ret == 3)
		return 1;
	else
		return 0;
}

unsigned int rainbowplus_hash(unsigned int config_value, unsigned int set_display_debuginfo, unsigned char* m, unsigned int mlen, unsigned char* hash_buf, unsigned int hash_size)
{
	unsigned int ret = 0;
	unsigned int ret2 = 0;
	unsigned int type = config_value >> 8;
	unsigned int subtype = config_value & 0xff;

	if ((m == NULL) || (hash_buf == NULL))
	{
		return ret;
	}
#ifdef  CHECK_IF_THE_CHOISED_CONFIG
	unsigned int if_ret = rainbowplus_if_the_choised_configvalue(config_value);
	if (if_ret != 1)
		return ret;
#endif

	if ((type == _PQC_MIX) && (subtype == 0x01))
	{
		unsigned int sub_config_value = 0;
		unsigned int sub_sk_size = 0;
		unsigned int sub_pk_size = 0;
		unsigned int sub_hash_size = 0;
		unsigned int sub_sign_size = 0;
		unsigned char* hash_buf_temp = hash_buf;
		for (int i = 1; i <= PUBLIC_MIX_CONFIG_NUM; i++)
		{
			unsigned int type = PUBLIC_MIX_CONFIG[i][0];
			unsigned int subtype = PUBLIC_MIX_CONFIG[i][1];
			sub_config_value = get_config_value(type, subtype);
			ret2 += rainbowplus_get_size_inner(sub_config_value, set_display_debuginfo, &sub_sk_size, &sub_pk_size, &sub_hash_size, &sub_sign_size);

			ret2 += rainbowplus_hash_inner(sub_config_value, set_display_debuginfo, m, mlen, hash_buf_temp, sub_hash_size);

			hash_buf_temp += sub_hash_size;

		}


		if (set_display_debuginfo >= 1)
		{
			printf_debuginfo("mix_hash_buf", hash_buf, hash_size);
		}

		if (ret2 == PUBLIC_MIX_CONFIG_NUM*2)
			ret = 1;
		else
			ret = 0;

	}
	else
	{
		ret = rainbowplus_hash_inner(config_value, set_display_debuginfo, m, mlen, hash_buf, hash_size);

	}
	return ret;
}

unsigned int rainbowplus_get_config(unsigned char* skpk_buf,  unsigned int* config_value, unsigned int* type, unsigned int* subtype, char* config_name, char* choised_sign_name, char* choised_sign_discription)
{
	unsigned int ret = 0;
	unsigned char a1, a2, a3, a4, type_inner, subtype_inner;

	if (skpk_buf)
	{
		a1 = *(skpk_buf + 0);
		a2 = *(skpk_buf + 1);
		a3 = *(skpk_buf + 2);
		a4 = *(skpk_buf + 3);
		type_inner = *(skpk_buf + 4);
		subtype_inner = *(skpk_buf + 5);

		if ((a1 == 0xff) && (a2 == 0xff) && (a3 == 0xff) && (a4 == 0xff))
		{
			ret = 1;
	
							if ((type_inner == _PQC_RAINBOWPRO) && (subtype_inner <= RAINBOWPRO_NUM))
							{
								if (type)
									*type = type_inner;
								if (subtype)
									*subtype = subtype_inner;
								if (config_value)
									*config_value = (unsigned char)type_inner << 8 | (unsigned char)subtype_inner;
								ret = 1;
							}
							else
							if ((type_inner == _PQC_MIX) && (subtype_inner <= MIX_NUM))
							{
								if (type)
									*type = type_inner;
								if (subtype)
									*subtype = subtype_inner;
								if (config_value)
									*config_value = (unsigned char)type_inner << 8 | (unsigned char)subtype_inner;
								ret = 1;
							}
							else
							{

								if (type)
									*type = 0;
								if (subtype)
									*subtype = 0;
								if (config_value)
									*config_value = 0;

								ret = 0;
							}
		}
		else
		{

			if (type)
				*type = 0;
			if (subtype)
				*subtype = 0;
			if (config_value)
				*config_value = 0;

			ret = 0;
		}
	}
	else
	{
		ret = 0;
	}

	if (ret != 0)
	{
		unsigned char type = type_inner;
		unsigned char subtype = subtype_inner;
		get_config_names_inner(type, subtype, config_name, choised_sign_name, choised_sign_discription);
	}
	return  ret;
}

unsigned int rainbowplus_if_the_choised_configvalue(unsigned int config_value)
{
	unsigned int ret = 0;
	unsigned int config_value_temp = 0;
	unsigned int type = 0;
	unsigned int subtype = 0;

	for(int i = 1; i <= PUBLIC_CONFIG_NUM; i++)
	{
		type = PUBLIC_CONFIG[i][0];
		subtype = PUBLIC_CONFIG[i][1];
		config_value_temp = get_config_value(type, subtype);
		if (config_value_temp == config_value)
		{
			ret = 1;
			break;
		}
	}
	return ret;
}

unsigned int rainbowplus_get_subtype_number_of_onetype(unsigned int type, unsigned int* subtypenum)
{
	unsigned int ret = 0;

	if (subtypenum != NULL)
	{
		if (type == _PQC_RAINBOWPRO)
		{
			*subtypenum = RAINBOWPRO_NUM;
			ret = 1;
		}
		else
		if (type == _PQC_MIX)
		{
			*subtypenum = MIX_NUM;
			ret = 1;
		}
		else
		if (type == 0)
		{
			*subtypenum = RAINBOWPRO_NUM + MIX_NUM;
			ret = 1;
		}
		else
			ret = 0;
	}
	else
	{
		ret = 0;
	}

	return ret;
}
unsigned int rainbowplus_sign_inner(unsigned int config_value, unsigned int set_display_debuginfo, unsigned int set_no_salt, unsigned char* sign_buf, unsigned int sign_size, unsigned char* m_hash, unsigned int mlen_hash, unsigned char* sk_buf, unsigned int sk_size, unsigned char* hash_of_sign)
{
	unsigned long handle = 0;
	unsigned int ret = 0;

	unsigned char* sm = NULL;
	unsigned int smlen = 0;
	unsigned char* hash_sign = NULL;
	unsigned int type_sign = 0;
	unsigned int subtype_sign = 0;
	unsigned int config_value_sign = 0;

	unsigned int type_inner = 0;
	unsigned int subtype_inner = 0;
	unsigned int config_value_inner = 0;


	unsigned int get_ret = rainbowplus_get_config(sk_buf, &config_value_inner, &type_inner, &subtype_inner,NULL,NULL,NULL);
	if (config_value == 0)
	{
		if (get_ret == 0)
			return 0;
		else
		{
			type_sign = type_inner;
			subtype_sign = subtype_inner;
			config_value_sign = config_value_inner;
		}

	}
	else
	{
		if (get_ret == 0)
			return 0;
		else
		{
			if (config_value == config_value_inner)
			{
				type_sign = type_inner;
				subtype_sign = subtype_inner;
				config_value_sign = config_value_inner;
			}
			else
				return 0;
		}
	}

	ret += rainbow_pro_init(&handle, type_sign, subtype_sign, set_display_debuginfo, set_no_salt, NULL, NULL, NULL, NULL);

	ret += rainbow_pro_sign(handle, set_display_debuginfo, &sm, &smlen, m_hash, mlen_hash, sk_buf + RAINBOWNPRO_HEADER_LEN, sk_size - RAINBOWNPRO_HEADER_LEN, (hash_of_sign == NULL) ? NULL : &hash_sign);
	if (sign_size == smlen)
	{
		memcpy(sign_buf, sm, sign_size);
	}
	else
		ret = 0;

	if (hash_of_sign && hash_sign)
	{
		memcpy(hash_of_sign, hash_sign, 32);
	}

	ret += rainbow_pro_uninit(handle);


	if (set_display_debuginfo >= 1)
	{
		if (set_display_debuginfo == 2)
		{
			printf_debuginfo("sign_buf", sign_buf, sign_size);
		}
		if (hash_of_sign)
			printf_debuginfo("hash_of_sign", hash_of_sign, 32);
	}

	if (ret == 3)
		return 1;
	else
		return 0;
}
unsigned int rainbowplus_sign(unsigned int config_value,  unsigned int set_display_debuginfo, unsigned int set_no_salt, unsigned char* sign_buf, unsigned int sign_size, unsigned char* m_hash, unsigned int mlen_hash, unsigned char* sk_buf, unsigned int sk_size, unsigned char* hash_of_sign)
{
	unsigned int ret = 0;
	unsigned int ret2 = 0;
	unsigned int type_sign = 0;
	unsigned int subtype_sign = 0;
	unsigned int config_value_sign = 0;

	unsigned int type_inner = 0;
	unsigned int subtype_inner = 0;
	unsigned int config_value_inner = 0;

	if ((sign_buf == NULL) || (m_hash == NULL) || (sk_buf == NULL))
	{
		return ret;
	}

	unsigned int get_ret = rainbowplus_get_config(sk_buf, &config_value_inner, &type_inner, &subtype_inner,NULL,NULL,NULL);
	if (config_value == 0)
	{
		if (get_ret == 0)
			return 0;
		else
		{
			type_sign = type_inner;
			subtype_sign = subtype_inner;
			config_value_sign = config_value_inner;
		}

	}
	else
	{
		if (get_ret == 0)
			return 0;
		else
		{
			if (config_value == config_value_inner)
			{
				type_sign = type_inner;
				subtype_sign = subtype_inner;
				config_value_sign = config_value_inner;
			}
			else
				return 0;
		}
	}

#ifdef  CHECK_IF_THE_CHOISED_CONFIG
	unsigned int if_ret = rainbowplus_if_the_choised_configvalue(config_value_sign);
	if (if_ret != 1)
		return ret;
#endif

	if ((type_sign == _PQC_MIX) && (subtype_sign == 0x01))
	{
		unsigned int sub_config_value = 0;
		unsigned int sub_sk_size = 0;
		unsigned int sub_pk_size = 0;
		unsigned int sub_hash_size = 0;
		unsigned int sub_sign_size = 0;
		unsigned char* hash_buf_temp = m_hash;
		unsigned char* sk_buf_temp = sk_buf+ RAINBOWNPRO_HEADER_LEN;
		unsigned char* sign_buf_temp = sign_buf;

		for (int i = 1; i <= PUBLIC_MIX_CONFIG_NUM; i++)
		{
			unsigned int type = PUBLIC_MIX_CONFIG[i][0];
			unsigned int subtype = PUBLIC_MIX_CONFIG[i][1];
			sub_config_value = get_config_value(type, subtype);
			ret2 += rainbowplus_get_size_inner(sub_config_value, set_display_debuginfo, &sub_sk_size, &sub_pk_size, &sub_hash_size, &sub_sign_size);

			ret2 += rainbowplus_sign_inner(
				sub_config_value,
				set_display_debuginfo,
				set_no_salt,
				sign_buf_temp,
				sub_sign_size,
				hash_buf_temp,
				sub_hash_size,
				sk_buf_temp,
				sub_sk_size,
				NULL);

			sign_buf_temp += sub_sign_size;
			hash_buf_temp += sub_hash_size;
			sk_buf_temp += sub_sk_size;

		}
		if (hash_of_sign)
		{
			rb_hash_msg(hash_of_sign, 32, sign_buf, sign_size, 32);
		}

		if (set_display_debuginfo >= 1)
		{
			if (set_display_debuginfo == 2)
			{
				printf_debuginfo("mix_sign_buf", sign_buf, sign_size);
			}
			if (hash_of_sign)
				printf_debuginfo("mix_hash_of_sign", hash_of_sign, 32);
		}

		if (ret2 == PUBLIC_MIX_CONFIG_NUM*2)
			ret = 1;
		else
			ret = 0;

	}
	else
	{
		ret = rainbowplus_sign_inner(
			config_value, 
			set_display_debuginfo, 
			set_no_salt, 
			sign_buf, 
			sign_size, 
			m_hash, 
			mlen_hash, 
			sk_buf, 
			sk_size, 
			hash_of_sign);
	}
	return ret;

}
unsigned int rainbowplus_verify_inner(unsigned int config_value, unsigned int set_display_debuginfo, unsigned char* hash_buf, unsigned int hash_size, unsigned char* sign_buf, unsigned int sign_size, unsigned char* pk_buf, unsigned int pk_size)
{
	unsigned long handle = 0;
	unsigned int ret = 0;
	unsigned int n_sk_size = 0;
	unsigned int n_pk_size = 0;
	unsigned int n_hash_size = 0;
	unsigned int n_sign_size = 0;
	unsigned int type_sign = 0;
	unsigned int subtype_sign = 0;
	unsigned int config_value_sign = 0;

	unsigned int type_inner = 0;
	unsigned int subtype_inner = 0;
	unsigned int config_value_inner = 0;

	unsigned int get_ret = rainbowplus_get_config(pk_buf, &config_value_inner, &type_inner, &subtype_inner,NULL,NULL,NULL);
	if (config_value == 0)
	{
		if (get_ret == 0)
			return 0;
		else
		{
			type_sign = type_inner;
			subtype_sign = subtype_inner;
			config_value_sign = config_value_inner;
		}

	}
	else
	{
		if (get_ret == 0)
			return 0;
		else
		{
			if (config_value == config_value_inner)
			{
				type_sign = type_inner;
				subtype_sign = subtype_inner;
				config_value_sign = config_value_inner;
			}
			else
				return 0;
		}
	}

	ret += rainbow_pro_init(&handle, type_sign, subtype_sign, set_display_debuginfo, 0, &n_sk_size, &n_pk_size, &n_hash_size, &n_sign_size);
	if ((n_pk_size + RAINBOWNPRO_HEADER_LEN != pk_size) || (n_hash_size != hash_size) || (n_sign_size != sign_size))
		return 0;
	ret += rainbow_pro_verify(handle, set_display_debuginfo, hash_buf, hash_size, sign_buf, sign_size, pk_buf + RAINBOWNPRO_HEADER_LEN, pk_size - RAINBOWNPRO_HEADER_LEN);
	ret += rainbow_pro_uninit(handle);

	if (ret == 3)
		return 1;
	else
		return 0;
}
unsigned int rainbowplus_verify(unsigned int config_value,  unsigned int set_display_debuginfo,  unsigned char* hash_buf, unsigned int hash_size, unsigned char* sign_buf, unsigned int sign_size, unsigned char* pk_buf, unsigned int pk_size)
{
	unsigned int ret = 0;
	unsigned int ret2 = 0;
	unsigned int type_sign = 0;
	unsigned int subtype_sign = 0;
	unsigned int config_value_sign = 0;

	unsigned int type_inner = 0;
	unsigned int subtype_inner = 0;
	unsigned int config_value_inner = 0;

	if ((hash_buf == NULL) || (sign_buf == NULL) || (pk_buf == NULL))
	{
		return ret;
	}

	unsigned int get_ret = rainbowplus_get_config(pk_buf, &config_value_inner, &type_inner, &subtype_inner,NULL,NULL,NULL);
	if (config_value == 0)
	{
		if (get_ret == 0)
			return 0;
		else
		{
			type_sign = type_inner;
			subtype_sign = subtype_inner;
			config_value_sign = config_value_inner;
		}

	}
	else
	{
		if (get_ret == 0)
			return 0;
		else
		{
			if (config_value == config_value_inner)
			{
				type_sign = type_inner;
				subtype_sign = subtype_inner;
				config_value_sign = config_value_inner;
			}
			else
				return 0;
		}
	}

#ifdef  CHECK_IF_THE_CHOISED_CONFIG
	unsigned int if_ret = rainbowplus_if_the_choised_configvalue(config_value_sign);
	if (if_ret != 1)
		return ret;
#endif

	if ((type_sign == _PQC_MIX) && (subtype_sign == 0x01))
	{
		unsigned int sub_config_value = 0;
		unsigned int sub_sk_size = 0;
		unsigned int sub_pk_size = 0;
		unsigned int sub_hash_size = 0;
		unsigned int sub_sign_size = 0;
		unsigned char* hash_buf_temp = hash_buf;
		unsigned char* pk_buf_temp = pk_buf + 6;
		unsigned char* sign_buf_temp = sign_buf;

		for (int i = 1; i <= PUBLIC_MIX_CONFIG_NUM; i++)
		{
			unsigned int type = PUBLIC_MIX_CONFIG[i][0];
			unsigned int subtype = PUBLIC_MIX_CONFIG[i][1];
			sub_config_value = get_config_value(type, subtype);
			ret2 += rainbowplus_get_size_inner(sub_config_value, set_display_debuginfo, &sub_sk_size, &sub_pk_size, &sub_hash_size, &sub_sign_size);

			ret2 += rainbowplus_verify_inner(
				sub_config_value,
				set_display_debuginfo,
				hash_buf_temp,
				sub_hash_size,
				sign_buf_temp,
				sub_sign_size,
				pk_buf_temp,
				sub_pk_size);

			sign_buf_temp += sub_sign_size;
			hash_buf_temp += sub_hash_size;
			pk_buf_temp += sub_pk_size;

		}

		if (ret2 == PUBLIC_MIX_CONFIG_NUM*2)
			ret = 1;
		else
			ret = 0;

	}
	else
	{
		ret = rainbowplus_verify_inner(
			config_value, 
			set_display_debuginfo, 
			hash_buf, 
			hash_size,
			sign_buf, 
			sign_size, 
			pk_buf, 
			pk_size);
	}
	return ret;

}


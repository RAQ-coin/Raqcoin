
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "rb_core_config.h"
#include "rb_core_utils.h"
#include "rb_core_rng.h"
#include "rb_core_api.h"
#include "rb_core_blas_comm.h"
#include "rb_core_blas.h"
#include "rb_core_parallel_matrix_op.h"
#include "rb_core_RAINBOW_PRO_API.h"
#include "rb_core_utils_hash.h"
#include "rb_core_blas_matrix_ref.h"
#include "rb_core_blas_comm_sse.h"



#ifdef WIN32_VERSION
#define PRINTF_TO_BUF sprintf_s
#endif

#if defined(LINUX_VERSION) || defined(MAC_OSX)
#define PRINTF_TO_BUF snprintf 
#endif



extern void rb_add_str001(unsigned long handle, char* str1, char* str2);

unsigned int rainbow_pro_signconfig_tobuffer(unsigned long handle)
{


	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

	
	int j = 0;

#ifdef WIN32_VERSION
	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%s", "RAINBOW(");
	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%2d,", HD->P_GFSIZE);
	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%2d,", HD->P_V1);
	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%2d,", HD->P_O1);
	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%2d)", HD->P_O2);

	if (HD->set_rainbow_type_id == _RAINBOW_COMPRESSED_TYPE)
	{
		j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%s,", " - compressed");
	}

	if (HD->set_display_debuginfo == 1)
	{
		printf("%s\n", HD->STR_CRYPTO_ALGNAME);
		printf("sk size: %lu\n", HD->P_CRYPTO_SECRETKEYBYTES);
		printf("pk size: %lu\n", HD->P_CRYPTO_PUBLICKEYBYTES);
		printf("hash size: %d\n", HD->P_HASH_LEN);
		printf("signature size: %d\n", HD->P_CRYPTO_BYTES);   
	}
#endif
#ifdef LINUX_VERSION

	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%s", "RAINBOW(");
	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%2d,", HD->P_GFSIZE);
	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%2d,", HD->P_V1);
	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%2d,", HD->P_O1);
	j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%2d)", HD->P_O2);
	
	if (HD->set_rainbow_type_id == _RAINBOW_COMPRESSED_TYPE)
	{
		j += PRINTF_TO_BUF(HD->STR_CRYPTO_ALGNAME + j, 100 - j, "%s,", " - compressed");
	}

	if (HD->set_display_debuginfo == 1)
	{
		printf("%s\n", HD->STR_CRYPTO_ALGNAME);
		printf("sk size: %08d\n", HD->P_CRYPTO_SECRETKEYBYTES);
		printf("pk size: %08d\n", HD->P_CRYPTO_PUBLICKEYBYTES);
		printf("hash size: %08d\n", HD->P_HASH_LEN);
		printf("signature size: %08d\n", HD->P_CRYPTO_BYTES);
	}

#endif



	return 1;

}
unsigned int rainbow_pro_index_mapping(unsigned int index)
{
	unsigned char index_out[72];
	unsigned char index_inner[72];
	for (int i = 0; i < 72; i++)
	{
		index_inner[i] = i;
	}
	for (int j = 0; j < 72; j++)
	{
		index_out[j] = index_inner[j];
	}

	return index_out[index];

}
#ifdef RB_SIMD_X86
extern int rb_get_cpu_support();
extern rb_cpu_s rb_cpu_su;
#endif

unsigned int rainbow_pro_init(unsigned long* handle, unsigned int  type, unsigned int  subtype, unsigned int set_display_debuginfo, unsigned int set_no_salt, unsigned int *sk_size, unsigned int *pk_size,unsigned int *hash_size, unsigned int *sign_size )
{
	unsigned int ret = 0;
	unsigned long hd_temp = 0;
	hd_temp=(unsigned long)rb_safe_malloc(sizeof(RB_CORE_HANDLE));
	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)hd_temp;
	
	if (HD == NULL)
		return 0;
	
	HD->set_display_debuginfo = set_display_debuginfo;
	HD->set_subtype = subtype;


	if (set_display_debuginfo != 1)
		set_display_debuginfo = 0;
	if (set_no_salt != 1)
		set_no_salt = 0;

	if ((type < 1) || (type > 1))
	{
		ret = 0;
		return ret;
	}


	HD->set_type = type;
	HD->set_subtype = subtype;


	if ((HD->set_type == _PQC_RAINBOWPRO) && (HD->set_subtype <= RAINBOWPRO_NUM))
	{
		HD->set_rainbow_type_id = (HD->set_subtype - 1) % 3 + 1;
		HD->set_rainbow_par_id = (HD->set_subtype - 1) / 3 + 1;
		HD->set_no_salt = set_no_salt;



	if (HD->set_rainbow_par_id == _UOV_RAINBOW256_112_44_0_PAR)
	{
	HD->set_use_gf16_gf256_par_id = _USE_GF256_PAR;
	HD->P_GFSIZE = 256;
	HD->P_V1 = 112;
	HD->P_O1 = 44;
	HD->P_O2 = 0;
	HD->P_HASH_LEN = 44;
	}
	else
	if (HD->set_rainbow_par_id == _UOV_RAINBOW256_184_72_0_PAR)
	{
	HD->set_use_gf16_gf256_par_id = _USE_GF256_PAR;
	HD->P_GFSIZE = 256;
	HD->P_V1 = 184;
	HD->P_O1 = 72;
	HD->P_O2 = 0;
	HD->P_HASH_LEN = 72;
	}
	else
	if (HD->set_rainbow_par_id == _UOV_RAINBOW256_244_96_0_PAR)
	{
		HD->set_use_gf16_gf256_par_id = _USE_GF256_PAR;
		HD->P_GFSIZE = 256;
		HD->P_V1 = 244;
		HD->P_O1 = 96;
		HD->P_O2 = 0;
		HD->P_HASH_LEN = 96;
	}
	else
	if (HD->set_rainbow_par_id == _RAINBOW16_72_8_48_PAR)
	{
		HD->set_use_gf16_gf256_par_id = _USE_GF16_PAR;
		HD->P_GFSIZE = 16;
		HD->P_V1 = 72;
		HD->P_O1 = 8;
		HD->P_O2 = 48;
		HD->P_HASH_LEN = 28;
	}
	else
	if (HD->set_rainbow_par_id == _RAINBOW256_104_8_64_PAR)
	{
		HD->set_use_gf16_gf256_par_id = _USE_GF256_PAR;
		HD->P_GFSIZE = 256;
		HD->P_V1 = 104;
		HD->P_O1 = 8;
		HD->P_O2 = 64;
		HD->P_HASH_LEN = 72;
	}
	else
	if (HD->set_rainbow_par_id == _RAINBOW256_148_8_96_PAR)
	{
		HD->set_use_gf16_gf256_par_id = _USE_GF256_PAR;
		HD->P_GFSIZE = 256;
		HD->P_V1 = 148;
		HD->P_O1 = 8;
		HD->P_O2 = 96;
		HD->P_HASH_LEN = 96;
	}
		HD->P_V2 = HD->P_V1 + HD->P_O1;
		HD->P_PUB_N = HD->P_V1 + HD->P_O1 + HD->P_O2; 
		HD->P_PUB_M = HD->P_O1 + HD->P_O2;			  

		if (HD->set_use_gf16_gf256_par_id == _USE_GF16_PAR)
		{
			HD->P_V1_BYTE = HD->P_V1 / 2;
			HD->P_V2_BYTE = HD->P_V2 / 2;
			HD->P_O1_BYTE = HD->P_O1 / 2;
			HD->P_O2_BYTE = HD->P_O2 / 2;
			HD->P_PUB_N_BYTE = HD->P_PUB_N / 2;
			HD->P_PUB_M_BYTE = HD->P_PUB_M / 2;
		}
		else
		{
			HD->P_V1_BYTE = HD->P_V1;
			HD->P_V2_BYTE = HD->P_V2;
			HD->P_O1_BYTE = HD->P_O1;
			HD->P_O2_BYTE = HD->P_O2;
			HD->P_PUB_N_BYTE = HD->P_PUB_N;
			HD->P_PUB_M_BYTE = HD->P_PUB_M;
		}
		HD->P_MAX_O = (HD->P_O1 > HD->P_O2) ? HD->P_O1 : HD->P_O2; 
		HD->P_MAX_O_BYTE = (HD->P_O1_BYTE > HD->P_O2_BYTE) ? HD->P_O1_BYTE : HD->P_O2_BYTE;

	
		HD->P_MAX_N = 340;  
		HD->P_MAX_ATTEMPT_FRMAT = 128; 

		if (HD->P_PUB_N > HD->P_MAX_N)
			exit(0);

		if (HD->P_GFSIZE == 16)
		{

			if (HD->P_PUB_M_BYTE > 32)
				exit(0);

			HD->P_TMPVEC_LEN = 32;
		}
		else
		{

			if (HD->P_PUB_M_BYTE > 128)
				exit(0);

			HD->P_TMPVEC_LEN = 128;
		}


		HD->P_SALT_BYTE = 16;
		HD->P_LEN_PKSEED = 32;
		HD->P_LEN_SKSEED = 32;
		HD->P_SIGNATURE_BYTE = HD->P_PUB_N_BYTE + HD->P_SALT_BYTE; 
		HD->P_CRYPTO_BYTES = HD->P_SIGNATURE_BYTE;

#ifdef MATCH_WITH_HAHAHA
		HD->sizeof_struct_pk_t = HD->P_PUB_M_BYTE * N_TRIANGLE_TERMS(HD->P_PUB_N) + 1;
		HD->member_len_of_pk_t[0] = HD->P_PUB_M_BYTE * N_TRIANGLE_TERMS(HD->P_PUB_N) + 1;
#else
		HD->sizeof_struct_pk_t = (HD->P_PUB_M_BYTE) * N_TRIANGLE_TERMS(HD->P_PUB_N);
		HD->member_len_of_pk_t[0] = (HD->P_PUB_M_BYTE) * N_TRIANGLE_TERMS(HD->P_PUB_N);
#endif

#ifdef MATCH_WITH_HAHAHA
		HD->sizeof_struct_sk_t = HD->P_O1_BYTE * HD->P_O1 +
			HD->P_O1_BYTE * HD->P_V1 * HD->P_O1 +
			HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_V1) +

			HD->P_O2_BYTE * HD->P_O2 +
			HD->P_O2_BYTE * HD->P_V2 * HD->P_O2 +
			HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_V2) +

			HD->P_PUB_N * HD->P_PUB_N_BYTE +
			HD->P_PUB_N_BYTE +
			HD->P_PUB_M * HD->P_PUB_M_BYTE +
			HD->P_PUB_M_BYTE +
			1;
#else
		HD->sizeof_struct_sk_t = HD->P_LEN_SKSEED +
			HD->P_O1_BYTE * HD->P_O2 +
			HD->P_V1_BYTE * HD->P_O1 +//
			HD->P_V1_BYTE * HD->P_O2 +
			HD->P_O1_BYTE * HD->P_O2 +//
			HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_V1) +//
			HD->P_O1_BYTE * HD->P_V1 * HD->P_O1 +//
			HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_V1) +
			HD->P_O2_BYTE * HD->P_V1 * HD->P_O1 +
			HD->P_O2_BYTE * HD->P_V1 * HD->P_O2 +
			HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_O1) +
			HD->P_O2_BYTE * HD->P_O1 * HD->P_O2;
#endif
		HD->member_len_of_sk_t[0] = HD->P_LEN_SKSEED;
		HD->member_len_of_sk_t[1] = HD->P_O1_BYTE * HD->P_O2;
		HD->member_len_of_sk_t[2] = HD->P_V1_BYTE * HD->P_O1;
		HD->member_len_of_sk_t[3] = HD->P_V1_BYTE * HD->P_O2;
		HD->member_len_of_sk_t[4] = HD->P_O1_BYTE * HD->P_O2;
		HD->member_len_of_sk_t[5] = HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_V1);
		HD->member_len_of_sk_t[6] = HD->P_O1_BYTE * HD->P_V1 * HD->P_O1;
		HD->member_len_of_sk_t[7] = HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_V1);
		HD->member_len_of_sk_t[8] = HD->P_O2_BYTE * HD->P_V1 * HD->P_O1;
		HD->member_len_of_sk_t[9] = HD->P_O2_BYTE * HD->P_V1 * HD->P_O2;
		HD->member_len_of_sk_t[10] = HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_O1);
		HD->member_len_of_sk_t[11] = HD->P_O2_BYTE * HD->P_O1 * HD->P_O2;

		HD->sizeof_struct_cpk_t = HD->P_LEN_PKSEED +
			HD->P_O1_BYTE * HD->P_V1 * HD->P_O2 +
			HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_O1) +
			HD->P_O1_BYTE * HD->P_O1 * HD->P_O2 +
			HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_O2) +
			HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_O2);

		HD->member_len_of_cpk_t[0] = HD->P_LEN_PKSEED;
		HD->member_len_of_cpk_t[1] = HD->P_O1_BYTE * HD->P_V1 * HD->P_O2;
		HD->member_len_of_cpk_t[2] = HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_O1);
		HD->member_len_of_cpk_t[3] = HD->P_O1_BYTE * HD->P_O1 * HD->P_O2;
		HD->member_len_of_cpk_t[4] = HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_O2);
		HD->member_len_of_cpk_t[5] = HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_O2);



		HD->sizeof_struct_csk_t = HD->P_LEN_PKSEED +
			HD->P_LEN_SKSEED;


		HD->member_len_of_csk_t[0] = HD->P_LEN_PKSEED;
		HD->member_len_of_csk_t[1] = HD->P_LEN_SKSEED;

		HD->sizeof_struct_ext_cpk_t = HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_V1) +
			HD->P_O1_BYTE * HD->P_V1 * HD->P_O1 +
			HD->P_O1_BYTE * HD->P_V1 * HD->P_O2 +
			HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_O1) +
			HD->P_O1_BYTE * HD->P_O1 * HD->P_O2 +
			HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_O2) +
			HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_V1) +
			HD->P_O2_BYTE * HD->P_V1 * HD->P_O1 +
			HD->P_O2_BYTE * HD->P_V1 * HD->P_O2 +
			HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_O1) +
			HD->P_O2_BYTE * HD->P_O1 * HD->P_O2 +
			HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_O2);

		HD->member_len_of_ext_cpk_t[0] = HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_V1);
		HD->member_len_of_ext_cpk_t[1] = HD->P_O1_BYTE * HD->P_V1 * HD->P_O1;
		HD->member_len_of_ext_cpk_t[2] = HD->P_O1_BYTE * HD->P_V1 * HD->P_O2;
		HD->member_len_of_ext_cpk_t[3] = HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_O1);
		HD->member_len_of_ext_cpk_t[4] = HD->P_O1_BYTE * HD->P_O1 * HD->P_O2;
		HD->member_len_of_ext_cpk_t[5] = HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_O2);
		HD->member_len_of_ext_cpk_t[6] = HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_V1);
		HD->member_len_of_ext_cpk_t[7] = HD->P_O2_BYTE * HD->P_V1 * HD->P_O1;
		HD->member_len_of_ext_cpk_t[8] = HD->P_O2_BYTE * HD->P_V1 * HD->P_O2;
		HD->member_len_of_ext_cpk_t[9] = HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_O1);
		HD->member_len_of_ext_cpk_t[10] = HD->P_O2_BYTE * HD->P_O1 * HD->P_O2;
		HD->member_len_of_ext_cpk_t[11] = HD->P_O2_BYTE * N_TRIANGLE_TERMS(HD->P_O2);


		
		if (HD->set_rainbow_type_id == _RAINBOW_COMPRESSED_TYPE)
		{
			HD->P_CRYPTO_SECRETKEYBYTES = HD->sizeof_struct_csk_t;// sizeof(rb_csk_t);
			HD->P_CRYPTO_PUBLICKEYBYTES = HD->sizeof_struct_cpk_t;// sizeof(rb_cpk_t);
		}


#if defined(WIN32_VERSION) || defined(LINUX_VERSION_X86)

#ifdef RB_SIMD_X86
		rb_get_cpu_support();
#endif
#endif

	
#ifdef OP_SSE
		if ((rb_cpu_su.supports_avx2 == 1) && (rb_cpu_su.supports_sse2 == 1))
		{
			HD->gf16mat_gauss_elim_8x16_impl = rb_gf16mat_gauss_elim_8x16_sse;
			HD->gf256mat_gauss_elim_impl = rb_gf256mat_gauss_elim_sse;
		}
		else
#endif
		{
			HD->gf16mat_gauss_elim_8x16_impl = rb_gf16mat_gauss_elim_8x16_ref;
			HD->gf256mat_gauss_elim_impl = rb_gf256mat_gauss_elim_ref;
		}

		HD->gf16mat_prod_impl		= rb_gf16mat_prod_ref;
		HD->gf256mat_prod_impl		= rb_gf256mat_prod_ref;

		HD->gf16mat_inv_8x8_impl	= rb_gf16mat_inv_8x8_ref;
		
		HD->gf16mat_solve_linear_eq_48x48_impl	= rb_gf16mat_solve_linear_eq_48x48_ref;

		HD->gf256mat_inv_8x8_impl   = rb_gf256mat_inv_8x8_ref;
		HD->gf256mat_inv_32x32_impl = rb_gf256mat_inv_32x32_ref;
		HD->gf256mat_inv_36x36_impl = rb_gf256mat_inv_36x36_ref;
		HD->gf256mat_inv_44x44_impl = rb_gf256mat_inv_44x44_ref;
		HD->gf256mat_inv_72x72_impl = rb_gf256mat_inv_72x72_ref;
		HD->gf256mat_inv_96x96_impl = rb_gf256mat_inv_96x96_ref;

		HD->gf256mat_solve_linear_eq_64x64_impl  = rb_gf256mat_solve_linear_eq_64x64_ref;
		HD->gf256mat_solve_linear_eq_96x96_impl  = rb_gf256mat_solve_linear_eq_96x96_ref;



		if (HD->set_rainbow_par_id == _UOV_RAINBOW256_112_44_0_PAR)
		{
			HD->gfmat_inv = rb_gf256mat_inv_44x44;
			HD->gfmat_solve_linear_eq = rb_gf16mat_solve_linear_eq_48x48;
		}
		else
		if (HD->set_rainbow_par_id == _UOV_RAINBOW256_184_72_0_PAR)
		{
			HD->gfmat_inv = rb_gf256mat_inv_72x72;
			HD->gfmat_solve_linear_eq = rb_gf16mat_solve_linear_eq_48x48;
		}
		else
		if (HD->set_rainbow_par_id == _UOV_RAINBOW256_244_96_0_PAR)
		{
			HD->gfmat_inv = rb_gf256mat_inv_96x96;
			HD->gfmat_solve_linear_eq = rb_gf16mat_solve_linear_eq_48x48;
		}
		else
		if (HD->set_rainbow_par_id == _RAINBOW16_72_8_48_PAR)
		{
			HD->gfmat_inv = rb_gf16mat_inv_8x8;
			HD->gfmat_solve_linear_eq = rb_gf16mat_solve_linear_eq_48x48;
		}
		else
		if (HD->set_rainbow_par_id == _RAINBOW256_104_8_64_PAR)
		{
			HD->gfmat_inv = rb_gf256mat_inv_8x8;
			HD->gfmat_solve_linear_eq = rb_gf256mat_solve_linear_eq_64x64;
		}
		else
		if (HD->set_rainbow_par_id == _RAINBOW256_148_8_96_PAR)
		{
			HD->gfmat_inv = rb_gf256mat_inv_8x8;
			HD->gfmat_solve_linear_eq = rb_gf256mat_solve_linear_eq_96x96;
		}

		if (HD->set_use_gf16_gf256_par_id == _USE_GF16_PAR)
		{
#ifdef OP_AVX2 
			if ((rb_cpu_su.supports_avx2 == 1)&& (rb_cpu_su.supports_sse2 == 1))
			{
				HD->gfv_mul_scalar = gf16v_mul_scalar_avx2;
				HD->gfv_madd = gf16v_madd_avx2;
				HD->gf256v_add = gf256v_add_avx2;
			}
			else
#endif
			{
				HD->gfv_mul_scalar = rb__gf16v_mul_scalar_u32;
				HD->gfv_madd = rb__gf16v_madd_u32;
				HD->gf256v_add = rb__gf256v_add_u32;
			}

			HD->gfv_get_ele = rb_gf16v_get_ele;
			HD->gf256v_conditional_add = rb__gf256v_conditional_add_u32;
			HD->gfmat_prod = rb_gf16mat_prod;

			HD->batch_trimat_madd = rb_batch_trimat_madd_gf16;
			HD->batch_trimatTr_madd = rb_batch_trimatTr_madd_gf16;
			HD->batch_2trimat_madd = rb_batch_2trimat_madd_gf16;
			HD->batch_matTr_madd = rb_batch_matTr_madd_gf16;
			HD->batch_bmatTr_madd = rb_batch_bmatTr_madd_gf16;
			HD->batch_mat_madd = rb_batch_mat_madd_gf16;

			HD->batch_quad_trimat_eval = rb_batch_quad_trimat_eval_gf16;
			HD->batch_quad_recmat_eval = rb_batch_quad_recmat_eval_gf16;

		}
		else
		{
#ifdef OP_AVX2
			if ((rb_cpu_su.supports_avx2 == 1) && (rb_cpu_su.supports_sse2 == 1))
			{
				HD->gfv_mul_scalar = gf256v_mul_scalar_avx2;
				HD->gfv_madd = gf256v_madd_avx2;
				HD->gf256v_add = gf256v_add_avx2;
			}
			else
#endif
			{
				HD->gfv_mul_scalar = rb__gf256v_mul_scalar_u32;
				HD->gfv_madd = rb__gf256v_madd_u32;
				HD->gf256v_add = rb__gf256v_add_u32;
			}

			HD->gfv_get_ele = rb_gf256v_get_ele;
			HD->gf256v_conditional_add = rb__gf256v_conditional_add_u32;
			HD->gfmat_prod = rb_gf256mat_prod;

			HD->batch_trimat_madd = rb_batch_trimat_madd_gf256;
			HD->batch_trimatTr_madd = rb_batch_trimatTr_madd_gf256;
			HD->batch_2trimat_madd = rb_batch_2trimat_madd_gf256;
			HD->batch_matTr_madd = rb_batch_matTr_madd_gf256;
			HD->batch_bmatTr_madd = rb_batch_bmatTr_madd_gf256;
			HD->batch_mat_madd = rb_batch_mat_madd_gf256;

			HD->batch_quad_trimat_eval = rb_batch_quad_trimat_eval_gf256;
			HD->batch_quad_recmat_eval = rb_batch_quad_recmat_eval_gf256;

		}

		*handle = (unsigned long)HD;



		rainbow_pro_signconfig_tobuffer(hd_temp);

		if (sk_size != NULL)
			*sk_size = HD->P_CRYPTO_SECRETKEYBYTES;
		if (pk_size != NULL)
			*pk_size = HD->P_CRYPTO_PUBLICKEYBYTES;
		if (hash_size != NULL)
			*hash_size = HD->P_HASH_LEN;
		if (sign_size != NULL)
		{
#ifdef ONLY_SIGN
			* sign_size = HD->P_SIGNATURE_BYTE;
#else
			* sign_size = HD->P_SIGNATURE_BYTE + HD->P_HASH_LEN;

#endif
		}


		ret = 1;
	}
	else
	ret = 0;

	return ret;
}




unsigned int rainbow_pro_genkey(unsigned long handle, unsigned int set_display_debuginfo, unsigned char* seed, unsigned int seed_len, unsigned char** Private_Key, unsigned int* Private_Key_Len, unsigned char** Public_Key, unsigned int* Public_Key_Len, unsigned char** Hash_of_Private_Key, unsigned char** Hash_of_Public_Key)
{
	unsigned int ret = 0;
	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

	HD->set_display_debuginfo = set_display_debuginfo;
	if(HD->set_display_debuginfo==1)
		printf("\nStart Genkey------>\n");


	if ((HD->set_type == _PQC_RAINBOWPRO) && (HD->set_subtype <= RAINBOWPRO_NUM))
	{
		if (HD->set_display_debuginfo == 1)
		{
			rainbow_pro_signconfig_tobuffer(handle);
		}

		// set random seed

		memset(HD->rnd_seed, 0, sizeof(HD->rnd_seed));
		rb_hash_msg(HD->rnd_seed, sizeof(HD->rnd_seed), seed, seed_len, 48);

		memset(HD->personalization_string, 0, sizeof(HD->personalization_string));
		rb_randombytes_init(handle, HD->rnd_seed, HD->personalization_string);


		HD->_sk = (uint8_t*)rb_safe_malloc(HD->P_CRYPTO_SECRETKEYBYTES);
		HD->_pk = (uint8_t*)rb_safe_malloc(HD->P_CRYPTO_PUBLICKEYBYTES);

		int r = rb_crypto_generate_keypair(handle, HD->_pk, HD->_sk);

		if (r != 0)
		{
			printf("%s genkey fails.\n", HD->STR_CRYPTO_ALGNAME);
			ret = 0;
			return ret;
		}
		*Private_Key = HD->_sk;
		*Private_Key_Len = HD->P_CRYPTO_SECRETKEYBYTES;
		*Public_Key = HD->_pk;
		*Public_Key_Len = HD->P_CRYPTO_PUBLICKEYBYTES;

		if (Hash_of_Private_Key)
		{
			rb_hash_msg(HD->hash_of_sk, 32, HD->_sk, HD->P_CRYPTO_SECRETKEYBYTES, 32);
			*Hash_of_Private_Key = HD->hash_of_sk;
		}

		if (Hash_of_Public_Key)
		{
			rb_hash_msg(HD->hash_of_pk, 32, HD->_pk, HD->P_CRYPTO_PUBLICKEYBYTES, 32);
			*Hash_of_Public_Key = HD->hash_of_pk;
		}

		if (HD->set_display_debuginfo == 1)
		{
			if (Hash_of_Private_Key)
				rainbow_pro_savebuffer_tofile(handle, NULL, HD->hash_of_sk, 32, " hash of secret key", 1, 0);
			if (Hash_of_Public_Key)
				rainbow_pro_savebuffer_tofile(handle, NULL, HD->hash_of_pk, 32, " hash of public key", 1, 0);
			printf("generate %s pk/sk success.\n", HD->STR_CRYPTO_ALGNAME);
		}

		ret = 1;
	}
	else
		ret = 0;

	return ret;
}



unsigned int rainbow_pro_readfilebyte_to_onebuffer(unsigned char** msg, unsigned int* len, const char* f_name)
{
	int ret = 1;
	int r = rb_byte_read_file_onebuffer(msg, len, f_name);
	if (0 != r)
	{
		printf("fail to read message file.\n");
		ret = 0;
	}
	return ret;
}
extern int rb_hash_msg(unsigned char* digest, unsigned int len_digest, const unsigned char* m, unsigned int mlen, unsigned int P_HASH_LEN);
unsigned int rainbow_pro_digest_hash(unsigned long handle, unsigned int set_display_debuginfo, unsigned char* m, unsigned int mlen, unsigned char** digest_hash, unsigned int* digest_hash_len)
{
	int ret = 0;
	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

	HD->set_display_debuginfo = set_display_debuginfo;


	if ((HD->set_type == _PQC_RAINBOWPRO) && (HD->set_subtype <= RAINBOWPRO_NUM))
	{
		HD->mlen = mlen;
		HD->msg = m;
		HD->_hash_buffer = (unsigned char*)rb_safe_malloc(HD->P_HASH_LEN);

		rb_hash_msg(HD->_hash_buffer, HD->P_HASH_LEN, m, mlen, HD->P_HASH_LEN);

		*digest_hash = HD->_hash_buffer;
		*digest_hash_len = HD->P_HASH_LEN;
		if (HD->set_display_debuginfo == 1)
		{
			rainbow_pro_savebuffer_tofile(handle, NULL, m, (unsigned int)mlen, " Msg", 1, 0);
			rainbow_pro_savebuffer_tofile(handle, NULL, HD->_hash_buffer, (unsigned int)HD->P_HASH_LEN, " Hash from rainbow_pro_digest_hash", 1, 0);
		}

		ret = 1;
	}
	else
		ret = 0;
	return ret;
}
void rb_add_str001(unsigned long handle, char* str1, char* str2)
{
	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
	int j = 0;

	if(HD->set_subtype <= RAINBOWPRO_NUM)
		j += PRINTF_TO_BUF(str1 + j, 100 - j, "%s,", HD->STR_CRYPTO_ALGNAME);

	j += PRINTF_TO_BUF(str1 + j, 100 - j, "%s,", str2);

	return;
}
unsigned int rainbow_pro_savebuffer_tofile(unsigned long handle, char* filename, unsigned char* buffer, unsigned int buffer_size, char* dataname, int mode, int format)
{

	FILE* fp = NULL;
	unsigned int ret = 0;
	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

	if (buffer == NULL)
		return 0;

	if (mode == 0)
	{

#ifdef WIN32_VERSION
		if (filename)
		{
			fopen_s(&fp, filename, "w+");
			if (NULL == fp) {
				printf("fail to open file of\n");
				printf("%s", dataname);
				ret = 0;
				return ret;
			}
		}
#endif

#ifdef LINUX_VERSION
		if (filename)
		{
			fp = fopen(filename, "w+");
			if (NULL == fp) {
				printf("fail to open file of\n");
				printf("%s", dataname);
				ret = 0;
				return ret;
			}
		}
#endif
		
		
		ALIGN(32) char out001[100];
		rb_add_str001(handle, out001, dataname);
		rb_byte_fdump(fp, out001, buffer, buffer_size, format);
		if (fp)
			fclose(fp);
	}
	else
		if (mode == 1) 
		{

#ifdef WIN32_VERSION
			if (filename)
			{
				fopen_s(&fp, filename, "w+");
				if (NULL == fp) {
					printf("fail to open file of\n");
					printf("%s", dataname);
					ret = 0;
					return ret;
				}
			}
#endif

#ifdef LINUX_VERSION
			if (filename)
			{
				fp = fopen(filename, "w+");
				if (NULL == fp) {
					printf("fail to open file of\n");
					printf("%s", dataname);
					ret = 0;
					return ret;
				}
			}
#endif
			
			ALIGN(32) char out001[100];
			rb_add_str001(handle, out001, dataname);
			rb_byte_fdump(stdout, out001, buffer, buffer_size, format);
			if(fp)
				fclose(fp);
		}
	if (mode == 2)
	{

#ifdef WIN32_VERSION
		if (filename)
		{
			fopen_s(&fp, filename, "w+");
			if (NULL == fp) {
				printf("fail to open file of\n");
				printf("%s", dataname);
				ret = 0;
				return ret;
			}
		}
#endif

#ifdef LINUX_VERSION
		if (filename)
		{
			fp = fopen(filename, "w+");
			if (NULL == fp) {
				printf("fail to open file of\n");
				printf("%s", dataname);
				ret = 0;
				return ret;
			}
		}
#endif
		
		ALIGN(32) char out001[100];
		rb_add_str001(handle, out001, dataname);
		rb_byte_fdump(fp, out001, buffer, buffer_size, format);
		rb_byte_fdump(stdout, out001, buffer, buffer_size, format);
		if (fp)
			fclose(fp);
	}
	ret = 1;
	return ret;

}
unsigned int rainbow_pro_uninit(unsigned long handle)
{
	unsigned int ret = 0;
	RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

	rb_safe_free(HD->_sk);

	rb_safe_free(HD->_pk);

	rb_safe_free(HD->signature);

	rb_safe_free(HD->_hash_buffer);

	rb_safe_free(HD);

	ret = 1;
	return ret;
	
}






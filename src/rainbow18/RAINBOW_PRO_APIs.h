#ifndef RAINBOWPROPLUS_APIs_H
#define RAINBOWPROPLUS_APIs_H



#include <time.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef __linux
/* Linux, or a Win32 static library */
#ifndef RBPROCALL
#define RBPROCALL extern unsigned int
#endif
#ifndef RBPROCALL_S
#define RBPROCALL_S extern int  
#endif
#ifndef RBPROCALL_VOID
#define RBPROCALL_VOID extern void
#endif
#ifndef RBPROCALL_VOIDP
#define RBPROCALL_VOIDP extern void*
#endif

#endif

#if defined(__APPLE__) || defined(__MACH__)
/* macOS */
#ifndef RBPROCALL
#define RBPROCALL extern unsigned int
#endif
#ifndef RBPROCALL_S
#define RBPROCALL_S extern int  
#endif
#ifndef RBPROCALL_VOID
#define RBPROCALL_VOID extern void
#endif
#ifndef RBPROCALL_VOIDP
#define RBPROCALL_VOIDP extern void*
#endif

#endif

#ifdef WIN32_VERSION
#ifndef RBPROCALL
#define RBPROCALL extern __declspec(dllexport)  unsigned int
#endif
#ifndef RBPROCALL_S
#define RBPROCALL_S extern __declspec(dllexport) int
#endif
#ifndef RBPROCALL_VOID
#define RBPROCALL_VOID extern __declspec(dllexport)   void
#endif
#ifndef RBPROCALL_VOIDP
#define RBPROCALL_VOIDP extern __declspec(dllexport)  void*
#endif

#endif

#if defined ( __cplusplus )
extern "C" {
#endif /* __cplusplus */




#ifndef DLL_DYNAMIC_LINK
/*RBPROCALL rainbow_pro_init(
	unsigned long* handle, 
	unsigned int  type,
	unsigned int  subtype,
	unsigned int set_display_debuginfo, 
	unsigned int set_no_salt, 
	unsigned int* sk_size, unsigned int* pk_size, unsigned int* hash_size, unsigned int* sign_size);
RBPROCALL rainbow_pro_uninit(unsigned long handle);
RBPROCALL rainbow_pro_genkey(
	unsigned long handle, 
	unsigned int set_display_debuginfo, 
	unsigned char* seed, unsigned int seed_len, 
	unsigned char** Private_Key, unsigned int* Private_Key_Len,  
	unsigned char** Public_Key, unsigned int* Public_Key_Len, 
	unsigned char** Hash_of_Private_Key, unsigned char** Hash_of_Public_Key);
RBPROCALL rainbow_pro_digest_hash(
	unsigned long handle, 
	unsigned int set_display_debuginfo, 
	unsigned char* m, unsigned int mlen, 
	unsigned char** digest, unsigned int* digest_len);
RBPROCALL rainbow_pro_sign(
	unsigned long handle, 
	unsigned int set_display_debuginfo, 
	unsigned char** sm, unsigned int* smlen, 
	unsigned char* m, unsigned int mlen, 
	unsigned char* sk_buf, unsigned int sk_size, 
	unsigned char** hash_of_sign);
RBPROCALL rainbow_pro_verify(
	unsigned long handle, 
	unsigned int set_display_debuginfo, 
	unsigned char* hash_buf, unsigned int hash_size, 
	unsigned char* sign_buf, unsigned int sign_size, 
	unsigned char* pk_buf, unsigned int pk_size);
RBPROCALL rainbow_pro_readfilebyte_to_onebuffer(unsigned char** msg, unsigned int* len, const char* f_name);
RBPROCALL rainbow_pro_savebuffer_tofile(unsigned long handle, char* filename, unsigned char* buffer, unsigned int buffer_size, char* dataname, int mode, int format);

RBPROCALL rainbow_pro_sha256_init(unsigned long* hash_handle);
RBPROCALL rainbow_pro_sha256_update(unsigned long hash_handle, unsigned char* data, unsigned int len);
RBPROCALL rainbow_pro_sha256_final(unsigned long hash_handle, unsigned char* hash_digest);

RBPROCALL rainbow_pro_sha384_init(unsigned long* hash_handle);
RBPROCALL rainbow_pro_sha384_update(unsigned long hash_handle, unsigned char* data, unsigned int len);
RBPROCALL rainbow_pro_sha384_final(unsigned long hash_handle, unsigned char* hash_digest);

RBPROCALL rainbow_pro_sha512_init(unsigned long* hash_handle);
RBPROCALL rainbow_pro_sha512_update(unsigned long hash_handle, unsigned char* data, unsigned int len);
RBPROCALL rainbow_pro_sha512_final(unsigned long hash_handle, unsigned char* hash_digest);

RBPROCALL_VOIDP rb_safe_calloc(unsigned int num, unsigned int size);
RBPROCALL_VOIDP rb_safe_malloc(unsigned int length);
RBPROCALL_VOID  rb_safe_free(void* p);
RBPROCALL_VOIDP rb_safe_realloc(void* p, unsigned int _NewSize);



RBPROCALL rainbowplus_help();
RBPROCALL rainbowplus_get_number_of_type(unsigned int* typenum);
RBPROCALL rainbowplus_get_subtype_number_of_onetype(unsigned int type, unsigned int* subtypenum);
RBPROCALL rainbowplus_get_number_of_subtype(unsigned int* subtypenum);
RBPROCALL rainbowplus_get_config_value_list(unsigned int* config_value_list, unsigned int  subtypenum);
RBPROCALL rainbowplus_get_info_of_config_value(unsigned int config_value, unsigned int *type,unsigned int *subtype,char *config_name);
*/
RBPROCALL rainbowplus_choised_help();
RBPROCALL rainbowplus_get_number_of_choised_total_config(unsigned int* choised_total_config_num);
RBPROCALL rainbowplus_get_choised_config_value_list(unsigned int* choised_config_value_list,unsigned int  choised_total_config_num);
RBPROCALL rainbowplus_get_choised_config(unsigned int choised_index, unsigned int* config_value, unsigned int* type, unsigned int* subtype, char * config_name, char *choised_sign_name, char *choised_sign_discription);
RBPROCALL rainbowplus_get_choised_config_from_config_value(unsigned int config_value, unsigned int* out_type, unsigned int* out_subtype, char* config_name, char* choised_sign_name, char* choised_sign_discription);
RBPROCALL rainbowplus_get_config(unsigned char* skpk_buf,unsigned int* config_value,unsigned int* type,unsigned int* subtype, char* config_name, char* choised_sign_name, char* choised_sign_discription);
RBPROCALL rainbowplus_get_choised_info(
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
	unsigned int* type,
	unsigned int* subtype
);
RBPROCALL rainbowplus_choised_check_len(unsigned int type,/*1:sk,2:pk,3:signsize,4:hashsize,0:guess*/unsigned int len);
RBPROCALL rainbowplus_if_the_choised_configvalue(unsigned int config_value);


RBPROCALL rainbowplus_get_size(
	unsigned int config_value,
	unsigned int set_display_debuginfo,
	unsigned int* sk_size, unsigned int* pk_size, unsigned int* hash_size, unsigned int* sign_size);
RBPROCALL rainbowplus_get_keypair(
	unsigned int config_value,
	unsigned int set_display_debuginfo,
	unsigned char* seed, unsigned int seed_len,
	unsigned char* sk_buf, unsigned int sk_size,
	unsigned char* pk_buf, unsigned int pk_size,
	unsigned char* Hash_of_Private_Key,
	unsigned char* Hash_of_Public_Key);
RBPROCALL rainbowplus_hash(
	unsigned int config_value,
	unsigned int set_display_debuginfo,
	unsigned char* m, unsigned int mlen,
	unsigned char* hash_buf, unsigned int hash_size);
RBPROCALL rainbowplus_sign(
	unsigned int config_value,
	unsigned int set_display_debuginfo,
	unsigned int set_no_salt,
	unsigned char* sign_buf, unsigned int sign_size,
	unsigned char* m_hash, unsigned int mlen_hash,
	unsigned char* sk_buf, unsigned int sk_size,
	unsigned char* hash_of_sign);
RBPROCALL rainbowplus_verify(
	unsigned int config_value,
	unsigned int set_display_debuginfo,
	unsigned char* hash_buf, unsigned int hash_size,
	unsigned char* sign_buf, unsigned int sign_size,
	unsigned char* pk_buf, unsigned int pk_size);

#endif





#if defined ( __cplusplus )
}
#endif /* __cplusplus */


#endif /* RAINBOWPROPLUS_APIs_H */

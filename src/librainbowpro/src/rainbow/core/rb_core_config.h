#ifndef rb__API_CONFIG_H_
#define rb__API_CONFIG_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>



//#define WIN32_VERSION
//#define LINUX_VERSION

#define CHECK_IF_THE_CHOISED_CONFIG

#ifdef WIN32_VERSION
	//#define RB_SIMD_X86

	#ifdef RB_SIMD_X86
		#define RB_SAFE_MEM
	#endif
	//#define RB_SAFE_MEM
	//#define rb_debug_printf
#endif

#ifdef LINUX_VERSION_X86
	#define LINUX_VERSION
	//#define RB_SIMD_X86

	#ifdef RB_SIMD_X86
		#define RB_SAFE_MEM
	#endif
	//#define RB_SAFE_MEM
	//#define rb_debug_printf
#endif

#ifdef LINUX_VERSION_ARM
	#define LINUX_VERSION
	//#define RB_SAFE_MEM
	//#define rb_debug_printf
#endif
 
#ifdef RB_SIMD_X86
	#define OP_AVX2
	#define OP_SSE
#endif


#define RAINBOWNPRO_HEADER_LEN 6


#define ONLY_SIGN


#define RAINBOWPRO_NUM	30
#define MIX_NUM			1

#if defined(__GNUC__)
#define ALIGN(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
#define ALIGN(x) __declspec(align(x))
#elif defined(__ARMCC_VERSION)
#define ALIGN(x) __align(x)
#else
#define ALIGN(x)
#endif



//#define HASH_APIs_Be_Used_IN_Inner

#ifdef WIN32_VERSION

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
#endif

#ifdef LINUX_VERSION

typedef signed char        int8_t;
typedef short       int16_t;
typedef int         int32_t;
typedef  __uint64_t     uint64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef __int64_t   int64_t;
#endif

#ifdef RB_SIMD_X86

typedef struct rb__cpu_s
{
	unsigned int supports_cpuid;
	unsigned int supports_mmx;
	unsigned int supports_sse;
	unsigned int supports_sse2;
	unsigned int supports_sse3;
	unsigned int supports_ssse3;
	unsigned int supports_sse4_1;
	unsigned int supports_sse4_2;
	unsigned int supports_avx;
	unsigned int supports_avx2;
	unsigned int supports_avx512;
	unsigned int supports_aes;
}rb_cpu_s;

#endif

#endif  

#ifndef _RAINBOW_CONFIG_H_
#define _RAINBOW_CONFIG_H_


/// (32,32,32) (56,48,48) (76,64,64)

#define _RAINBOW16_32_32_32
//#define _RAINBOW16_56_48_48
//#define _RAINBOW16_76_64_64


#if defined _RAINBOW16_32_32_32
#define _V1 32
#define _O1 32
#define _O2 32
#define _HASH_LEN 32

#elif defined _RAINBOW16_56_48_48
#define _V1 56
#define _O1 48
#define _O2 48
#define _HASH_LEN 48

#elif defined _RAINBOW16_76_64_64
#define _V1 76
#define _O1 64
#define _O2 64
#define _HASH_LEN 64

#else
error here.
#endif


#define _GFSIZE 16

#define STR1(x) #x
#define THE_NAME(gf,v1,o1,o2) "RAINBOW(" STR1(gf) "," STR1(v1) "," STR1(o1) "," STR1(o2) ")"

#define _S_NAME THE_NAME(_GFSIZE,_V1,_O1,_O2)


#define _RAINBOW_16


#define V2 ((_V1)+(_O1))

#define _V1_BYTE (_V1/2)
#define _V2_BYTE (V2/2)
#define _O1_BYTE (_O1/2)
#define _O2_BYTE (_O2/2)


#ifdef _RAINBOW_16
#define _PUB_N  (_V1+_O1+_O2)
#define _PUB_M  (_O1+_O2)
#define _SEC_N (_PUB_N)
#define _PUB_N_BYTE  (_PUB_N/2)
#define _PUB_M_BYTE  (_PUB_M/2)

#else
error
#endif


#define _SALT_BYTE 16

#define _SIGNATURE_BYTE (_PUB_N_BYTE + _SALT_BYTE )


#define TERMS_QUAD_POLY(N) (((N)*(N+1)/2)+N+1)

/// extra 1 for salt
#define _PUB_KEY_LEN (TERMS_QUAD_POLY(_PUB_N)*(_PUB_M_BYTE)+1)




#endif

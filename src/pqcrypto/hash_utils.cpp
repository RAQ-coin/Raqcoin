
#include "hash_utils.h"
#include "pqcrypto.h"



#ifndef _HASH_LEN
#define _HASH_LEN (32)
#endif

static inline
int _sha2_str( unsigned char * digest , const unsigned char * m , unsigned long long mlen )
{
#if 32 == _HASH_LEN
	Sha256 sha256;
	sha256Init( &sha256 );
	sha256Process( &sha256 , m , mlen );
	sha256Done(&sha256, digest );
#elif 48 == _HASH_LEN
	Sha384 sha384;
	sha384Init( &sha384 );
	sha384Process( &sha384 , m , mlen );
	sha384Done(&sha384,digest );
#elif 64 == _HASH_LEN
	Sha512 sha512;
	sha512Init( &sha512 );
	sha512Process( &sha512 , m , mlen );
	sha512Done( &sha512,digest  );
#else
error: un-supported _HASH_LEN
#endif
	return 0;
}


int sha2_chain( unsigned char * d2 , const unsigned char * d1 )
{
	return _sha2_str( d2 , d1 , _HASH_LEN );
}


#define _BUFFER_SIZE ((_HASH_LEN)*128)

int sha2_file( unsigned char * digest , FILE * fp )
{
	unsigned char buffer[_BUFFER_SIZE] ={0};
	int nbyte = 0;
	int is_read = -1;

#if 32 == _HASH_LEN
	Sha256 sha256;
	sha256Init( &sha256 );
	while( ( nbyte = fread(buffer,1,_BUFFER_SIZE,fp) ) ) { sha256Process( &sha256 , buffer , nbyte ); is_read = 0; }
	sha256Done( &sha256, digest );
#elif 48 == _HASH_LEN
	Sha384 sha384;
	sha384Init( &sha384 );
	while( ( nbyte = fread(buffer,1,_BUFFER_SIZE,fp) ) ) { sha384Process( &sha384 , buffer , nbyte ); is_read = 0; }
	sha384Done(  &sha384,digest  );
#elif 64 == _HASH_LEN
	Sha512 sha512;
	sha512Init( &sha512 );
	while( ( nbyte = fread(buffer,1,_BUFFER_SIZE,fp) ) ) { sha512Process( &sha512 , buffer , nbyte ); is_read = 0; }
	sha512Done( &sha512 ,digest );
#else
error: un-supported _HASH_LEN
#endif


	return is_read;
}


//////////////////////////////////////////////////////////////



static inline
int expand_sha2( unsigned char * digest , unsigned n_digest , const unsigned char * hash )
{
	if( _HASH_LEN >= n_digest ) {
		for(unsigned i=0;i<n_digest;i++) digest[i] = hash[i];
		return 0;
	} else {
		for(unsigned i=0;i<_HASH_LEN;i++) digest[i] = hash[i];
		n_digest -= _HASH_LEN;
	}

	while( _HASH_LEN <= n_digest ) {
		sha2_chain( digest+_HASH_LEN , digest );

		n_digest -= _HASH_LEN;
		digest += _HASH_LEN;
	}
	unsigned char temp[_HASH_LEN];
	if( n_digest ){
		sha2_chain( temp , digest );
		for(unsigned i=0;i<n_digest;i++) digest[_HASH_LEN+i] = temp[i];
	}
	return 0;
}


int sha2_chain_file( unsigned char * digest , unsigned n_digest , const char * f_name )
{
	FILE * fp = fopen( f_name , "rb");
        if( NULL == fp ) return -1;

	unsigned char hash[_HASH_LEN] = {0};
	if( 0 > sha2_file( hash , fp ) ) {
		fclose( fp );
		return -1;
	}
	fclose( fp );

	return expand_sha2( digest , n_digest , hash );
}


int sha2_chain_msg( unsigned char * digest , unsigned n_digest , const unsigned char * m , unsigned long long mlen )
{
	unsigned char hash[_HASH_LEN];
	_sha2_str( hash , m , mlen );

	return expand_sha2( digest , n_digest , hash );
}



int sha2_chain_byte( unsigned char * output_bytes , unsigned output_size , unsigned * chain_status , unsigned char * chain )
{
	unsigned counter = chain_status[0];
	for(unsigned i=0;i<output_size;i++) {
		if( _HASH_LEN <= counter ) {
			sha2_chain( chain , chain );
			counter = 0;
		}
		output_bytes[i] = chain[counter++];
	}
	chain_status[0] = counter;
	return 0;
}



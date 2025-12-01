#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rb_core_config.h"
#include "rb_core_RAINBOW_PRO_API.h"
#include "rb_core_utils.h"



int rb_byte_fdump(FILE * fp, const char * extra_msg , const unsigned char *v, unsigned int n_byte, int format)
{
	int r = 0;
	int t = fprintf( fp , "\n\n Len:%d,%s = \n",n_byte, extra_msg );
	if( 0 > t ) 
		return t;
	else 
		r += t;
	if (format == 0)
	{
		for (unsigned int i = 0; i < n_byte; i++)
		{
			int t1 = fprintf(fp, "%02x", v[i]);
			if (0 > t1)
				return t1;
			else
				r += t1;
		}
	}
	else
	{
		fprintf(fp, "%s", v);
	}

	fprintf(fp, "\n\n");
	return r;
}


static inline
int rb_is_empty( char c )
{
	if( ' ' == c ) return 1;
	if( '\t' == c ) return 1;
	if( '\n' == c ) return 1;
	return 0;
}

int rb_byte_fget( FILE * fp, unsigned char *v , unsigned int n_byte )
{
	char c0 = 0, c1 = 0;
	char vv[3]; vv[2] = '\0';

	while( EOF != c0 ) {
		c0 = (char)fgetc( fp );
		if( ('=' == c0) ) break;
	}

	int r = 0;
	while( 1 ) {
		while(rb_is_empty( c0 = (char)fgetc(fp) ) ) ;
		c1 = (char)fgetc( fp );
		if( EOF == c0 ) break;
		if( EOF == c1 ) break;

		vv[0] = c0;
		vv[1] = c1;

		int value = 0;
#ifdef WIN32_VERSION
		int t = sscanf_s(vv, "%2x", &value);
#endif

#if defined(LINUX_VERSION) || defined(MAC_OSX)
		int t = sscanf( vv , "%2x" , &value );
#endif
		if( 1 == t ) {
			v[r] = (unsigned char)value;
			r++;
		}
		else break;
		if( n_byte == (unsigned int)r ) return r;
	}
	if( 0 < r ) 
		return r;

	return -1;

}



int rb_byte_from_file( unsigned char *v , unsigned int n_byte , const char * f_name )
{
	//FILE * fp = fopen_s( f_name , "r");
	FILE* fp = NULL;
	
#ifdef WIN32_VERSION
	fopen_s(&fp, f_name, "r");
#endif

#ifdef LINUX_VERSION
	fp = fopen(f_name, "r");
#endif

	if( NULL == fp ) return -1;
	unsigned int r = rb_byte_fget( fp ,  v , n_byte );
	fclose( fp );
	if( r != n_byte ) return -2;
	return 0;
}



int rb_byte_from_binfile( unsigned char *v , unsigned int n_byte , const char * f_name )
{
	//FILE * fp = fopen( f_name , "r");

	FILE* fp = NULL;
	
#ifdef WIN32_VERSION
	fopen_s(&fp, f_name, "r");
#endif

#ifdef LINUX_VERSION
	fp = fopen(f_name, "r");
#endif

	if( NULL == fp ) return -1;
	unsigned int r = fread( v,  1 , n_byte , fp );
	fclose( fp );
	if( r != n_byte ) return -2;
	return 0;
}



////////////////////////////////////////////////////////////////////


int rb_byte_read_file_onebuffer(unsigned char** msg, unsigned int* len, const char* f_name)
{

	FILE* fp = NULL;
	unsigned int fsize = 0;
	unsigned char *temp = NULL;

#ifdef WIN32_VERSION
	fopen_s(&fp, f_name, "rb");
#endif

#ifdef LINUX_VERSION
	fp = fopen(f_name, "rb");
#endif

	if (NULL == fp) 
		return -3;

	if (fseek(fp, 0, SEEK_SET))
	{
		return 1;
	}
	if (fseek(fp, 0, SEEK_END))
	{
		return 2;
	}
	fsize = ftell(fp);
	if (fseek(fp, 0, SEEK_SET))
	{
		return 1;
	}
	
	
		
	temp = (unsigned char*)rb_safe_malloc(fsize+1);

	unsigned int n_read = 0;
	

n_read = fread(temp, 1, fsize, fp);
	

	msg[0] = temp;
	if (NULL == msg[0])
		return -1;

	if (n_read != fsize)
		return -1;

	*len = n_read;

	fclose(fp);
	return 0;
}

int rb_byte_read_file( unsigned char ** msg , unsigned int * len , const char * f_name )
{

	if( NULL == msg[0] ) {
		msg[0] = (unsigned char *)rb_safe_malloc(4096);
	
		if( NULL == msg[0] ) return -1;
	} else {
		return -2;
	}


	FILE* fp = NULL;
	
#ifdef WIN32_VERSION
	fopen_s(&fp, f_name, "rb");
#endif

#ifdef LINUX_VERSION
	fp = fopen(f_name, "rb");
#endif

	if( NULL == fp ) return -3;

	unsigned int size = 4096;
	unsigned int total_read = 0;

	unsigned int n_read = 0;
	while( 0 != (n_read = fread( msg[0]+total_read , 1 , 4096 , fp ) ) ) 
	{
		total_read += n_read;
		if( 4096 != n_read ) 
			break;
		if( total_read + 4096 > size ) 
		{
			size *= 2;
			msg[0] = rb_safe_realloc( msg[0] , (size_t)size );
			if( NULL == msg[0] ) 
				return -1;
		}
	}
	len[0] = total_read;
	fclose( fp );
	return 0;
}

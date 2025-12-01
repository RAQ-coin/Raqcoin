#ifndef rb__UTILS_H_
#define rb__UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include "rb_core_config.h"

#ifdef  __cplusplus
extern  "C" {
#endif



int rb_byte_fdump(FILE * fp, const char * extra_msg , const unsigned char *v, unsigned int n_byte, int format);

int rb_byte_fget( FILE * fp, unsigned char *v , unsigned int n_byte );

int rb_byte_from_file( unsigned char *v , unsigned int n_byte , const char * f_name );

int rb_byte_from_binfile( unsigned char *v , unsigned int n_byte , const char * f_name );


int rb_byte_read_file( unsigned char ** msg , unsigned int * len , const char * f_name );

int rb_byte_read_file_onebuffer(unsigned char** msg, unsigned int* len, const char* f_name);
#ifdef  __cplusplus
}
#endif



#endif // rb__UTILS_H_



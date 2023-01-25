#ifndef _COMMON_FNCS_H
#define _COMMON_FNCS_H
/*
   common_fncs.h 
   
   functions used by all instructions
*/
#include <stdio.h>

void standard_init(int *cs,FILE **stream,char *name);
void standard_clean_up(int cs,FILE *stream);

#endif // _COMMON_FNCS_H

/*
   functions used by all instructions
*/

#include "cliser.h"
#include "DV.h"

void standard_init(int *cs,FILE **stream,char *name)
{
   if (!getenv("DVHOST")) 
   {
      printf("%s: Environment variable DVHOST not set\n",name);
      exit(-1);
   }
                                          
   if ((*cs=ser0_connect(getenv("DVHOST"),DV_PORT))<0) 
   {
      printf("%s: Connect to '%s' failed\n",name,getenv("DVHOST"));
      exit(-1);
   }

   if (!(*stream=fdopen(*cs,"w"))) 
   {
      printf("%s: fdopen(%d) failed\n",name,*cs);
      exit(-1);
   }

   return;
}


void standard_clean_up(int cs,FILE *stream)
{
   fclose(stream);
   close(cs);
}     

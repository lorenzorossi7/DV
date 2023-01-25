/*
   functions used by all instructions
*/

#include "cliser.h"
#include "DVault.h"

void standard_init(int *cs,FILE **stream,char *name,char *DVHOST, char *DVPORT)
{
   int DV_PORT;
   int ltrace=0;

   if (!getenv(DVHOST)) 
   {
      printf("%s: Environment variable %s not set\n",name,DVHOST);
      exit(-1);
   }
                                          
   if( ! (getenv(DVPORT) && (sscanf(getenv(DVPORT),"%d",&DV_PORT) == 1)) )
   {
      if (!(strcmp(DVPORT,"DVRPORT"))) DV_PORT = DEFAULT_DVR_PORT;
      else DV_PORT = DEFAULT_DV_PORT;
   }
   if( ltrace ) printf("standard_init: DV_PORT = %d\n",DV_PORT);

   if ((*cs=ser0_connect(getenv(DVHOST),DV_PORT))<0) 
   {
      printf("%s: Connect to '%s' failed\n",name,getenv(DVHOST));
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

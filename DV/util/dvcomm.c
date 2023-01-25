/*
 issues a command to DV
*/

#include "cliser.h"
#include "common_fncs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdf_priv.h>
#include <math.h>
#include "DVault.h"

int main(int argc,char **argv)
{
   FILE *stream;
   int cs,rank=1,dsize=1,csize=2,shape[1]={1};
   char *command;
   double time=0;
   double data[1]={0};
   double coords[2]={0,1};
 
   if (argc!=2)
   {
      printf("usage: %s command\n",argv[0]);
      exit(-1);
   }

   if (!(command=malloc((strlen(argv[1])+strlen(DV_CALC_TAG)+2)*sizeof(char))))
   {
      printf("%s error: out of memory\n",argv[0]);
      exit(-1);
   }

   sprintf(command,"%s%s;",DV_CALC_TAG,argv[1]);

   standard_init(&cs,&stream,argv[0],"DVHOST","DVPORT");

   if (!low_write_sdf_stream(stream,command,time,rank,dsize,csize,"","",shape,coords,data))
      printf("low_write_sdf_stream error sending command %s\n",command);

   standard_clean_up(cs,stream);
 
   exit(0);
}     

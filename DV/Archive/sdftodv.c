/*
 sends sdf file(s) to the data-vault
*/

#include "cliser.h"
#include "common_fncs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdf_priv.h>

#define IFL if (ltrace)
#define MAX_IVECL 1000000

int main(int argc,char **argv)
{
   FILE *stream;
   int i,j,cs,version,rank,dsize,csize,*shape;
   int ltrace=0;
   char *cnames,*pname,*tag,*ivec,*ivec2,iivec;
   int tivec[MAX_IVECL],it; 
   double time;
   double *data;
   double *coords,*bbox; 
   gft_sdf_file_data *gp;
 
   if (argc<2)
   {
      printf("usage: %s [-i 'ivec(1-indexed)'] <sdf_file_name> ... \n",argv[0]);
      exit(-1);
   }

   standard_init(&cs,&stream,argv[0]);

   ivec=0;
   ivec2=0;
   iivec = 0;
   for(i=1; i<(argc-1); i++) { if (!(strcmp(argv[i],"-i"))) {ivec=argv[i+1]; iivec=i;} }
   if (ivec)
   {
      if (!(ivec2=(char *)malloc(strlen(ivec)+4)))
      {
         printf("out of memory trying to allocate %i bytes\n",strlen(ivec)+4);
         exit(-1);
      }
      strcpy(ivec2,"t:=");
      strcpy(&ivec2[3],ivec);
      IFL printf("ivec:%s\n\n",ivec2);
   }
            
   for(i=1; i<argc; i++)
   {
      IFL printf("file:%s\n\n",argv[i]);
      it=1;
      if (ivec2)
      {
         if (!(sget_ivec_param(ivec2,"t",tivec,MAX_IVECL))) 
         {
            printf("init_s_iter: sget_ivec_param failed for <%s>\n",ivec2);
            free(ivec2); ivec2=0;
         }
         else fixup_ivec(1,MAX_IVECL,0,tivec);
      }
      if ((!ivec2) || (i!=iivec && i!=(iivec+1)))
      {
         if(!(gp=gft_open_sdf_stream(argv[i])))
            printf("gft_open_sdf_stream error with file %s\n",argv[i]);
         else
         {
            while(low_read_sdf_stream(1,gp->fp,&time,&version,&rank,&dsize,&csize,&pname,&cnames,
                                     &tag,&shape,&bbox,&coords,&data))
            {
               IFL
               {
                  printf("data to be sent:\ntime=%f\nversion=%i\nrank=%i\n",time,version,rank);
                  printf("dsize=%i\ncsize=%i\npname=%s\ncnames=%s\n",dsize,csize,pname,cnames);
                  printf("tag=%s\nshape=",tag);
                  for(j=0;j<rank;j++) printf("%i ",shape[j]);
                  printf("\nbbox=");
                  for(j=0;j<2*rank;j++) printf("%f ",bbox[j]);
                  printf("it=%i\n",it);
                  printf("\n");
               }
               if (!(ivec2) || do_ivec(it,MAX_IVECL,tivec))
               {
                  if (!low_write_sdf_stream(stream,pname,time,rank,dsize,csize,cnames,"",shape,coords,data))
                      printf("low_write_sdf_stream error sending %s\n",argv[i]);
               }
               if (pname) free(pname);
               if (cnames) free(cnames);
               if (coords) free(coords);
               if (data) free(data);
               if (bbox) free(bbox);
               if (tag) free(tag);
               if (shape) free(shape);
               it++;
            }
         }
      }
      gft_close_sdf_stream(argv[i]);
   }

   if (ivec2) free(ivec2);

   standard_clean_up(cs,stream);
 
   exit(0);
}     

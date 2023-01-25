/*
 sends sdf file(s) to the data-vault
*/

#include "cliser.h"
#include "common_fncs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <bbhutil.h> 
#include <sdf_priv.h>
#include <math.h>

#define IFL if (ltrace)
#ifdef DARWIN
#define MAX_IVECL 100000
#else
#define MAX_IVECL 1000000
#endif
#define MAX_DATA 1.0e20

int nans(double *data, int size)
{
   int i;

   for (i=0; i<size; i++) if (fabs(data[i])>MAX_DATA || !finite(data[i])) return 1;

   return 0;
}

int main(int argc,char **argv)
{
   FILE *stream;
   int i,j,cs,version,rank,dsize,csize,*shape;
   int ltrace=0;
   char *cnames,*pname,*tag,*ivec,*ivec2,iivec;
   int tivec[MAX_IVECL],it; 
   int n,in,cn,cne,skip_nan,iskip_nan,nan,ioname,remote=0,iremote,it12,skip_rest;
   char  *oname = (char *) NULL;
   double time;
   double *data;
   double *coords,*bbox,t1,t2; 
   gft_sdf_file_data *gp;
   char file_name[1024];
 
   if (argc<2)
   {
      printf("usage: %s [-r] [-i 'ivec(1-indexed)'] [-t t1 t2] [-n oname] [-N nmax] [-s] sdf_file_name ... \n",argv[0]);
      printf("\n");
      printf("   -i ivec  -- use ivec (1 based) for output control\n");
      printf("   -t t1 t2  -- only send data with a time between t1 and t2 inclusive\n");
      printf("   -n oname -- name ALL data sets oname\n");
      printf("   -N nmax -- send files of the form sdf_file_name_i, i = 1 .. nmax\n");
      printf("   -s -- removes data containing Nan's output stream\n");
      printf("   -r -- send to server define by (DVRHOST,DVRPORT) environment variables instead of (DVHOST,DVPORT)\n");
      exit(-1);
   }

   ivec=0;
   it12=0;
   ivec2=0;
   iivec=0;
   in=0;
   n=0;
   skip_nan=0;
   iskip_nan=0;
   ioname=0;
   for(i=1; i<(argc-1); i++) { if (!(strcmp(argv[i],"-i"))) {ivec=argv[i+1]; iivec=i;} }
   for(i=1; i<(argc-1); i++) { if (!(strcmp(argv[i],"-n"))) {oname=argv[i+1]; ioname=i;} }
   for(i=1; i<(argc-1); i++) { if (!(strcmp(argv[i],"-N"))) {n=atoi(argv[i+1]); in=i;} }
   for(i=1; i<(argc-1); i++) { if (!(strcmp(argv[i],"-t"))) {t1=atof(argv[i+1]); t2=atof(argv[i+2]); it12=i;} }
   for(i=1; i<(argc); i++) { if (!(strcmp(argv[i],"-s"))) {skip_nan=1; iskip_nan=i;} }
   for(i=1; i<(argc); i++) { if (!(strcmp(argv[i],"-r"))) {remote=1; iremote=i;} }
   if (n<0) { printf("error ... n should be greater than zero ... ignoring it\n"); n=0; }

   if (remote) standard_init(&cs,&stream,argv[0],"DVRHOST","DVRPORT");
   else standard_init(&cs,&stream,argv[0],"DVHOST","DVPORT");

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

    IFL printf("%s: oname='%s'\n",argv[0],oname?oname:"NULL");
           
   for(i=1; i<argc; i++)
   {
      if ( ((!ivec2) || (i!=iivec && i!=(iivec+1))) &&
           (!n || (i!=in && i!=(in+1))) && 
           (!it12 || (i!=it12 && i!=(it12+1) && i!=(it12+2))) && 
           (!skip_nan || (i!=iskip_nan)) && 
           (!remote || (i!=iremote)) && 
           (!oname || ((i!=ioname) && (i!=(ioname+1)))) )
      {
         if (n) cne=n; else cne=1;
         for (cn=0; cn<cne; cn++)
         {
            if (n) sprintf(file_name,"%s_%i",argv[i],cn);
            else sprintf(file_name,"%s",argv[i]);

            IFL printf("file:%s\n\n",file_name);
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
            if(!(gp=gft_open_sdf_stream(file_name)))
               printf("gft_open_sdf_stream error with file %s\n",file_name);
            else
            {
               skip_rest=0;
               while(!skip_rest &&
                     low_read_sdf_stream(1,gp->fp,&time,&version,&rank,&dsize,&csize,&pname,&cnames,
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
                     if (ivec2) printf("do_ivec:%i\n",do_ivec(it,MAX_IVECL,tivec));
                     printf("\n");
                  }
                  nan=0;
                  if ((!(ivec2) || do_ivec(it,MAX_IVECL,tivec)) &&
                      (!it12 || (time>=t1 && time<=t2)) &&
                      (!skip_nan || !(nan=nans(data,dsize))))
                  {
                     if (cnames)
                     { 
                        if (!low_write_sdf_stream(stream,oname?oname:pname,time,rank,dsize,csize,cnames,"",shape,coords,data))
                           printf("low_write_sdf_stream error sending %s\n",file_name);
                     }
                     else 
                     {
                        if (!low_write_sdf_stream(stream,oname?oname:pname,time,rank,dsize,csize,"x|y|z","",shape,coords,data))
                            printf("low_write_sdf_stream error sending %s\n",file_name);
                     }
                  }
                  if (it12 && time>t2) skip_rest=1;
                  if (skip_nan && nan)
                  {
                     printf("warning : index %i of <%s> has data > %lf, or NaNs ... skipping\n",it,file_name,MAX_DATA);
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
               gft_close_sdf_stream(file_name);
            }
         }
      }
   }

   if (ivec2) free(ivec2);

   standard_clean_up(cs,stream);
 
   exit(0);
}     

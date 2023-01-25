/*
utility to read a (t,x) text file and save it as a 1D sdf file
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <bbhutil.h> 
#include <math.h>

#define BUFSIZE 1025

int main(int argc,char **argv)
{
   FILE *file;
   double *t=0,*x=0;
   char buffer[BUFSIZE],c,*in,*out;
   int t_pos,b_pos,tcol,xcol,sizet,col,comment;
   int col_start[BUFSIZE],line;

   if (!(argc==3 || argc==5))
   {
      printf("usage: %s [tcol xcol] in.txt out.sdf\n",argv[0]);
      printf("\n");
      printf("   [tcol xcol] -- column numbers for t & x components of data\n");
      printf("               -- default 1 and 2\n");
      printf("   in.txt -- input text file name\n");
      printf("   out.sdf -- output sdf file name\n");
      printf("\nInput lines beginning with #,!,$ or % are ignored\n\n");
      exit(-1);
   }

   tcol=1;
   xcol=2;

   if (argc==3)
   {
      in=argv[1];
      out=argv[2];
   }
   else
   {
      tcol=atoi(argv[1]);
      xcol=atoi(argv[2]);
      in=argv[3];
      out=argv[4];
   }

   printf("input file:%s\n",in);
   printf("output file:%s\n",out);
   printf("(t,x) in columns (%i,%i)\n\n",tcol,xcol);

   if (!(file=fopen(in,"r")))
   {
      printf("error opening file %s\n",in);
      exit(-1);
   }

   sizet=0;b_pos=0,comment=0;
   while((c=fgetc(file))!=EOF) 
   {
      if (b_pos==0 && (c=='!' || c=='#' || c=='$' || c=='%')) comment=1;
      else if (b_pos==0) comment=0;
      if (c=='\n') { b_pos=0; if (!comment) sizet++; }
      else b_pos++;
   }
   fclose(file);
   if (!sizet)
   {
      printf("error ... file is empty\n");
      exit(-1);
   }

   if (!(t=(double *)malloc(2*sizeof(double)*sizet)))
   {
      printf("error ... out of memory\n");
      exit(-1);
   }

   x=&t[(sizet)];

   if (!(file=fopen(in,"r")))
   {
      printf("error re-opening file %s\n",in);
   }

   b_pos=0;
   t_pos=0;
   line=1;
   while((buffer[b_pos]=fgetc(file))!=EOF && b_pos<BUFSIZE && t_pos<(sizet))
   {
      c=buffer[b_pos];
      if (b_pos==0 && (c=='!' || c=='#' || c=='$' || c=='%')) comment=1;
      else if (b_pos==0) comment=0;

      if (c=='\n' && !comment)
      {
         b_pos=0;
         col=1;
         while(buffer[b_pos]!='\n')
         {
            while(buffer[b_pos]==' ' || buffer[b_pos]=='\t' ) b_pos++;
            col_start[col-1]=b_pos;
            col++;
            while(!(buffer[b_pos]==' ' || buffer[b_pos]=='\t' || buffer[b_pos]=='\n')) b_pos++;
         }
	 if (col<=tcol || col<=xcol) 
	 {
	    printf("Warning : line %i does not have enough columns of data ... skipping\n",line);
	    sizet--;
	 }
	 else
	 {
            sscanf(&buffer[col_start[tcol-1]],"%lf",&t[t_pos]);
            sscanf(&buffer[col_start[xcol-1]],"%lf",&x[t_pos]);
            t_pos++;
	 }
         b_pos=0;
      }
      else if (c=='\n') b_pos=0; else b_pos++;

      if (b_pos==0) line++;
   }

   fclose(file);

   gft_out_full(out,0.0e0,&sizet,"t|x",1,t,x);

   exit(0);
}     

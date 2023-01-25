//=============================================================================
// instr.c 
// 
// implementation of some instructions
//=============================================================================

#include "reg.h"
#include "instr.h"
#include "misc.h"
#include "cliser.h"
#ifndef DARWIN
#include <malloc.h>
#endif 
#include <stdio.h>
#include <stdlib.h>
#include "sdf.h"
#include "sdf_priv.h"
#include "DVault.h"
#include "DV_calc/parse.h"
//#include "bbhutil.h"

//=============================================================================
// reads an SDF from stream
//=============================================================================
#define PROG_TRACE_LEN 80
void instr_read_sdf_stream(FILE *stream)
{
   char name[MAX_NAME_LENGTH];
   char prog_trace[PROG_TRACE_LEN];
   int ltrace=1,first=1;
   int n,len,failed;
   int *shape;
   int rank,nrank,i,ind,dsize,csize,version,ps,ctype,j,k;
   char *cnames,*pname,*tag,*cbuf[MAX_DIM],*cq;
   double time;
   double *data;
   double *coords,*bbox,*p,dx0,*cp,*ccp;
   reg *r;

   while(low_read_sdf_stream(1,stream,&time,&version,&rank,&dsize,&csize,&pname,
                             &cnames,&tag,&shape,&bbox,&coords,&data))
   {
      if (ltrace>1)
      {
         printf("data from stream:\ntime=%f\nversion=%i\nrank=%i\n",time,version,rank);
         printf("dsize=%i\ncsize=%i\npname=%s\ncnames=%s\n",dsize,csize,pname,cnames);
         printf("tag=%s\nshape=",tag);
         for(i=0;i<rank;i++) printf("%i ",shape[i]);
         printf("\nbbox=");
         for(i=0;i<2*rank;i++) printf("%f ",bbox[i]);
         printf("\n");
      }

      if (!(strncmp(pname,DV_CALC_TAG,strlen(DV_CALC_TAG))))
      {
         IFL
         {
            strncpy(prog_trace,pname+strlen(DV_CALC_TAG),PROG_TRACE_LEN-4);
            prog_trace[strlen(prog_trace)]=0;
            printf("dv_service: program = {%s",prog_trace);
            if (strlen(pname+strlen(DV_CALC_TAG))>PROG_TRACE_LEN-4) printf(" ...} \n");
            else printf("}\n");
         }
         instr_ex_dv_calc_prog(pname);
         ccp=0;
      }
      else
      {
         IFL if(first) printf("dv_service: reading sdf stream <%s>\n",pname); first=0;
         //=======================================================================
         // check for perimeter/curvilinear, and if non-uniform, add as a semi_uniform
         // (rank=2) for now. rank 1's are added as curvilinear
         //=======================================================================
         ps=0; for (i=0;i<rank;i++) ps+=shape[i];
         ctype=COORD_UNIFORM;
         cp=bbox;ccp=0;
         if (rank==1 && csize==ps)
         {
            cp=coords;
            ctype=COORD_CURVILINEAR;
         }
         else if (csize==ps)
         {
            ps=0;
            cp=coords;
            for (i=0;i<rank && !ps;i++)
            {
               dx0=cp[1]-cp[0];
               for (j=1;j<shape[i] && !(ps);j++) if (fabs(cp[j]-cp[j-1]-dx0)>FUZZY) ps=1;
               cp+=shape[i];
            }
            cp=bbox;
            if (ps)
            {
               printf("instr_read_sdf_stream: perimeter grid detected ... converting to semi-uniform\n");
               if (!(ccp=malloc(sizeof(double)*rank*dsize))) 
               {
                  printf("instr_read_sdf_stream: out of memory \n");
               }
               else if (rank==2)
               {
                  p=ccp;
                  ctype=COORD_SEMI_UNIFORM;
                  for (i=0;i<shape[0];i++)
                     for (j=0; j<shape[1];j++) 
                     {
                        p[shape[0]*shape[1]+i+j*shape[0]]=coords[shape[0]+j]; 
                        p[i+j*shape[0]]=coords[i]; 
                     }
               }
   /*            else if (rank==3)
               {
                  p=ccp;
                  ctype=COORD_SEMI_UNIFORM;
                  for (i=0;i<shape[0];i++)
                     for (j=0; j<shape[1];j++) 
                        for (k=0; k<shape[2];k++) 
                        {
                           p[i+j*shape[0]+k*shape[0]*shape[1]]=coords[i]; 
                           p[shape[0]*shape[1]*shape[2]+i+j*shape[0]+k*shape[0]*shape[1]]
                              =coords[shape[0]+j]; 
                           p[2*shape[0]*shape[1]*shape[2]+i+j*shape[0]+k*shape[0]*shape[1]]
                              =coords[shape[0]+shape[1]+k]; 
                        }
               }*/
               else
               {
                  printf("instr_read_sdf_stream: cannot handle perimeter rank>2 yet\n");
                  free(ccp);
                  ccp=0;
               }
            }
         }
         else if (csize==rank*dsize)
         {
            printf("instr_read_sdf_stream: curvilinear grid ... converting to semi-uniform\n");
            cp=bbox;
            ccp=coords;
            ctype=COORD_SEMI_UNIFORM;
         }
   
         if (!add_grid(pname,time,rank,ctype,shape,cp,ccp,data))
         {
            printf("instr_read_sdf_stream: ERROR adding grid to register %s\n",pname);
         }
         else
         {
            if (!(r=find_reg(pname,1,1)))
            {
               printf("instr_read_sdf_stream: BUG - cannot find register %s that must be there \n",pname);
               return;
            }
            // unpack coordinate names. decoding code lifted from Matt's AcceptSDF0.c routine.
            len=strlen(cnames);
            for (i=0,failed=0; i<rank; i++)
               if (!(cbuf[i]=(char *)malloc(sizeof(char)*(len+1))))
               {
                  printf("instr_read_sdf_stream: out of memory ... cannot allocate cbuf\n"); failed=1;
               }
            if (!failed)
            {
               switch(rank)
               {
                  // sometimes (default?) cnames is "x|y|z" regardless of rank ... need to strip if that is 
                  // the case
                  case 1:
                     if (sscanf(cnames,"%s",cbuf[0])!=1) failed=1;
                     else { cq=cbuf[0]; while(*cq++) if (*cq=='|') *cq=0; } 
                     break;
                  case 2:
                     if (sscanf(cnames,"%[^|]|%[^|]",cbuf[0],cbuf[1])!=2) failed=1;
                     else { cq=cbuf[1]; while(*cq++) if (*cq=='|') *cq=0; }
                     break;
                  case 3:
                     if (sscanf(cnames,"%[^|]|%[^|]|%[^|]",cbuf[0],cbuf[1],cbuf[2])!=3) failed=1;
                     break;
                  default:
                     printf("instr_read_sdf_stream: unable to convert rank %i coordinate names\n",rank);
                     failed=1;
               }
               if (!failed)
                  for(i=0; i<rank; i++)
                  {
                     if (r->coord_names[i+1]) free(r->coord_names[i+1]); r->coord_names[i+1]=0;
                     len=min(MAX_NAME_LENGTH,strlen(cbuf[i]));
                     if (!(r->coord_names[i+1]=(char *)malloc(sizeof(char)*(len+1))))
                        printf("instr_read_sdf_stream: out of memory ... cannot allocate coordinate name\n");
                     else
                     {
                        strncpy(r->coord_names[i+1],cbuf[i],len); 
                        (r->coord_names[i+1])[len]=0; 
                     }
                  }
               else
                  printf("instr_read_sdf_stream: Error converting coordinate name %s\n",cnames);
            }
   
            for(i=0; i<rank; i++) if (cbuf[i]) free(cbuf[i]);
   
            FREE_REG(r);
         }
      }
   
next:
    
      if (pname) free(pname);
      if (cnames) free(cnames);
      if (tag) free(tag);
      if (shape) free(shape);
      if (bbox) free(bbox);
      if (coords) free(coords);
      if (data) free(data);
      if (ccp && ccp!=coords) free(ccp);
   }

   return;
}

//=============================================================================
// reads an SDF from a file
// 
// if (as_perim), then the coordinates are set up as perimeter (or curvilinear
// for a 1D file)
// optionally specify a new name
//=============================================================================
void instr_read_sdf(char *file_name, int as_perim, char *new_name)
{
   char name[MAX_NAME_LENGTH];
   int ltrace=0;
   int n;
   int shape[MAX_DIM];
   int rank,nrank,i,ind,size,csize,ctype;
   char cnames[MAX_DIM*MAX_NAME_LENGTH];
   double time;
   double *data;
   double *coords;
   double uniform_coords[MAX_DIM*2];

   IFL
   {
      printf("instr_read_sdf: to do list -- \n");
      printf("\t set coordinate names \n\t handle non-uniform grids\n");
      printf("\t multiple grid functions per sdf ??\n");
   }

   if (new_name && strlen(new_name))
   {
      strncpy(name,new_name,MAX_NAME_LENGTH);
   }
   else if (!(gft_read_name(file_name,1,name)))
   {
      printf("instr_read_sdf: ERROR -- gft_read_name() failed for file_name=%s\n",file_name);
      return;
   }

   IFL printf("\t name=%s\n",name);
   
   if (!(gft_read_rank(file_name,1,&rank)))
   {
      printf("instr_read_sdf: ERROR -- gft_read_rank() failed for file_name=%s\n",file_name);
      return;
   }
   
   IFL printf("\t rank=%i\n",rank);

   n=1; 
   
   while(gft_read_shape(file_name,n,shape))
   {
      size=1; i=rank; while(i--) size*=shape[i];
      csize=0; i=rank; while(i--) csize+=shape[i];
      if (!(data=(double *)malloc(size*sizeof(double))))
      {
         printf("instr_read_sdf: out of memory. file_name=%s, n=%d, size=%d\n",file_name,n,size);
         return;
      }
      if (!(coords=(double *)malloc(csize*sizeof(double))))
      {
         free(data);
         printf("instr_read_sdf: out of memory. file_name=%s, n=%d, csize=%d\n",file_name,n,csize);
         return;
      }

      if (!(gft_read_full(file_name,n,shape,cnames,rank,&time,coords,data)))
      {
         printf("instr_read_sdf: error reading time level %d from %s\n",n,file_name);
      }
      else
      {
         i=0;ind=0;
         while(i<rank)
         {
            uniform_coords[2*i]=coords[ind];
            ind=ind+shape[i];
            uniform_coords[2*i+1]=coords[ind-1];
            i++;
         }

         if (as_perim)
         {
            if (rank==1) ctype=COORD_CURVILINEAR; else ctype=COORD_PERIMETER;
            if (!(add_grid(name,time,rank,ctype,shape,coords,0,data)))
               printf("instr_read_sdf: add_grid failed for time level %d of %s\n",n,file_name);
         }
         else
         {
            if (!(add_grid(name,time,rank,COORD_UNIFORM,shape,uniform_coords,0,data)))
               printf("instr_read_sdf: add_grid failed for time level %d of %s\n",n,file_name);
         }
      }
      
      free(data);
      free(coords);
      n=n+1;
   }

   IFL printf("READ_SDF: Added %i grid(s) to %s\n",n-1,name);

   return;
}

//=============================================================================
// writes an SDF from a file (0 if failure)
//=============================================================================
int instr_write_sdf(char *name, char *file_name, int lock_rl)
{
   int ltrace=0;
   int n,i,ret=0;
   char cnames[MAX_DIM*MAX_NAME_LENGTH];
   reg *r;
   time_str *ts;
   level *l;
   grid *g;
   gft_sdf_file_data *gp=0;
   int csize,dsize;

   if (!(r=find_reg(name,lock_rl,lock_rl)))
   {
      printf("instr_write_sdf: register %s does not exist \n",name);
      return 0;
   }

   cnames[0]=0;
   for(i=1;i<=MAX_DIM;i++)
      if (r->coord_names[i])
      {
         if (strlen(cnames)==0) sprintf(cnames,"%s",r->coord_names[i]);
         else sprintf(&cnames[strlen(cnames)],"|%s",r->coord_names[i]);
      }

   IFL printf("instr_write_sdf: saving %s to file %s\n",name,file_name);

   if(!(gp=gft_create_sdf_stream(file_name)))
   {
      printf("instr_write_sdf: gft_create_sdf_stream(%s) failed\n",file_name);
      goto cleanup;
   }

   ts=r->ts;
   while(ts)
   {
      l=ts->levels;
      while(l)
      {
         g=l->grids;
         while(g)
         {
            dsize=sizeof_data(g);
            csize=sizeof_coords(g);
            if (g->ccoords)
               printf("instr_write_sdf: WARNING ... curvilinear coordinate info not saved for semi-uniform grids\n");
             
            if (!(low_write_sdf_stream(gp->fp,name,ts->time,g->dim,dsize,csize,cnames,"",g->shape,g->coords,g->data)))
            {
               printf("instr_write_sdf: Error saving register %s to file %s\n",name,file_name);
               goto cleanup;
            }
            g=g->next;
         }
         l=l->next;
      }
      ts=ts->next;
   }

   ret=1;
cleanup:
   FREE_REG(r);
   if (gp) gsfd_close(gp);
   return ret;
}

//=============================================================================
// sends (to BBHHOST) a register via gft_send_sdf_stream_bbox
//
// NOT TESTED YET!! (based upon send_to_accept experience, probably need
// to use low level stream writes)
//=============================================================================
void instr_write_sdf_stream(char *name)
{
   reg *r;
   grid *g;
   time_str *ts;
   level *l;

   if (!(r=find_reg(name,1,1)))
   {
      printf("instr_send_sdf: register %s does not exist\n",name);
      return;
   }

   ts=r->ts;
   while(ts)
   {
      l=ts->levels;
      while(l)
      {
         g=l->grids;
         while(g)
         {
            if(!(gft_send_sdf_stream_bbox(name,ts->time,g->shape,g->dim,g->coords,g->data)))
            {
               printf("instr_write_sdf_stream: gft_send_sdf_stream_bbox failed\n");
               return;
            }
            g=g->next;
         }
         l=l->next;
      }
      ts=ts->next;
   }

   FREE_REG(r);

   return;
}
   
//=============================================================================
// the following routines send a register (name) to different server.
// it the GUI is open, this is defined as the primary server, and
// info is routed to the 'remote' server, as declared in environment variables 
// (DVRHOST,DVRPORT). Otherwise, this is the remote server, and info
// is routed to (DVHOST,DVPORT)
//=============================================================================
int open_rhost(int *cs,FILE **stream)
{
   int DV_PORT;
   char *DV_HOST;

   if (no_GUI)
   {
      if (!(DV_HOST=getenv("DVHOST")))
      {
         printf("open_rhost: Environment variable DVHOST not set ... cannot route\n");
         return 0;
      }
      if( ! (getenv("DVPORT") && (sscanf(getenv("DVPORT"),"%d",&DV_PORT) == 1)) ) 
         DV_PORT = DEFAULT_DV_PORT;
   }
   else
   {
      if (!(DV_HOST=getenv("DVRHOST")))
      {
         printf("open_rhost: Environment variable DVRHOST not set ... cannot route\n");
         return 0;
      }
      if( ! (getenv("DVRPORT") && (sscanf(getenv("DVRPORT"),"%d",&DV_PORT) == 1)) ) 
         DV_PORT = DEFAULT_DVR_PORT;
   }

   if ((*cs=ser0_connect(DV_HOST,DV_PORT))<0)
   {
      printf("open_rhost: Connect to '%s' failed\n",DV_HOST);
      return 0;
   }

   if (!(*stream=fdopen(*cs,"w")))
   {
      printf("open_rhost: fdopen(%d) failed\n",*cs);
      close(*cs);
      return 0;
   }

   return 1;
}

void close_rhost(int cs,FILE *stream)
{
   fclose(stream);
   close(cs);
}

void instr_route(char *name)
{
   int cs;
   FILE *stream;
   reg *r;
   int dsize,csize,i;
   char cnames[MAX_DIM*MAX_NAME_LENGTH],cname[10],*cnamei;
   double *coords;
   time_str *ts;
   level *l;
   grid *g;

   if (!(r=find_reg(name,1,1))) { printf("instr_route: register <%s> not found\n",name); return; }

   if (!(open_rhost(&cs,&stream))) { FREE_REG(r); return; }

   cnames[0]=0;
   for(i=1;i<=MAX_DIM;i++)
   {
      if (r->coord_names[i]) cnamei=r->coord_names[i];
      else { cnamei=cname; sprintf(cname,"x%i",i); }

      if (strlen(cnames)==0) sprintf(cnames,"%s",cnamei);
      else sprintf(&cnames[strlen(cnames)],"|%s",cnamei);
   }

   ts=r->ts;
   while(ts)
   {
      l=ts->levels;
      while(l)
      {
         g=l->grids;
         while(g)
         {
            dsize=sizeof_data(g);
            csize=sizeof_coords(g);
            coords=g->coords;
            if (g->ccoords)
            {
               csize=dsize*g->dim;
               coords=g->ccoords;
            }
             
            if (!(low_write_sdf_stream(stream,name,ts->time,g->dim,dsize,csize,cnames,"",g->shape,coords,g->data)))
            {
               printf("route: low_write_sdf_stream error with register %s\n",name);
               goto cleanup;
            }
            g=g->next;
         }
         l=l->next;
      }
      ts=ts->next;
   }

cleanup:
   FREE_REG(r);
   close_rhost(cs,stream);
}

//=============================================================================
// executes the DV_calc program specified via the null terminated string
// prog
//=============================================================================
int DV_calcwrap();
void instr_ex_dv_calc_prog(char *prog)
{
   int exit=0;
   int ltrace=0;

   DV_calc_input=DV_calc_input_ptr=(prog+strlen(DV_CALC_TAG));

   IFL printf("instr_ex_dv_calc_prog ... executing:\n%s\n",DV_calc_input);

   if (!DV_calcparse())
   {
      exit=eval_stmnt_list();
      free_stmnt_list();
      refresh_GUI_now=1;
   }
   else
   {
      printf("instr_ex_dv_calc_prog: parser returned an error executing program:\n%s\n",prog);
      DV_calcwrap();
   }
   if (exit<0) 
   {
      if (!no_GUI) printf("instr_ex_dv_calc_prog: cannot stop DV remotely when the GUI is running\n");
      else stop_dv_service=1;
   }
}

//=============================================================================
// diagnostic routine
//=============================================================================

void instr_dump_all(void)
{
   reg *r;
   time_str *ts;
   grid *g;
   level *l;
   int i,j,k;
   char *name;

   LOCK_REG_LIST;
   
   r=reg_list;

   printf("DUMP_ALL ... contents of data vault:\n");
   printf("====================================\n");

   if (!r) printf("empty\n");
   else
   {
      while(r)
      {
         LOCK_REG(r);
         printf("register %s:\n\n",r->name);
         i=0;
         while(i<=MAX_DIM)
         {
            if (name=r->coord_names[i]) printf("coordinate %i = %s\n",i,name);
            i++;
         }
         ts=r->ts;
         while(ts)
         {
            printf("\ttime %16.10lf\n",ts->time);
            l=ts->levels;
            i=0;
            while(l)
            {
               printf("\t\tlevel %i, dx=%16.10lf\n",i,l->dx);
               g=l->grids;
               j=0;
               printf("\t\t\tgrid \tdim \tshape \t[x0,y0,...]\n");
               printf("\t\t\t---- \t--- \t----- \t-----------\n");
               while(g)
               {
                  printf("\t\t\t%i\t%i\t%i",j,g->dim,g->shape[0]);
                  if (g->dim>1) printf("X%i",g->shape[1]);
                  if (g->dim>2) printf("X%i",g->shape[2]);
                  if (g->dim>3) printf("X...");
                  if (g->coord_type!=COORD_UNIFORM) printf("\t[non-uniform]\n");
                  else
                  {
                     printf("\t[%16.10lf",g->coords[0]);
                     if (g->dim>1) printf(",%16.10lf",g->coords[2]);
                     if (g->dim>2) printf(",%16.10lf",g->coords[4]);
                     if (g->dim>3) printf(",...");
                     printf("]\n");
                  }
                  g=g->next;
                  j=j+1;
               }
               printf("\n");
               i=i+1;
               l=l->next;
            }
            ts=ts->next;
         }
         printf("-------------------\n");
         FREE_REG(r);
         r=r->next;
      }
   }
   printf("====================================\n");

   FREE_REG_LIST;

   return;
}

void instr_mem_usage(void)
{
   reg *r;
   time_str *ts;
   grid *g;
   level *l;
   int i,j,k;
   char *name;

   int d_mem,c_mem,s_mem,ds;

   LOCK_REG_LIST;
   
   r=reg_list;

   printf("==============================================\n");
   printf("Register based memory usage :\n");
   printf("==============================================\n");

   d_mem=c_mem=s_mem=0;

   while(r)
   {
      s_mem+=sizeof(reg);
      LOCK_REG(r);
      ts=r->ts;
      while(ts)
      {
         s_mem+=sizeof(time_str);
         l=ts->levels;
         while(l)
         {
            s_mem+=sizeof(level);
            g=l->grids;
            while(g)
            {
               s_mem+=sizeof(grid);
               c_mem+=sizeof_coords(g)*sizeof(double);
               ds=sizeof_data(g)*sizeof(double);
               d_mem+=ds;
               if (g->coord_type==COORD_SEMI_UNIFORM) c_mem+=ds*g->dim;
               g=g->next;
            }
            l=l->next;
         }
         ts=ts->next;
      }
      FREE_REG(r);
      r=r->next;
   }
   
   printf("structure  : %f \t MB\n",s_mem/1.0e6);
   printf("data       : %f \t MB\n",d_mem/1.0e6);
   printf("coordinate : %f \t MB\n",c_mem/1.0e6);
   printf("\ntotal      : %f \t MB\n",(c_mem+s_mem+d_mem)/1.0e6);
   printf("==============================================\n");

   FREE_REG_LIST;

   return;
}


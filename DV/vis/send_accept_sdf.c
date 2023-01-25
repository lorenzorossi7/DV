/*

to send output to the AcceptXYZ explorer module

*/

#include "DVault.h"
#include "reg.h"
#include "misc.h"
#include <stdio.h>
#include "sdf.h"
#include "sdf_priv.h"
#include "cliser.h"
#ifndef DARWIN
#include <malloc.h>
#endif 
#include <stdlib.h>

//-----------------------------------------------------------------------------
// sends a single grid
//-----------------------------------------------------------------------------
void send_accept_sdf_1g(char *name, double t, grid *g)
{
   int i,cs,dsize,csize;
   FILE *stream;

   if (g->coord_type != COORD_UNIFORM)
   {
      printf("send_accept_sdf_1g: can only send uniform grids as of this time\n");
      return;
   }

   if (!getenv("BBHHOST"))
   {
      printf("send_accept_sdf_1g: Environment variable BBHHOST not set\n");
      return;
   }

   if ((cs=ser0_connect(getenv("BBHHOST"),BBH_PORT0))<0)
   {
      printf("send_accept_sdf_1g: Connect to '%s' failed\n",getenv("DVHOST"));
      return;
   }
 
   if (!(stream=fdopen(cs,"w")))
   {
      printf("send_accept_sdf_1g: fdopen(%d) failed\n",cs);
      return;
   }

   csize=2*g->dim;
   for (dsize=1, i=0; i<g->dim; i++) dsize*=g->shape[i];

   if (!low_write_sdf_stream(stream,name,t,g->dim,dsize,csize,"t|x1|x2|x3","",g->shape,g->coords,g->data))
   {
      printf("send_accept_sdf_1g:low_write_sdf_stream error sending %s\n",name); 
   }

   fclose(stream);
   close(cs); 

   return; 
}

void send_accept_sdf(int send_children)
{
   int i,n,j,k,ltrace=0;
   reg *r;
   time_str *ts;
   level *l;
   grid *g;

   LOCK_REG_LIST;

   r=reg_list;

   while(r)
   {
      LOCK_REG(r);
      ts=r->ts;
      while(ts)
      {
         l=ts->levels;
         while(l)
         {
            g=l->grids;
            while(g)
            {
               if (g->selected||(send_children&&(r->selected||ts->selected||l->selected)))
                  send_accept_sdf_1g(r->name,ts->time,g);
               g=g->next;
            }
            l=l->next;
         }
         ts=ts->next;
      }
      FREE_REG(r);
      r=r->next;
   }
   FREE_REG_LIST;

   return;
}


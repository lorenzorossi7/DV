/*

to send output to the AcceptXYZ explorer module

*/

#include "DV.h"
#include "reg.h"
#include "misc.h"
#include "bbh_xyz.h"
#include <malloc.h>

void send_bbh_xyz(int send_children)
{
   int num,i,n,j,k,ltrace=0;
   reg *r;
   time_str *ts;
   level *l;
   grid *g;
   double *data,x1,x2,x3,dx1,dx2,dx3;

   LOCK_REG_LIST;

   r=reg_list;

   while(r)
   {
      LOCK_REG(r);
      ts=r->ts;
      while(ts)
      {
         l=ts->levels;
         num=0;
         while(l)
         {
            g=l->grids;
            while(g)
            {
               if (g->selected||(send_children&&(r->selected||ts->selected||l->selected)))
               {
                  n=0;
                  if (g->dim<=3 && g->coord_type==COORD_UNIFORM)
                     for(i=0,n=1;i<g->dim;i++) n=n*g->shape[i];
                  num+=n;
               }
               g=g->next;
            }
            l=l->next;
         }
        
         if (num)
         {
            if (!(data=(double *)malloc(num*4*sizeof(double))))
               {
                  printf("send_bbh_xyz:: unable to allocate %i doubles to send to server",4*num);
                  FREE_REG(r);
                  FREE_REG_LIST;
                  return;
               }

               n=0;
               l=ts->levels;
               while(l)
               {
                  g=l->grids;
                  while(g)
                  {
                     if (g->dim<=3 && g->coord_type==COORD_UNIFORM && 
                        (g->selected||(send_children&&(r->selected||ts->selected||l->selected))))
                     {
                        switch(g->dim)
                        {
                           case 1:
                              for (i=0,x1=g->coords[0],dx1=(g->coords[1]-x1)/(g->shape[0]-1);
                                 i<g->shape[0];
                                 i++,x1+=dx1)
                              {
                                 data[n]=x1; 
                                 data[n+num]=g->data[i];
                                 data[n+2*num]=0;
                                 data[n+3*num]=g->data[i];
                                 n++;
                              }
                              break;
                           case 2:
                              for (i=0,x1=g->coords[0],dx1=(g->coords[1]-x1)/(g->shape[0]-1);
                                 i<g->shape[0];
                                 i++,x1+=dx1)
                                 for (j=0,x2=g->coords[2],dx2=(g->coords[3]-x2)/(g->shape[1]-1);
                                    j<g->shape[1];
                                    j++,x2+=dx2)
                                    {
                                       data[n]=x1; 
                                       data[n+num]=x2;
                                       data[n+2*num]=data[n+3*num]=g->data[i+g->shape[0]*j];
                                       n++;
                                    }
                              break;
                           case 3:
                              for (i=0,x1=g->coords[0],dx1=(g->coords[1]-x1)/(g->shape[0]-1);
                                 i<g->shape[0];
                                 i++,x1+=dx1)
                                 for (j=0,x2=g->coords[2],dx2=(g->coords[3]-x2)/(g->shape[1]-1);
                                    j<g->shape[1];
                                    j++,x2+=dx2)
                                    for (k=0,x3=g->coords[4],dx3=(g->coords[5]-x3)/(g->shape[2]-1);
                                       k<g->shape[2];
                                       k++,x3+=dx3)
                                       {
                                          data[n]=x1; 
                                          data[n+num]=x2;
                                          data[n+2*num]=x3;
                                          data[n+3*num]=g->data[i+g->shape[0]*j+g->shape[0]*g->shape[1]*k];
                                          n++;
                                       }
                              break;
                        }
                     }
                     g=g->next;
                  }
                  l=l->next;
               }
               if ((bbh_xyzf_out(r->name,ts->time,data,num)))
                  printf("send_bbh_xyz:: bbh_xyz_out() failed\n");
               else
                  IFL printf("sent grid(s) of %s at t=%f, num=%i\n",r->name,ts->time,num);
               free(data);
            }
            ts=ts->next;
         }
      FREE_REG(r);
      r=r->next;
   }
   FREE_REG_LIST;

   return;
}


//==========================================================================================
// reg.c -- main register functions
//==========================================================================================

#include "reg.h"
#include "misc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "DV.h"
#include "s_iter.h"
#include <math.h>

reg *reg_list;
sem_t reg_list_lock;

//==========================================================================================
// utility functions
//==========================================================================================
int sizeof_data(grid *g)
{
   int n,i;

   for(n=1,i=0;i<g->dim;i++) n*=g->shape[i];

   return n;
}

int sizeof_coords(grid *g)
{
   int nc,i;

   switch(g->coord_type)
   {
         case COORD_UNIFORM: case COORD_SEMI_UNIFORM:
         nc=2*g->dim;
         break; 
      case COORD_PERIMETER: 
         for(nc=0,i=0;i<g->dim;i++) nc+=g->shape[i];
         break;
      case COORD_CURVILINEAR:
         nc=sizeof_data(g)*g->dim;
         break;
      default:
         printf("sizeof_coords:: unknown coordinate type %i\n",g->coord_type);
         nc=0;
   }
   return nc;
}

//==========================================================================================
// gridcmp(g1,g2) = 
//
// 0 if g1=g2
// -1 if g1<g2
// +1 if g1>g2
//
// currently the comparison is made using the following sequence of keys:
// dim
// min[x1]
// ...
// min[xdim]
// shape[x1]
// ...
// shape[xdim]
//==========================================================================================
int gridcmp(grid *g1,grid *g2)
{
   double x1_min[MAX_DIM];
   double x2_min[MAX_DIM];
   int n1,nc1,n2,nc2,i,dim1,dim2,d,ind;

   dim1=g1->dim;
   dim2=g2->dim;
   if (dim1>dim2) return 1;
   if (dim1<dim2) return -1;
   d=dim1;
   
   //--------------------------------------------------------------------------
   // find size n of data and nc of coordinate vectors
   //--------------------------------------------------------------------------
   n1=sizeof_data(g1);
   n2=sizeof_data(g2);
   nc1=sizeof_coords(g1);
   nc2=sizeof_coords(g2);
   
   //--------------------------------------------------------------------------
   // minimum bounds
   //--------------------------------------------------------------------------
   switch(g1->coord_type)
   {
      case COORD_UNIFORM: case COORD_SEMI_UNIFORM:
         for(i=0; i<d; i++) x1_min[i]=g1->coords[2*i];
         break; 
      case COORD_PERIMETER: 
         for(i=0,ind=0; i<d; ind=ind+g1->shape[i],i++) x1_min[i]=g1->coords[ind];
         break;
      case COORD_CURVILINEAR:
         for(i=0,ind=0; i<d; ind=ind+n1,i++) x1_min[i]=g1->coords[ind];
         break;
   }
   switch(g2->coord_type)
   {
      case COORD_UNIFORM: case COORD_SEMI_UNIFORM:
         for(i=0; i<d; i++) x2_min[i]=g2->coords[2*i];
         break; 
      case COORD_PERIMETER: 
         for(i=0,ind=0; i<d; ind=ind+g2->shape[i],i++) x2_min[i]=g2->coords[ind];
         break;
      case COORD_CURVILINEAR:
         for(i=0,ind=0; i<d; ind=ind+n2,i++) x2_min[i]=g2->coords[ind];
         break;
   }

   for(i=0; i<d; i++)
   {
      if (x1_min[i]<x2_min[i]) return -1;
      if (x1_min[i]>x2_min[i]) return 1;
   }

   for(i=0; i<d; i++)
   {
      if (g1->shape[i]<g2->shape[i]) return -1;
      if (g1->shape[i]>g2->shape[i]) return 1;
   }

   return 0;
}

//==========================================================================================
// Utility routines that allocate/deallocate a grid structure and storage for 
// data and coordinates. If the coords,ccoords or data arguments are non-null, then
// they are copied over to the new grid.
// 
// NOTE: nan's in the data sometimes cause the visualization program to crash.
//       for speed, we don't want to filter out the nan's as we send the data
//       to GL, but once before. As a **temporary** measure then, before filters
//       are properly implemented, we weed out the nan's here in galloc.
//==========================================================================================
#define MAX_DATA 1.0e20
grid *galloc(int dim, int coord_type, int *shape, double t, double *coords, double *ccoords, double *data)
{
   int i,n,nc; 
   double *ndata=0,*ncoords=0;
   grid *g=0;

   if (dim>MAX_DIM)
   {
      printf("galloc:: dimension %i too large\n",dim);
      return 0;
   }

   //--------------------------------------------------------------------------
   // find size n of data and nc of coordinate vectors
   //--------------------------------------------------------------------------
   for(n=1,i=0;i<dim;i++) n*=shape[i];
   switch(coord_type)
   {
      case COORD_UNIFORM: case COORD_SEMI_UNIFORM:
         nc=2*dim;
         break; 
      case COORD_PERIMETER: 
         for(nc=0,i=0;i<dim;i++) nc+=shape[i];
         break;
      case COORD_CURVILINEAR:
         nc=n*dim;
         break;
      default:
         printf("galloc:: unknown coordinate type %i\n",coord_type);
         return 0;
   }

   if(   !(g=(grid *)malloc(sizeof(grid))) 
      || !(ndata=(double *)malloc(sizeof(double)*n))
      || !(ncoords=(double *)malloc(sizeof(double)*nc)) )
   {
      printf("galloc:: out of memory, n=%i, nc=%i\n",n,nc);
      if (g) free(g);
      if (ndata) free(ndata);
      if (ncoords) free(ncoords);
      return 0;
   }

   g->dim=dim;
   g->coord_type=coord_type;
   for (i=0;i<dim;i++) g->shape[i]=shape[i];
   g->time=t;
   g->coords=ncoords;
   g->data=ndata;

   if (data) for (i=0; i<n; i++) 
   {
      if (finite(data[i]) && fabs(data[i])<MAX_DATA)
         g->data[i]=data[i];
      else
      {
         printf("galloc: WARNING, x=data[%i]=nan/inf, or |x|>%e ... setting to 0\n",i,MAX_DATA);
         g->data[i]=0;
      }
   }
   else for (i=0; i<n; i++) g->data[i]=0;
   if (coords) for (i=0; i<nc; i++) g->coords[i]=coords[i];

   g->ccoords=0;
   if (g->coord_type==COORD_SEMI_UNIFORM) 
   {
      nc=n*dim;
      if (!(g->ccoords=(double *)malloc(sizeof(double)*nc)))
      {
         printf("galloc: unable to allocate ccoords structure \n");
         gfree(g);
         return 0;
      }
      if (ccoords) for (i=0; i<g->dim*n; i++) g->ccoords[i]=ccoords[i];
   }
   return g;
}

void gfree(grid *g)
{
   free(g->data);
   free(g->coords);
   if (g->ccoords) free(g->ccoords);
   free(g);
   return;
}

//==========================================================================================
// If the register exists, find_reg() locks it, to give the calling program
// exclusive access to the register. It is then the calling program's responsibility to
// correctly 'unlock' the register (FREE_REG, delete it, ...)
//
// if (lock_rl!=0) then LOCK_REG_LIST is issued before the search;
// if (free_rl!=0) then FREE_REG_LIST is issued after the search;
//
// NOTE: to avoid deadlocks, multiple register finds (i.e. locks) must always be issued
// between a *single* enclosing LOCK_REG_LIST/FREE_REG_LIST pair. Use the
// lock_rl, free_rl parameters to accomplish this. 
//==========================================================================================
reg *find_reg(char *name, int lock_rl, int free_rl)
{
   reg *r;

   if (lock_rl!=0) LOCK_REG_LIST;

   r=reg_list;
   while(r && strncmp(name,r->name,MAX_NAME_LENGTH)) r=r->next;
   if (r) LOCK_REG(r);
   
   if (free_rl!=0) FREE_REG_LIST;

   return r;
}

//==========================================================================================
// Add a grid to register 'name', which is created if it doesn't already exist.
// returns 0 if failed.
//==========================================================================================
int add_grid_str(char *name, grid *ng)
{
   reg *r,*nr,*qr,*pr;
   int ltrace=0,i,ret,ind; 
   time_str *ts,*pts,*nts;
   level *l,*nl,*pl;
   grid *g,*pg;
   double dx,*temp,t;
   double fuzz;

   t=ng->time;

   //--------------------------------------------------------------------------
   // find dx for level sorting
   //--------------------------------------------------------------------------
   switch(ng->coord_type)
   {
      case COORD_UNIFORM: case COORD_SEMI_UNIFORM:
         if (ng->shape[0]==1) dx=0;
         else dx=(ng->coords[1]-ng->coords[0])/(ng->shape[0]-1);
         break; 
      case COORD_PERIMETER: 
      case COORD_CURVILINEAR:
         if (ng->shape[0]==1) dx=0;
         else dx=(ng->coords[1]-ng->coords[0]);
         break;
      default:
         printf("add_grid:: unknown coordinate type %i for register %s\n",ng->coord_type,name);
         return 0;
   }

   //--------------------------------------------------------------------------
   // allocate any memory that may be required before we proceed;
   // this is so that we don't have to clean up partially allocated
   // structures if memory allocation were to fail somewhere in the middle
   // of the add process.
   //--------------------------------------------------------------------------
   if(   !(nr=(reg *)malloc(sizeof(reg)))    
      || !(nts=(time_str *)malloc(sizeof(time_str))) 
      || !(nl=(level *)malloc(sizeof(level))) )
   {
      printf("add_reg:: out of memory trying to create register %s\n",name);
      if (nr) free(nr);
      if (nts) free(nts);
      if (nl) free(nl);
      return 0;
   }

   //--------------------------------------------------------------------------
   // use existing register if already in the vault
   //--------------------------------------------------------------------------
   if (r=find_reg(name,1,1))
   {
      free(nr);
   }
   else 
   {
      refresh_GUI_now=1;
      r=nr;
      //-----------------------------------------------------------------------
      // names greater than the allowed length are truncated
      //-----------------------------------------------------------------------
      strncpy(r->name,name,MAX_NAME_LENGTH-1);
      r->name[MAX_NAME_LENGTH-1]=0;
      //-----------------------------------------------------------------------
      // default fuzzies
      //-----------------------------------------------------------------------
      for (i=0; i<ng->dim; i++) r->fuzzy_x[i]=FUZZY;
      r->fuzzy_t=FUZZY;

      r->ts=0;
      r->selected=0;
      r->browser_index=0;
      r->opened=0;
      //-----------------------------------------------------------------------
      // no coordinate names by default
      //-----------------------------------------------------------------------
      for(i=0;i<=MAX_DIM;i++) r->coord_names[i]=0;   
      if ((ret=sem_init(&r->reg_lock,0,(unsigned int)1)))
      {
         printf("add_grid: ERROR --- sem_init(&r->reg_lock,0,(unsigned int 1) failed.\n");
         printf("ret code=%i\n",ret);
         free(nr);
         free(nts);
         free(nl);
         return 0;
      }
      //-----------------------------------------------------------------------
      // register is still in an undefined state, so lock it before
      // adding it to reg_list;
      //-----------------------------------------------------------------------
      LOCK_REG(r);

      LOCK_REG_LIST;
      if (!reg_list)
      {
         reg_list=r;
         r->next=0;
         r->prev=0;
      }
      else
      {
         qr=reg_list; pr=0;
         while (qr && (strcmp(r->name,qr->name)>0)) {pr=qr; qr=qr->next;}
         if (!pr)
            reg_list=r;
         else
            pr->next=r;

         if (qr) qr->prev=r;
         r->prev=pr;
         r->next=qr;
      }
      FREE_REG_LIST;
   }

   //--------------------------------------------------------------------------
   // find/add time
   //--------------------------------------------------------------------------
   nts->levels=0;
   nts->selected=0;
   nts->opened=0;
   nts->browser_index=0;
   nts->time=t;
   fuzz=r->fuzzy_t;
   if (ts=r->ts)
   {
      pts=0;
      while(ts->next && fuzz_gt(t,ts->time,fuzz)) {pts=ts; ts=ts->next;}
      if (fuzz_eq(t,ts->time,fuzz)) // time already exists
         free(nts);
      else if (fuzz_gt(t,ts->time,fuzz)) // new latest time
      {
         if (r->opened) refresh_GUI_now=1;
         ts->next=nts;
         nts->next=0;
         nts->prev=ts;
         ts=nts;
      }
      else 
      {
         if (r->opened) refresh_GUI_now=1;
         if (pts) pts->next=nts; else r->ts=nts;
         nts->next=ts;
         if (ts) ts->prev=nts;
         nts->prev=pts;
         ts=nts;
      }
   }
   else // register was empty
   {
      ts=nts;
      r->ts=ts;
      ts->next=0;
      ts->prev=0;
   }

   //--------------------------------------------------------------------------
   // find/add level
   //--------------------------------------------------------------------------
   nl->dx=dx;
   nl->grids=0;
   nl->selected=0;
   nl->opened=0;
   nl->browser_index=0;
   fuzz=r->fuzzy_x[0];
   if (l=ts->levels)
   {
      pl=0;
      while(l->next && fuzz_lt(dx,l->dx,fuzz)) {pl=l; l=l->next;}
      if (fuzz_eq(dx,l->dx,fuzz)) // level already exists
      {
         free(nl);
      }
      else if (fuzz_lt(dx,l->dx,fuzz)) // new finest level 
      {
         if (ts->opened) refresh_GUI_now=1;
         l->next=nl;
         nl->prev=l;
         l=nl;
         l->next=0;
      }
      else 
      {
         if (ts->opened) refresh_GUI_now=1;
         if (pl) pl->next=nl; else ts->levels=nl;
         nl->next=l;
         if (l) l->prev=nl;
         nl->prev=pl;
         l=nl;
      }
   }
   else // level was empty
   {
      l=nl;
      ts->levels=l;
      l->next=0;
      l->prev=0;
   }
  
   if (l->opened) refresh_GUI_now=1;

   //--------------------------------------------------------------------------
   // initialize part of grid data structure
   //--------------------------------------------------------------------------
   ng->selected=0;
   ng->browser_index=0;

   //--------------------------------------------------------------------------
   // insert grid, sorted via gridcmp, currently without concern for overlap 
   //--------------------------------------------------------------------------
   if (g=l->grids)
   {
      while(g && gridcmp(ng,g)>0) {pg=g; g=g->next;}
      if (g && gridcmp(ng,g)==0)
      {
         if (REPLACE_IDENTICAL_GRIDS)
         {
            printf("add_grid: WARNING: gridcmp(ng,g)=0 ... replacing existing data and coords\n");
            temp=g->coords; g->coords=ng->coords; ng->coords=temp;
            temp=g->data; g->data=ng->data; ng->data=temp;
            gfree(ng);
            ng=g;
         } 
         else
         {
            printf("add_grid: grid not added as gridcmp(ng,g)=0 [would not be able to guarentee a unique register structure]\n");
            FREE_REG(r);
            return 0;
         }
      }
      else if (!g)
      {
         pg->next=ng;
         ng->prev=pg;
         ng->next=0;
      }
      else
      {
         ng->next=g;
         if(!(ng->prev=g->prev))
            l->grids=ng;
         else
            ng->prev->next=ng;
         g->prev=ng;
      }
   }
   else
   {
     ng->next=ng->prev=0;
     l->grids=ng;
   }

   IFL
   {
      printf("add_grid:: added grid to register %s\n",name);
      printf("t=%f, dim=%i, dx=%f, coord_type=%i\n",t,ng->dim,dx,ng->coord_type);
      for (i=0; i<ng->dim; i++) printf("n[%i]=%i ",i,ng->shape[i]);
      printf("\n");
   }

   FREE_REG(r);

   return 1;
}

//==========================================================================================
// Add a grid to register 'name', via a call to add_grid_str(), after the data
// and coords are copied over to new storage.
// returns 0 if failed.
//==========================================================================================
int add_grid(char *name, double t, int dim, int coord_type, 
             int *shape, double *coords, double *ccoords, double *data)
{
   grid *g;

   if (!(g=galloc(dim,coord_type,shape,t,coords,ccoords,data))) return 0;

   if (!add_grid_str(name,g))
   {
      gfree(g);
      return 0;
   }

   return 1;
}

//==========================================================================================
// this routine frees all resources (including the register) associated with r.
// only call this routine after r has been removed from the register list.
//==========================================================================================
void delete_reg_contents(reg *r)
{
   time_str *ts,*pts;
   level *l,*pl;
   grid *g,*pg;
   int i;

   sem_destroy(&(r->reg_lock));

   ts=r->ts;
   
   while(ts)
   {
      l=ts->levels;
      while(l)
      {
         g=l->grids;
         while(g)
         {
            free(g->coords);
            free(g->data);
            pg=g;
            g=g->next;
            free(pg);
         }
         pl=l;
         l=l->next;
         free(pl);
      }
      pts=ts;
      ts=ts->next;
      free(pts);
   }

   for (i=0; i<=MAX_DIM; i++) if (r->coord_names[i]) free(r->coord_names[i]);

   free(r);
   return;
}

//==========================================================================================
// deletes register 'name'; returns 0 if 'name' didn't exist
//==========================================================================================
int delete_reg(char *name)
{
   reg *r,*rl;

   if (!(r=find_reg(name,1,0)))  // maintain lock on register list
   {
      FREE_REG_LIST;
      return 0;
   }

   if (r==reg_list) reg_list=r->next;
   else
   {
      rl=reg_list; 
      while(rl->next!=r) rl=rl->next;
      rl->next=r->next;
   }

   FREE_REG_LIST;

   delete_reg_contents(r);
   return 1;
}

//==========================================================================================
// the following deletes selected grids/levels/registers.
// note:the only allowed leaves in this tree are grids, so any resulting
// bare branches are also deleted.
//==========================================================================================
void delete_grids(int mode)
{
   reg *r,*pr,*nr;
   time_str *ts,*nts,*pts;
   level *l,*pl,*nl;
   grid *g,*ng;
   int i;

   switch(mode)
   {
           case DM_ALL: case DM_SELECTED: case DM_SELECTED_AND_OPENED: 
           case DM_LAST_TIME_OF_SELECTED: break; 

      default:
         printf("delete_grids:: ERROR ... invalid mode %i\n",mode);
         return;
   }

   LOCK_REG_LIST;

   r=reg_list; pr=0;

   while(r)
   {
      LOCK_REG(r);
      ts=r->ts;
      pts=0;
      while(ts)
      {
         l=ts->levels;
         pl=0;
         while(l)
         {
            g=l->grids;
            while(g)
            {
               ng=g->next;
               if ( mode==DM_ALL || 
                   (mode==DM_SELECTED && (g->selected || l->selected || ts->selected || r->selected)) ||
                   (mode==DM_SELECTED_AND_OPENED && ((g->selected && l->opened) || (l->selected && ts->opened) ||
                                                     (ts->selected && r->opened) || r->selected)) ||
                   (mode==DM_LAST_TIME_OF_SELECTED && (r->selected && ts->next==0)) )
               {
                  if (g->prev) g->prev->next=g->next; else l->grids=g->next;
                  if (g->next) g->next->prev=g->prev;
                  free(g->coords);
                  free(g->data);
                  free(g);
               }
               g=ng;
            }
            nl=l->next;
            if (!(l->grids))
            {
               if (pl) pl->next=l->next; else ts->levels=l->next;
               if (l->next) l->next->prev=pl;
               free(l);
            }
            else
               pl=l;
            l=nl;
         }
         nts=ts->next;
         if (!(ts->levels))
         {
            if (pts) pts->next=ts->next; else r->ts=ts->next;
            if (ts->next) ts->next->prev=pts;
            free(ts);
         }
         else
            pts=ts;
         ts=nts;
      }
      nr=r->next;
      if (!(r->ts))
      {
         if (pr) pr->next=r->next; else reg_list=r->next;
         if (r->next) r->next->prev=pr;
         for(i=0; i<=MAX_DIM; i++) if (r->coord_names[i]) free(r->coord_names[i]);
         sem_destroy(&(r->reg_lock)); 
         free(r);
      }
      else
      {
         FREE_REG(r);
         pr=r;
      }
      r=nr;
   }

   FREE_REG_LIST;
}


//==========================================================================================
// initializes dv structure. returns 1 if successful, 0 if failure
// This routine should *only* be called at the beginning of program execution.
//==========================================================================================
int initialize_dv(void)
{
   int ret;

   reg_list=0;

   if ((ret=sem_init(&reg_list_lock,0,(unsigned int)1)))
   {
      printf("initialize_dv: sem_init(&reg_list_write_lock,0,1) failed\n");
      printf("ret code=%i\n",ret);
      return 0;
   }

   return 1;
}

//==========================================================================================
// Shuts down the dv 
//==========================================================================================
void shut_down_dv(void)
{
   reg *pr,*r;

   LOCK_REG_LIST;

   while(r=reg_list)
   {
      LOCK_REG(r);
      reg_list=reg_list->next;
      delete_reg_contents(r);
   }

   reg_list=0;
   sem_destroy(&reg_list_lock);
}

//==========================================================================================
// returns the coordinate number (0 for t, i for xi) of coordinate 'coord' for register r. 
// r IS ASSUMED TO BE LOCKED.
// returns -1 if not found
//==========================================================================================
int get_coord_index(reg *r, char *cx)
{
   char cxdef1[10],cxdef2[10];
   int x,done;

   done=0;
   x=0;
   while(!done)
   {
      if (x>MAX_DIM)
      {
         printf ("get_coord_index:: coordinate %s not found\n",cx);
         return -1;
      }
      sprintf(cxdef1,"%i",x);
      sprintf(cxdef2,"x%i",x);
      if ( (r->coord_names[x] && !(strcmp(cx,r->coord_names[x]))) ||
           !(strcmp(cx,cxdef1)) || !(strcmp(cx,cxdef2)) ||
           (x==0 && !(strcmp(cx,"t"))) ) done=1;
      else
         x++;
   }

   return x;
}

//==========================================================================================
// sets a given coordinate name in the register structure
//==========================================================================================
int set_coord_name(char *reg_name, char *coord_name, int coord)
{
   reg *r;

   if (!(r=find_reg(reg_name,1,1)))
   {
      printf ("set_coord_name:: cannot find register %s\n",reg_name);
      return 0;
   }

   if (r->coord_names[coord]) free(r->coord_names[coord]);

   if (!(r->coord_names[coord]=(char *)malloc(sizeof(char)*(strlen(coord_name)+1))))
   {
       printf ("set_coord_name:: out of memory\n");
       FREE_REG(r);
       return 0;
   }

   strcpy(r->coord_names[coord],coord_name);
   FREE_REG(r);
   return 1;
}

//==========================================================================================
// returns 1 if the grid structure of r1 and r2 is equal for each time 
// (as defined by gridcmp, 's_iter')
// -- times currently not compared
// returns 2 if the grid structure of r1 and r2 is equal and constant for all times,
// else returns 0.
// r1 AND r2 ARE ASSUMED TO BE LOCKED.
//==========================================================================================
int grid_str_eq(reg *r1, reg *r2)
{
   grid *g1,*g2,*g1_tp1;
   int ret=2;
   s_iter it1,it2,it1_tp1;

   g1=init_s_iter(&it1,r1,0);
   g2=init_s_iter(&it2,r2,0);
   init_s_iter(&it1_tp1,r1,0);
   if (next_ts(&it1_tp1)) g1_tp1=it1_tp1.g; else g1_tp1=0;

   while(g1)
   {
      if (!g2 || gridcmp(g1,g2)) return 0;
      //-----------------------------------------------------------------------
      // comparing times might be too strict
      //-----------------------------------------------------------------------
      // if (!(fuzz_eq(it1.ts->time,it2.ts->time,r1->fuzzy_t))) return 0;
      if (g1_tp1 && gridcmp(g1,g1_tp1)) ret=1;
      g1=next_g(&it1);
      g2=next_g(&it2);
      g1_tp1=next_g(&it1_tp1);
   }

   return ret;
}

//==========================================================================================
// if (new_reg) doesn't already exist, clone_reg() creates a grid-wise identical copy
// (to within the resolving power of gridcmp) of old_reg --> new_reg.
//
// returns 0 upon failure, else 1;
//==========================================================================================
int clone_reg(char *old_reg, char *new_reg)
{
   reg *o_r,*n_r;
   time_str *ts;
   level *l;
   grid *g;
   int i;
   s_iter it;

   if (n_r=find_reg(new_reg,1,1))
   {
      printf ("clone_reg:: register %s already exists\n",new_reg);
      FREE_REG(n_r);
      return 0;
   }

   if (!(o_r=find_reg(old_reg,1,1)))
   {
      printf ("clone_reg:: register %s not found\n",old_reg);
      return 0;
   }

   g=init_s_iter(&it,o_r,0);
   while(g)
   {
      if (!add_grid(new_reg,it.ts->time,g->dim,g->coord_type,g->shape,g->coords,g->ccoords,g->data))
      {
         printf ("clone_reg:: add_grid failed. unable to clone register %s\n",old_reg);
         FREE_REG(o_r);
         delete_reg(new_reg);
         return 0;
      }
      g=next_g(&it);
   }

   for(i=0; i<MAX_DIM+1; i++)
      if (o_r->coord_names[i]) set_coord_name(new_reg,o_r->coord_names[i],i);

   FREE_REG(o_r);
   return 1;
}

//==========================================================================================
// renames a register
//==========================================================================================
int rename_reg(char *old_reg, char *new_reg)
{
   reg *o_r=0,*r,*pr;
   int cmp;
   char nr[MAX_NAME_LENGTH];

   LOCK_REG_LIST;

   strncpy(nr,new_reg,MAX_NAME_LENGTH-1);
   nr[MAX_NAME_LENGTH-1]=0;

   r=reg_list;
   pr=0;
   while(r)
   {
      if (!strcmp(r->name,nr))
      {
         printf ("rename_reg:: register %s already exists\n",nr);
         FREE_REG_LIST;
         return 0;
      }
      if (!strcmp(r->name,old_reg)) o_r=r;
      if (!o_r) pr=r;
      r=r->next;
   }
   if (!o_r)
   {
      printf ("rename_reg:: register %s not found\n",old_reg);
      FREE_REG_LIST;
      return 0;
   }
   strcpy(o_r->name,nr);
   if (!pr) reg_list=reg_list->next;
   else pr->next=o_r->next;
   
   pr=0;
   r=reg_list;
   while(r && (strcmp(r->name,new_reg)<0)) { pr=r; r=r->next; }
   if (!pr)
   {
      o_r->next=reg_list;
      reg_list=o_r;
   }
   else
   {
      o_r->next=pr->next;
      pr->next=o_r;
   }
      
   FREE_REG_LIST;
   return 1;
}

//==========================================================================================
// clips a grid; i.e. returns a new grid that contains that piece of the
// old grid g contain within ibbox (or 0 of none)
//==========================================================================================
grid *gclip(grid *g, int *ibbox)
{
   int shape[MAX_DIM];
   int is[MAX_DIM],ie[MAX_DIM],i,j,k,n,ni,nim;
   double coords[3*MAX_DIM];
   double dx,*p,*q,*qp;
   grid *ng;

   if (g->coord_type!=COORD_UNIFORM && g->coord_type!=COORD_SEMI_UNIFORM)
   {
      printf("gclip: only uniform, semi/uniform grids supported at this time\n");
      return 0;
   }

   for (i=0; i<MAX_DIM; i++) is[i]=ie[i]=1;
   for (i=0; i<g->dim; i++)
   {
      if (ibbox[2*i]>g->shape[i] || ibbox[2*i+1]<1) return 0;
      is[i]=max(1,ibbox[2*i]);
      ie[i]=min(g->shape[i],ibbox[2*i+1]);
      shape[i]=ie[i]-is[i]+1;
      if (shape[i]<1)
      {
         printf("gclip: shape < 1 ... ibbox out of order\n");
         return 0;
      }
      if (g->shape[i]>1)
         dx=(g->coords[2*i+1]-g->coords[2*i])/(g->shape[i]-1);
      else dx=1;
      coords[2*i]=g->coords[2*i]+(is[i]-1)*dx;
      coords[2*i+1]=g->coords[2*i]+(ie[i]-1)*dx;
   }

   if (!(ng=galloc(g->dim,g->coord_type,shape,g->time,coords,0,0)))
   {
      printf("gclip: galloc failed.\n");
      return 0;
   }

   if (g->ccoords) nim=1+g->dim; else nim=1;
   for (ni=1; ni<=nim; ni++)
   {
      if (ni==1) { p=ng->data; qp=g->data; }
      else { p=ng->ccoords+sizeof_data(ng)*(ni-2); qp=g->ccoords+sizeof_data(g)*(ni-2); }

      // eventually generalize to arbitrary dimensions:
      switch(g->dim)
      {
         case 1: 
            n=shape[0]; q=&qp[is[0]-1]; while(n--) *p++=*q++; 
            break;
         case 2: 
            j=1;
            while(j<=shape[1])
            {
               q=&qp[is[0]-1+(is[1]-1+j-1)*g->shape[0]]; 
               n=shape[0]; while(n--) *p++=*q++;
               j++;
            }
            break;
         case 3: 
            k=1;
            while(k<=shape[2])
            {
               j=1;
               while(j<=shape[1])
               {
                  q=&qp[is[0]-1+(is[1]-1+j-1)*g->shape[0]+(is[2]-1+k-1)*g->shape[0]*g->shape[1]]; 
                  n=shape[0]; while(n--) *p++=*q++;
                  j++;
               }
               k++;
            }
            break;
         default: 
            printf("gclip: dim>3 not yet supported\n"); 
            gfree(ng); return 0;
      }
   }

   return ng;
}


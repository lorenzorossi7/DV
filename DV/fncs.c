//=============================================================================
// fncs.c 
// 
// implementation of various register functions
//
//=============================================================================

#include "reg.h"
#include "fncs.h"
#include "misc.h"
#include "s_iter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef DARWIN
#include <malloc.h>
#endif 
#include <math.h>

#ifdef __
#define _upd_f10__        upd_f10__        
#define _upd_f20__        upd_f20__        
#define _upd_f21__        upd_f21__        
#define _upd_f30__        upd_f30__        
#define _upd_f31__        upd_f31__        
#define _upd_f32__        upd_f32__        
#define _upd_f11__        upd_f11__        
#define _upd_f22__        upd_f22__        
#define _upd_f23__        upd_f23__        
#define _upd_f33__        upd_f33__        
#define _upd_f34__        upd_f34__        
#define _upd_f35__        upd_f35__        
#define _dc_cls_2d__         dc_cls_2d__         
#define _apply_dis_ex_bdy__  apply_dis_ex_bdy__  
#define _int_f_ds__          int_f_ds__          
#define _fill_eps__          fill_eps__          
#define _diss_ko__           diss_ko__           
#define _diss_ko_axis__      diss_ko_axis__           
#define _diss_ko2__          diss_ko2__          
#define _uni_avg__           uni_avg__           
#define _separate_rb__       separate_rb__           
#define _distance_2bh__      distance_2bh__      
#define _d_overrho__         d_overrho__
#define _d_overrho_d__       d_overrho_d__
#define _d_byrhosq__         d_byrhosq__
#else
#define _upd_f10__        upd_f10_        
#define _upd_f20__        upd_f20_        
#define _upd_f21__        upd_f21_        
#define _upd_f30__        upd_f30_        
#define _upd_f31__        upd_f31_        
#define _upd_f32__        upd_f32_        
#define _upd_f11__        upd_f11_        
#define _upd_f22__        upd_f22_        
#define _upd_f23__        upd_f23_        
#define _upd_f33__        upd_f33_        
#define _upd_f34__        upd_f34_        
#define _upd_f35__        upd_f35_        
#define _dc_cls_2d__         dc_cls_2d_       
#define _apply_dis_ex_bdy__  apply_dis_ex_bdy_
#define _int_f_ds__          int_f_ds_
#define _fill_eps__          fill_eps_
#define _diss_ko__           diss_ko_           
#define _diss_ko_axis__      diss_ko_axis_           
#define _diss_ko2__          diss_ko2_
#define _uni_avg__           uni_avg_           
#define _separate_rb__       separate_rb_           
#define _distance_2bh__      distance_2bh_
#define _d_overrho__         d_overrho_
#define _d_overrho_d__       d_overrho_d_
#define _d_byrhosq__         d_byrhosq_
#endif

//=============================================================================
// Apply unary grid function 'gf(args)' to register 'f'; 
// new register 'nf' created
// 
// optional mask register 'mask' with mask_val (mask can be the same register
// as f, but nf must not exist)
//
// optional 'givec' selection control 
//=============================================================================
int apply_unary_gf(UNARY_FNC gf, char *args, char *f, char *nf, 
                   char *mask, double mask_val, char *givec)
{
   reg *rf,*rnf,*rm;
   s_iter it, it_m;
   grid *g,*ng,*gm,*tg,*cgm;
   int first,i,warn=0,ncall=0;

   // printf("maskval=%f\n",mask_val);
   
   if (mask && !strlen(mask)) mask=0; // null string is also ok for no mask

   if (!(strcmp(f,nf)))
   {
      printf ("apply_unary_gf: ERROR -- new register must be different from %s\n",f);
      return 0;
   }

   if (rnf=find_reg(nf,1,1)) 
   {
      printf ("apply_unary_gf: ERROR -- new register %s already exists\n",nf);
      FREE_REG(rnf);
      return 0;
   }

   // maintain lock on register list if we also want to find a mask 
   if (!(rf=find_reg(f,1,!mask))) 
   {
      printf ("apply_unary_gf: ERROR -- register %s does not exist\n",f);
      if (mask) FREE_REG_LIST;
      return 0;
   }

   rm=0;
   if (mask && !strcmp(mask,f))
   {
      rm=rf;
      FREE_REG_LIST;
   }
   else if (mask && !(rm=find_reg(mask,0,1)))
   {
      printf ("apply_unary_gf: ERROR -- mask register %s does not exist\n",mask);
      FREE_REG(rf);
      return 0;
   }
   if (mask && !(grid_str_eq(rf,rm)))
   {
      printf ("apply_unary_gf: ERROR -- mask %s and register %s do not have the same grid structure\n",mask,f);
      FREE_REG(rf);
      FREE_REG(rm);
      return 0;
   }
         
   g=init_s_iter(&it,rf,givec);
   if (rm) gm=init_s_iter(&it_m,rm,0); else gm=0;

   first=1;
   while(g)
   {
      if (it.selected!=GIV_OFF) 
      {
         cgm=gm;
         if (it.selected==GIV_CLIP)
         {
            // we temporarily replace it.g with the clipped grid. This in
            // general will put the register in an 'illegal' state, as the
            // clipping could affect the grid ordering; but grid functions
            // are only passed the iterator for convenience and information
            // on the time, register name, etc; they should not assume
            // or care about the register structure.
            // (A->unigrid/uniform violates this a bit ... may want
            //  to extend gf_... definition to incorporate functions
            //  like that more readily)
            tg=it.g;
            set_ibbox(&it);
            if (!(it.g=gclip(it.g,it.ibbox)))
            {
               printf("apply_unary_gf: warning --- gclip returned zero;"
                      " inconsistancy between s_iter and gclip on ibbox interpretation?\n");
            }
            else
            {
               it.g->next=tg->next;
               it.g->prev=tg->prev;
               if (it.g->next) it.g->next->prev=it.g;
               if (it.g->prev) it.g->prev->next=it.g;
            }

            if (gm) cgm=gclip(gm,it.ibbox);
         }
         if (it.g) { ng=gf(&it,cgm,mask_val,nf,args,first); ncall++; } else ng=0;
         if (it.selected==GIV_CLIP)
         {
            if (it.g) gfree(it.g);
            it.g=tg;
            if (it.g->next) it.g->next->prev=it.g;
            if (it.g->prev) it.g->prev->next=it.g;
            if (cgm) gfree(cgm);
         }
         if (!ng) warn=1;
         else
         {
            if (!(add_grid_str(nf,ng)))
            {
               printf ("apply_unary_gf: ERROR -- add_grid_str() failed [warn=%i]\n",warn);
               FREE_REG(rf);
               if (rm && rm!=rf) FREE_REG(rm);
               return 0;
            }
         }
         first=0;
      }
      g=next_g(&it);
      if (gm) gm=next_g(&it_m);
   }

   for(i=0; i<(MAX_DIM+1); i++) if (rf->coord_names[i]) set_coord_name(nf,rf->coord_names[i],i);

   FREE_REG(rf);
   if (rm && rm!=rf) FREE_REG(rm);

   if (warn && ncall) printf ("apply_unary_gf: warning -- grid function returned null at least once.\n");
   else if (!ncall) printf ("apply_unary_gf: warning -- no grid functions selected\n"
                            "(either register is empty, or no grids matched the filter selection)\n");
   
   return 1;
}
         
//=============================================================================
// Apply binary grid function 'gf(args)' to registers 'f1' and 'f2'; 
// new register 'nf' created
// 
// optional mask register 'mask' with mask_val (mask can be the same as f1
// or f2, which can also be the same, but nf must not exist)
//
// optional selection control via 'givec'
//=============================================================================
int apply_binary_gf(BINARY_FNC gf, char *args, char *f1, char *f2, char *nf, 
                    char *mask, double mask_val, char *givec)
{
   reg *rf1=0,*rf2=0,*rnf=0,*rm=0;
   s_iter it1,it2,it_m;
   grid *g1,*g2,*ng,*gm,*cgm,*tg1,*tg2;
   int first,i;
   int ret=0,rl_freed=0,warn=0,ncall=0;

   if (mask && !strlen(mask)) mask=0; // null string is also ok for no mask

   if (!(strcmp(f1,nf) || !(strcmp(f2,nf))))
   {
      printf ("apply_binary_gf: ERROR -- new register must be different from %s or %s\n",f1,f2);
      return 0;
   }

   if (rnf=find_reg(nf,1,1))
   {
      printf ("apply_binary_gf: ERROR -- new register %s already exists\n",nf);
      FREE_REG(rnf);
      return 0;
   }

   if (!(rf1=find_reg(f1,1,0)))
   {
      printf ("apply_binary_gf: ERROR -- register %s does not exist\n",f1);
      FREE_REG_LIST;
      return 0;
   }
   if (strcmp(f1,f2))
   {
      if (!(rf2=find_reg(f2,0,0)))
      {
         printf ("apply_binary_gf: ERROR -- register %s does not exist\n",f2);
         goto cleanup;
      }
      if (!grid_str_eq(rf1,rf2))
      {
         printf ("apply_binary_gf: ERROR -- registers %s,%s do not have the same structure\n",f1,f2);
         goto cleanup;
      }
   }
   else rf2=rf1;

   rm=0;
   if (mask && !strcmp(mask,f1))
   {
      rm=rf1;
   }
   else if (mask && !strcmp(mask,f2))
   {
      rm=rf2;
   }
   else if (mask && !(rm=find_reg(mask,0,0)))
   {
      printf ("apply_binary_gf: ERROR -- mask register %s does not exist\n",mask);
      goto cleanup;
   }
   if (mask && !(grid_str_eq(rf1,rm)) && !(grid_str_eq(rf2,rm)))
   {
      printf ("apply_binary_gf: ERROR -- mask %s and registers %s,%s do not have the same grid structure\n",mask,f1,f2);
      goto cleanup;
   }

   FREE_REG_LIST; rl_freed=1;
         
   g1=init_s_iter(&it1,rf1,givec); // only invoke selection with 1 iterator;
                                   // afterall, grids must have identical structure
   g2=init_s_iter(&it2,rf2,0);
   if (rm) gm=init_s_iter(&it_m,rm,0); else gm=0;

   first=1;
   while(g1)
   {
      if (it1.selected!=GIV_OFF)
      {
         cgm=gm;
         if (it1.selected==GIV_CLIP)
         {
            // see note here in apply_unary_gf()
            tg1=it1.g;
            tg2=it2.g;
            set_ibbox(&it1);
            if (!(it1.g=gclip(it1.g,it1.ibbox)))
            {
               printf("apply_unary_gf: warning --- gclip returned zero;"
                      " inconsistancy between s_iter and gclip on ibbox interpretation?\n");
            }
            it2.g=gclip(it2.g,it1.ibbox);  // only it1's iterator set up
            if (it1.g && it2.g)
            {
               it1.g->next=tg1->next;
               it1.g->prev=tg1->prev;
               if (it1.g->next) it1.g->next->prev=it1.g;
               if (it1.g->prev) it1.g->prev->next=it1.g;
               it2.g->next=tg2->next;
               it2.g->prev=tg2->prev;
               if (it2.g->next) it2.g->next->prev=it2.g;
               if (it2.g->prev) it2.g->prev->next=it2.g;
            }
            if (gm) cgm=gclip(gm,it1.ibbox);
         }
         if (it1.g && it2.g) { ng=gf(&it1,&it2,cgm,mask_val,nf,args,first); ncall++; } else ng=0;
         if (it1.selected==GIV_CLIP)
         {
            if (it1.g) gfree(it1.g);
            it1.g=tg1;
            if (it2.g) gfree(it2.g);
            it2.g=tg2;
            if (it1.g->next) it1.g->next->prev=it1.g;
            if (it1.g->prev) it1.g->prev->next=it1.g;
            if (it2.g->next) it2.g->next->prev=it2.g;
            if (it2.g->prev) it2.g->prev->next=it2.g;
            if (cgm) gfree(cgm);
         }
         if (!ng) warn=1;
         else
         {
            if (!(add_grid_str(nf,ng)))
            {
               printf ("apply_binary_gf: ERROR -- add_grid_str() failed [warn=%i]\n",warn);
               goto cleanup;
            }
         }
         first=0;
      }
      g1=next_g(&it1);
      g2=next_g(&it2);
      if (gm) gm=next_g(&it_m);
   }

   for(i=0; i<(MAX_DIM+1); i++) if (rf1->coord_names[i]) set_coord_name(nf,rf1->coord_names[i],i);

   ret=1;
cleanup:
   if (!rl_freed) FREE_REG_LIST;
   if (rf1) FREE_REG(rf1);
   if (rf2 && rf2!=rf1) FREE_REG(rf2);
   if (rm && rm!=rf1 && rm!=rf2) FREE_REG(rm);

   if (warn && ncall) printf ("binary_unary_gf: warning -- grid function returned null at least once.\n");
   else if (!ncall) printf ("apply_unary_gf: warning -- no grid functions selected\n"
                            "(either register is empty, or no grids matched the filter selection)\n");
   return ret;
}

//=============================================================================
// merge is handled seperately ... no mask is applied, and during proper
// use the two registers will have different structures
//
// optional selection control via 'givec' (applied individually to both
// registers)
//=============================================================================
int merge_gf(char *f1, char *f2, char *nf, char *givec)
{
   reg *rf1=0,*rf2=0,*rnf=0;
   s_iter it1;
   grid *g1,*ng;
   int i;
   int ret=0,rl_freed=0;

   if (!(strcmp(f1,nf) || !(strcmp(f2,nf))))
   {
      printf ("merge_gf: ERROR -- new register must be different from %s or %s\n",f1,f2);
      return 0;
   }
   if (!(strcmp(f1,f2)))
   {
      printf ("merge_gf: ERROR -- the two registers must be different\n");
      return 0;
   }
   if (rnf=find_reg(nf,1,1))
   {
      printf ("merge_gf: ERROR -- new register %s already exists\n",nf);
      FREE_REG(rnf);
      return 0;
   }
   if (!(rf1=find_reg(f1,1,0)))
   {
      printf ("merge_gf: ERROR -- register %s does not exist\n",f1);
      FREE_REG_LIST;
      return 0;
   }
   if (!(rf2=find_reg(f2,0,0)))
   {
      printf ("merge_gf: ERROR -- register %s does not exist\n",f2);
      goto cleanup;
   }

   FREE_REG_LIST; rl_freed=1;

   if (givec) printf("merge_gf: WARNING --- same givec used for both registers\ncould have unexpected results using level/time filters\n");
        
   for (i=1; i<=2; i++)
   {
      if (i==1) g1=init_s_iter(&it1,rf1,givec);
      else g1=init_s_iter(&it1,rf2,givec);
      while(g1)
      {
         if (it1.selected!=GIV_OFF)
         {
            ng=0;
            if (it1.selected==GIV_CLIP)
            {
               set_ibbox(&it1);
               if (!(ng=gclip(it1.g,it1.ibbox)))
               {
                  printf("merge_gf: warning --- gclip returned zero;"
                         " inconsistancy between s_iter and gclip on ibbox interpretation?\n");
               }
            }
            else ng=galloc(g1->dim,g1->coord_type,g1->shape,g1->time,g1->coords,g1->ccoords,g1->data);
            if (ng)
            {
               if (!(add_grid_str(nf,ng)))
               {
                  printf ("merge_gf: ERROR -- add_grid_str() failed \n");
                  goto cleanup;
               }
            }
         }
         g1=next_g(&it1);
      }
   }

   for(i=0; i<MAX_DIM; i++) if (rf1->coord_names[i]) set_coord_name(nf,rf1->coord_names[i],i);

   ret=1;
cleanup:
   if (!rl_freed) FREE_REG_LIST;
   if (rf1) FREE_REG(rf1);
   if (rf2 && rf2!=rf1) FREE_REG(rf2);

   return ret;
}
//=============================================================================
//
// The following grid functions are to be called from apply_unary_gf
// or apply_binary_gf, and so do not need to check that grids/masks have
// the same structure, etc ...
//
//=============================================================================

//=============================================================================
// spatial derivative grid function. argument is spatial coordinate 
//=============================================================================
grid *s_diff(s_iter *it, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   int n,i;
   static int x;
   double h,*cmask,*f;
   grid *g,*ng;

   if (first_call) x=get_coord_index(it->r,args);

   if (x<=0) return 0;

   g=it->g;
   if (g->coord_type!=COORD_UNIFORM && g->coord_type!=COORD_SEMI_UNIFORM)
   {
      printf("diffg: non-uniform grids not supported yet\n");
      return 0;
   }
   if (g->coord_type==COORD_SEMI_UNIFORM)
      printf("diffg: WARNING --- differentiating w.r.t bounding box\n");

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;

   if (!mask)
   {
      for(n=1,i=0;i<g->dim;i++) n*=g->shape[i];
      if (!(cmask=(double *)malloc(sizeof(double)*n)))
      {
         printf("s_diff: ERROR --- out of memory\n");
         gfree(ng);
         return 0;
      }
      for (i=0;i<n;i++) cmask[i]=mask_val-1.0e10;
   }
   else cmask=mask->data;

   switch(g->dim)
   {
      case 1:
        switch(x)
        {
           case 1:
              h=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
              _upd_f10__(g->data,cmask,ng->data,&g->shape[0],&h,&mask_val);
              break;
        }
        break;
      case 2:
        switch(x)
        {
           case 1:
              h=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
              _upd_f20__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&h,&mask_val);
              break;
           case 2:
              h=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
              _upd_f21__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&h,&mask_val);
              break;
        }
        break;
      case 3:
        switch(x)
        {
           case 1:
              h=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
              _upd_f30__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&g->shape[2],&h,&mask_val);
              break;
           case 2:
              h=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
              _upd_f31__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&g->shape[2],&h,&mask_val);
              break;
           case 3:
              h=(g->coords[5]-g->coords[4])/(g->shape[2]-1);
              _upd_f32__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&g->shape[2],&h,&mask_val);
              break;
        }
        break;
      default:
         printf("s_diff: dim=%i grids not supported yet\n",g->dim);
         gfree(ng);
         return 0;
   }

   if (!mask) free(cmask);
   return ng;
}

//=============================================================================
// spatial derivative grid function, but using CN stencil, so resultant
// grid is shrunk by 1. argument is spatial coordinate 
//=============================================================================
grid *s_diff_CN(s_iter *it, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   int n,i;
   static int x;
   double h,*cmask,*f;
   grid *g,*ng,*ng2;
   int ibbox[MAX_DIM*2];

   if (first_call) x=get_coord_index(it->r,args);

   if (x<=0) return 0;

   g=it->g;
   if (g->coord_type!=COORD_UNIFORM)
   {
      printf("diffg: non-uniform grids not supported yet\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;

   if (!mask)
   {
      for(n=1,i=0;i<g->dim;i++) n*=g->shape[i];
      if (!(cmask=(double *)malloc(sizeof(double)*n)))
      {
         printf("s_diff: ERROR --- out of memory\n");
         gfree(ng);
         return 0;
      }
      for (i=0;i<n;i++) cmask[i]=mask_val-1.0e10;
   }
   else cmask=mask->data;

   ibbox[0]=1;
   ibbox[2]=1;
   ibbox[4]=1;
   switch(g->dim)
   {
      case 1:
        switch(x)
        {
           case 1:
              h=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
              _upd_f11__(g->data,cmask,ng->data,&g->shape[0],&h,&mask_val);
              ibbox[1]=g->shape[0]-1;
              break;
        }
        break;
      case 2:
        ibbox[1]=g->shape[0];
        ibbox[3]=g->shape[1];
        switch(x)
        {
           case 1:
              h=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
              _upd_f22__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&h,&mask_val);
              ibbox[1]--;
              break;
           case 2:
              h=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
              _upd_f23__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&h,&mask_val);
              ibbox[3]--;
              break;
        }
        break;
      case 3:
        ibbox[1]=g->shape[0];
        ibbox[3]=g->shape[1];
        ibbox[5]=g->shape[2];
        switch(x)
        {
           case 1:
              h=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
              _upd_f33__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&g->shape[2],&h,&mask_val);
              ibbox[1]--;
              break;
           case 2:
              h=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
              _upd_f34__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&g->shape[2],&h,&mask_val);
              ibbox[3]--;
              break;
           case 3:
              h=(g->coords[5]-g->coords[4])/(g->shape[2]-1);
              _upd_f35__(g->data,cmask,ng->data,&g->shape[0],&g->shape[1],&g->shape[2],&h,&mask_val);
              ibbox[5]--;
              break;
        }
        break;
      default:
         printf("s_diff: dim=%i grids not supported yet\n",g->dim);
         gfree(ng);
         return 0;
   }
   ng2=gclip(ng,ibbox);
   gfree(ng);
   ng2->coords[(x-1)*2]+=h/2;
   ng2->coords[(x-1)*2+1]+=h/2;

   if (!mask) free(cmask);
   return ng2;
}

//=============================================================================
// basic arithmetic
//=============================================================================

grid *gf_add(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *ng,*g=it_a->g;
   double *a,*b,*d,*m;
   int i,n;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;

   n=sizeof_data(ng);
   a=it_a->g->data; b=it_b->g->data; d=ng->data;
   if (mask)
   {
      m=mask->data;
      for (i=0;i<n;i++) if (*(m++)==mask_val) { *(d++)=0; a++; b++; } else *(d++)=*(a++)+*(b++);
   }
   else
      for (i=0;i<n;i++) *(d++)=*(a++)+*(b++);
   return ng;
}

grid *gf_sub(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *ng,*g=it_a->g;
   double *a,*b,*d,*m;
   int i,n;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;

   n=sizeof_data(ng);
   a=it_a->g->data; b=it_b->g->data; d=ng->data;
   if (mask)
   {
      m=mask->data;
      for (i=0;i<n;i++) if (*(m++)==mask_val) { *(d++)=0; a++; b++; } else *(d++)=*(a++)-*(b++);
   }
   else
      for (i=0;i<n;i++) *(d++)=*(a++)-*(b++);
   return ng;
}

grid *gf_mult(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *ng,*g=it_a->g;
   double *a,*b,*d,*m;
   int i,n;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;

   n=sizeof_data(ng);
   a=it_a->g->data; b=it_b->g->data; d=ng->data;
   if (mask)
   {
      m=mask->data;
      for (i=0;i<n;i++) if (*(m++)==mask_val) { *(d++)=0; a++; b++; } else *(d++)=*(a++) * *(b++);
   }
   else
      for (i=0;i<n;i++) *(d++)=*(a++) * *(b++);
   return ng;
}

grid *gf_div(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *ng,*g=it_a->g;
   double *a,*b,*d,*m;
   int i,n;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;

   n=sizeof_data(ng);
   a=it_a->g->data; b=it_b->g->data; d=ng->data;
   if (mask)
   {
      m=mask->data;
      for (i=0;i<n;i++) 
         if (*(m++)==mask_val || *b==0) { *(d++)=0; a++; b++; } else *(d++)=*(a++)/(*(b++));
   }
   else
      for (i=0;i<n;i++) if (*b==0) { *(d++)=0; a++; b++; } else *(d++)=*(a++)/(*(b++));
   return ng;
}

//=============================================================================
// scanf's a single float argument ... if the need arises (i.e. we
// have many functions that use more that one float argument), 
// it would be more efficient to implement a general argument parser.
//=============================================================================
void get_single_farg(double *val, char *args, int first_call, double def, char *fname)
{
   double fval;

   if (!first_call) return;
   if (args && strlen(args)>0)
   {
      sscanf(args,"%lf",&fval);
      *val=fval;
   }
   else *val=def;
   printf("%s: arg=%f\n",fname,*val);
}
   
grid *gf_smult(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double c;
   int n,i;
   double *p,*m;

   get_single_farg(&c,args,first_call,1.0,"gf_smult");

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);

   if (mask)
      for (m=mask->data,p=ng->data,i=0;i<n;i++,m++,p++) if (*m==mask_val) *p=0; else (*p)*=c;
   else 
      for (p=ng->data,i=0;i<n;i++,p++) (*p)*=c;
   
   return ng;
}

grid *gf_smult_x(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double ct,c1,c2,c3;

   if (first_call)
   {
      ct=c1=c2=c3=1;
      if (args && strlen(args)>0) sscanf(args,"%lf,%lf,%lf,%lf",&ct,&c1,&c2,&c3);
      if (ct==0) ct=1;
      if (c1==0) c1=1;
      if (c2==0) c2=1;
      if (c3==0) c3=1;
      printf("gf_smult_x: c=[%f,%f,%f,%f]\n",ct,c1,c2,c3);
   }

   if (g->dim>3) printf("gf_smult_x: dimensions >3 not yet supported\n");
   if (g->coord_type!=COORD_UNIFORM && (c1!=1 || c2!=1 || c3!=1))
   {
      printf("gf_smult_x: non-uniform coordinates not yet supported\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;

   ng->coords[0]*=c1;  
   ng->coords[1]*=c1;  
   if (g->dim>1)
   {
      ng->coords[2]*=c2;  
      ng->coords[3]*=c2;  
      if (g->dim>2)
      {
         ng->coords[4]*=c3;  
         ng->coords[5]*=c3;  
      }
   }

   ng->time*=ct;
   
   return ng;
}

grid *gf_unit_bbox(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;

   if (g->dim>3) printf("gf_unit_bbox: dimensions >3 not yet supported\n");
   if (g->coord_type!=COORD_UNIFORM && g->coord_type!=COORD_SEMI_UNIFORM)
   {
      printf("gf_unit_bbox: non-uniform coordinates not yet supported\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;

   ng->coords[0]=0;  
   ng->coords[1]=1;  
   if (g->dim>1)
   {
      ng->coords[2]=0;  
      ng->coords[3]=1;  
      if (g->dim>2)
      {
         ng->coords[4]=0;  
         ng->coords[5]=1;  
      }
   }

   return ng;
}

grid *gf_sadd(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double c;
   int n,i;
   double *p,*m;

   get_single_farg(&c,args,first_call,0.0,"gf_sadd");

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);

   if (mask)
      for (m=mask->data,p=ng->data,i=0;i<n;i++,m++,p++) if (*m==mask_val) *p=0; else (*p)+=c;
   else 
      for (p=ng->data,i=0;i<n;i++,p++) (*p)+=c;
   
   return ng;
}

grid *gf_sadd_x(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double c1,c2,c3,ct;

   if (first_call)
   {
      ct=c1=c2=c3=0;
      if (args && strlen(args)>0) sscanf(args,"%lf,%lf,%lf,%lf",&ct,&c1,&c2,&c3);
      printf("gf_sadd_x: c=[%f,%f,%f,%f]\n",ct,c1,c2,c3);
   }

   if (g->dim>3) printf("gf_sadd_x: dimensions >3 not yet supported\n");
   if (g->coord_type!=COORD_UNIFORM)
   {
      printf("gf_sadd_x: non-uniform coordinates not yet supported\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;

   ng->coords[0]+=c1;  
   ng->coords[1]+=c1;  
   if (g->dim>1)
   {
      ng->coords[2]+=c2;  
      ng->coords[3]+=c2;  
      if (g->dim>2)
      {
         ng->coords[4]+=c3;  
         ng->coords[5]+=c3;  
      }
   }

   ng->time+=ct;
   
   return ng;
}

grid *gf_pow(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double c;
   int n,i;
   double *p,*m;

   get_single_farg(&c,args,first_call,1.0,"gf_pow");

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);

   if (mask)
      for (m=mask->data,p=ng->data,i=0;i<n;i++,m++,p++) if (*m==mask_val) *p=0; else *p=pow(*p,c);
   else 
      for (p=ng->data,i=0;i<n;i++,p++) *p=pow(*p,c);
   
   return ng;
}

grid *gf_ln(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i;
   double *p,*m;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);

   if (mask)
      for (m=mask->data,p=ng->data,i=0;i<n;i++,m++,p++) if (*m==mask_val || *p<=0.0) *p=0; else *p=log(*p);
   else 
      for (p=ng->data,i=0;i<n;i++,p++) if (*p<=0.0) *p=0; else *p=log(*p);
   
   return ng;
}

grid *gf_log(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i;
   double *p,*m;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);

   if (mask)
      for (m=mask->data,p=ng->data,i=0;i<n;i++,m++,p++) if (*m==mask_val || *p<=0.0) *p=0; else *p=log10(*p);
   else 
      for (p=ng->data,i=0;i<n;i++,p++) if (*p<=0) *p=0; else *p=log10(*p);
   
   return ng;
}

grid *gf_exp(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i;
   double *p,*m;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);

   if (mask)
      for (m=mask->data,p=ng->data,i=0;i<n;i++,m++,p++) if (*m==mask_val) *p=0; else *p=exp(*p);
   else 
      for (p=ng->data,i=0;i<n;i++,p++) *p=exp(*p);
   
   return ng;
}

grid *gf_sin(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i;
   double *p,*m;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);

   if (mask)
      for (m=mask->data,p=ng->data,i=0;i<n;i++,m++,p++) if (*m==mask_val) *p=0; else *p=sin(*p);
   else 
      for (p=ng->data,i=0;i<n;i++,p++) *p=sin(*p);
   
   return ng;
}

grid *gf_cos(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i;
   double *p,*m;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);

   if (mask)
      for (m=mask->data,p=ng->data,i=0;i<n;i++,m++,p++) if (*m==mask_val) *p=0; else *p=cos(*p);
   else 
      for (p=ng->data,i=0;i<n;i++,p++) *p=cos(*p);
   
   return ng;
}

grid *gf_abs(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i;
   double *p,*m;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);

   if (mask)
      for (m=mask->data,p=ng->data,i=0;i<n;i++,m++,p++) if (*m==mask_val) *p=0; else *p=fabs(*p);
   else 
      for (p=ng->data,i=0;i<n;i++,p++) *p=fabs(*p);
   
   return ng;
}

grid *gf_clone(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g;

   return galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data);
}

grid *gf_trim(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double c;
   static int n;
   int new_ibb[2*MAX_DIM];

   get_single_farg(&c,args,first_call,1.0,"gf_trim");

   n=c; if (n<1) n=1;

   if (first_call) printf("gf_trim: trimming grid by %i point(s)\n",n);

   new_ibb[0]=n+1;
   new_ibb[1]=g->shape[0]-n;
   if (new_ibb[1]<=new_ibb[0])
   {
      printf("gf_trim: trim amount too large\n"); return 0;
   }
   if (g->dim>1)
   {
      new_ibb[2]=n+1;
      new_ibb[3]=g->shape[1]-n;
      if (new_ibb[3]<=new_ibb[2])
      {
         printf("gf_trim: trim amount too large\n"); return 0;
      }
   }
   if (g->dim>2)
   {
      new_ibb[3]=n+1;
      new_ibb[4]=g->shape[2]-n;
      if (new_ibb[5]<=new_ibb[4])
      {
         printf("gf_trim: trim amount too large\n"); return 0;
      }
   }

   ng=gclip(g,new_ibb);
   
   return ng;
}

grid *gf_zero_bnd(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double c;
   static int n;
   int i,j,k;
   double *p;
   int new_ibb[2*MAX_DIM];

   get_single_farg(&c,args,first_call,1.0,"gf_zero_bnd");

   n=c; if (n<1) n=1;

   if (first_call) printf("gf_trim: zeroing boundary zone of grid by %i point(s)\n",n);

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;

   p=ng->data;
   for (i=0; i<g->shape[0]; i++)
   {
      if (g->dim==1)
      {
         if (i<c || i>=(g->shape[0]-c)) p[i]=0;
      }
      else
      {
         for (j=0; j<g->shape[1]; j++)
         {
            if (g->dim==2)
            {
               if ((i<c || i>=(g->shape[0]-c)) ||
                   (j<c || j>=(g->shape[1]-c)) ) p[i+j*g->shape[0]]=0;
            }
            else
            {
               for (k=0; k<g->shape[2]; k++)
               {
                  if ((i<c || i>=(g->shape[0]-c)) ||
                      (j<c || j>=(g->shape[1]-c)) ||
                      (k<c || k>=(g->shape[2]-c))) p[i+j*g->shape[0]+k*g->shape[0]*g->shape[1]]=0;
               }
            }
         }
      }
   }

   return ng;
}

//============================================================================= 
// change t to -(ln(t*-t)).
//============================================================================= 
grid *gf_ln_ts_m_t(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double ts,prev_t,dt,prev_t0;

   get_single_farg(&ts,args,first_call,1.0,"gf_ln_tps_m_tp");
   if (first_call) { prev_t=0; dt=1; prev_t0=-1;}

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   if (ts>g->time) ng->time=-log(ts-g->time);
   else ng->time=prev_t+dt;
 
   if (prev_t0!=g->time)
   {
      dt=ng->time-prev_t;
      prev_t=ng->time;
   }
   prev_t0=g->time;
   return ng;
}

//============================================================================= 
// change t to lambda(t) ... the argument is a 1D non-uniform coordinate 
// system specifying the transformation (first grid of register used)
//============================================================================= 
grid *gf_t_to_lt(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng=0,*lg;
   reg *lambda=0;
   s_iter lambda_it;
   double *t,*l,new_t,frac;
   int n;

   if (!(lambda=find_reg(args,1,1))) { printf("gf_t_to_lt: cannot find register <%s>\n",args); return 0; }

   lg=init_s_iter(&lambda_it,lambda,0);
   if (!lg || lg->dim!=1 || (lg->coord_type!=COORD_CURVILINEAR && lg->coord_type!=COORD_SEMI_UNIFORM))
   {
      printf("gf_t_to_lt: register <%s> must have dim=1, and coord_type=COORD_CURVILINEAR or COORD_SEMI_UNIFORM\n",args);
      goto clean_up;
   }

   if (lg->coord_type==COORD_CURVILINEAR) t=lg->coords; else t=lg->ccoords;
   l=lg->data;

   n=0;
   while(n<sizeof_data(g) && g->time>t[n]) n++;
   if (n==0 || (n==(sizeof_data(g)-1) && g->time>t[n]))
   {
      printf("gf_t_to_lt: time %lf of <%s> out of range ... clipping\n",g->time,it_a->r->name);
      goto clean_up;
   }

   frac=(g->time-t[n-1])/(t[n]-t[n-1]);
   new_t=frac*l[n]+(1-frac)*l[n-1];
   
   ng=galloc(g->dim,g->coord_type,g->shape,new_t,g->coords,g->ccoords,g->data);
   
clean_up:
   if (lambda) FREE_REG(lambda);
   return ng;
}

//=============================================================================
// If an argument is supplied, then a coordinate grid corresponding to the
// coordinate specified by the argument is returned; else all spatial
// coordinates are converted into registers with names corresponding to
// the coordinate names (and null is returned).
//
// ALL SPATIAL COORDINATE OPTION DISABLED --- not consistant with 
// grid-function mechanism; i.e. existing registers with coordinate names
// will have grids appended, not over-written.
//=============================================================================
grid *gf_coords(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i,j,k;
   double xi,dx,yi,dy,zi,dz,x,y,z,*cp;
   static int xc;
   int do1,do2,do3;

   if (g->coord_type != COORD_UNIFORM && g->coord_type != COORD_SEMI_UNIFORM)
   {
      printf("gf_coords: non uniform coordinates currently not supported\n");
      return 0;
   }

   if (g->dim > 3)
   {
      printf("gf_coords: spatial dimension > 3 currently not supported\n");
      return 0;
   }

   if (first_call) xc=get_coord_index(it_a->r,args);

   do1=do2=do3=0;
   switch(xc)
   {
      case 1: do1=1; break;
      case 2: do2=1; break;
      case 3: do3=1; break;
      default: 
         if (first_call) printf("gf_coords: invalid coordinate name %s\n",args);
         return 0;
         // DISABLED
         switch(g->dim)
         {
            case 3: do3=1;
            case 2: do2=1;
            case 1: do1=1;
         }
   }

   cp=0;
   if (g->coord_type == COORD_SEMI_UNIFORM) cp=g->ccoords+(xc-1)*sizeof_data(g);

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,cp))) return 0;
   if (cp) return ng;

   n=sizeof_data(ng);

   switch(g->dim)
   {
      case 3: dz=(g->coords[5]-g->coords[4])/(g->shape[2]-1);
              zi=g->coords[4];
      case 2: dy=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
              yi=g->coords[2];
      case 1: dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
              xi=g->coords[0];
   }
   if (do1)
   {
      switch(g->dim)
      {
         case 1:
            for(i=0,x=xi;i<g->shape[0];i++,x+=dx) ng->data[i]=x;
            break;
         case 2:
            for(i=0,x=xi;i<g->shape[0];i++,x+=dx)
               for(j=0;j<g->shape[1];j++) ng->data[i+j*g->shape[0]]=x;
            break;
         case 3:
            for(i=0,x=xi;i<g->shape[0];i++,x+=dx)
               for(j=0;j<g->shape[1];j++)
                  for(k=0;k<g->shape[2];k++)
                     ng->data[i+j*g->shape[0]+k*g->shape[0]*g->shape[1]]=x;
            break;
      }

      if (xc==1) return ng;
      if (it_a->r->coord_names[1]) 
      {
         if (!(add_grid_str(it_a->r->coord_names[1],ng)))
         {
            printf("gf_coord: ERROR adding grid to register %s\n",it_a->r->coord_names[1]);
            gfree(ng); return 0;
         }
      }
      else
      {
         if (!(add_grid_str("x1",ng)))
         {
            printf("gf_coord: ERROR adding grid to register x1\n");
            gfree(ng); return 0;
         }
      }
      ng=0;
   }
   if (do2)
   {
      if (!ng) { if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0; }

      switch(g->dim)
      {
         case 1: 
            printf("gf_coords: ERROR --- dim=1, coord=x2\n"); 
            gfree(ng); return 0;
            break;
         case 2:
            for(i=0;i<g->shape[0];i++)
               for(j=0,y=yi;j<g->shape[1];j++,y+=dy) ng->data[i+j*g->shape[0]]=y;
            break;
         case 3:
            for(i=0;i<g->shape[0];i++)
               for(j=0,y=yi;j<g->shape[1];j++,y+=dy)
                  for(k=0;k<g->shape[2];k++)
                     ng->data[i+j*g->shape[0]+k*g->shape[0]*g->shape[1]]=y;
            break;
      }

      if (xc==2) return ng;
      if (it_a->r->coord_names[2]) 
      {
         if (!(add_grid_str(it_a->r->coord_names[2],ng)))
         {
            printf("gf_coord: ERROR adding grid to register %s\n",it_a->r->coord_names[2]);
            gfree(ng); return 0;
         }
      }
      else
      {
         if (!(add_grid_str("x2",ng)))
         {
            printf("gf_coord: ERROR adding grid to register x2\n");
            gfree(ng); return 0;
         }
      }
      ng=0;
   }
   if (do3)
   {
      if (!ng) { if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0; }

      switch(g->dim)
      {
         case 1: 
            printf("gf_coords: ERROR --- dim=1, coord=x2\n"); 
            gfree(ng); return 0;
            break;
         case 2:
            printf("gf_coords: ERROR --- dim=2, coord=x3\n"); 
            gfree(ng); return 0;
            break;
         case 3:
            for(i=0;i<g->shape[0];i++)
               for(j=0;j<g->shape[1];j++)
                  for(k=0,z=zi;k<g->shape[2];k++,z+=dz)
                     ng->data[i+j*g->shape[0]+k*g->shape[0]*g->shape[1]]=z;
            break;
      }

      if (xc==3) return ng;
      if (it_a->r->coord_names[3]) 
      {
         if (!(add_grid_str(it_a->r->coord_names[3],ng)))
         {
            printf("gf_coord: ERROR adding grid to register %s\n",it_a->r->coord_names[3]);
            gfree(ng); return 0;
         }
      }
      else
      {
         if (!(add_grid_str("x3",ng)))
         {
            printf("gf_coord: ERROR adding grid to register x3\n");
            gfree(ng); return 0;
         }
      }
      ng=0;
   }
   
   return 0;
}

grid *gf_l2norm(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   double *p,*m,pt;
   grid *g=it_a->g,*ng,*nxt_g;
   int n,ret;
   static time_str *ts;
   static double *norm,*t;
   static int ct,numt,sum;

   if (first_call)
   {
      printf("l2 norm of %s:\nt\t\tnorm\n",it_a->r->name);
      save_s_iter(it_a);
      nxt_g=g;
      pt=it_a->ts->time;
      numt=1;
      while(nxt_g) 
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            pt=it_a->ts->time;
         } 
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);
      if (!(norm=(double *)malloc(sizeof(double)*numt)) ||
          !(t=(double *)malloc(sizeof(double)*numt)) )
      {
         printf("gf_l2norm: out of memory\n");
         return 0;
      }
      ts=it_a->ts;
      ct=0;
      norm[ct]=0;
      t[0]=ts->time;
      sum=0;
   }
   if (!norm) return 0;
   if (it_a->ts!=ts)
   {
      if (sum==0) norm[ct]=0; else norm[ct]=sqrt(norm[ct]/sum);
      printf("%f\t%e\n",t[ct],norm[ct]);
      ts=it_a->ts;
      ct++; norm[ct]=0; t[ct]=ts->time;
      sum=0;
   }

   p=g->data;
   n=sizeof_data(g);
   if (mask)
   {
      m=mask->data;
      while(n--)
      {
         if (*m++!=mask_val) {norm[ct]+=(*p)*(*p); sum++;}
         p++;
      }
   }
   else
   {
      while(n--)
      {
         norm[ct]+=(*p)*(*p); sum++;
         p++;
      }
   }

   ng=0;
   ret=1;
   save_s_iter(it_a);
   nxt_g=next_g(it_a);
   while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
   restore_s_iter(it_a);
   if (ret)
   {
      if (sum==0) norm[ct]=0; else norm[ct]=sqrt(norm[ct]/sum);
      if (!(ng=galloc(1,COORD_CURVILINEAR,&numt,0.0,t,0,norm))) return 0;
      printf("%f\t%e\n",t[ct],norm[ct]);
      free(norm);
      free(t);
   }

   return ng;
}

grid *gf_infnorm(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   double *p,*m,pt;
   grid *g=it_a->g,*ng,*nxt_g;
   int n,ret;
   static time_str *ts;
   static double *norm,*t;
   static int ct,numt;

   if (first_call)
   {
      printf("infinity norm of %s:\nt\t\tnorm\n",it_a->r->name);
      save_s_iter(it_a);
      nxt_g=g;
      pt=it_a->ts->time;
      numt=1;
      while(nxt_g) 
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            pt=it_a->ts->time;
         } 
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);
      if (!(norm=(double *)malloc(sizeof(double)*numt)) ||
          !(t=(double *)malloc(sizeof(double)*numt)) )
      {
         printf("gf_infnorm: out of memory\n");
         return 0;
      }
      ts=it_a->ts;
      ct=0;
      norm[ct]=0;
      t[0]=ts->time;
   }
   if (!norm) return 0;
   if (it_a->ts!=ts)
   {
      printf("%f\t%e\n",t[ct],norm[ct]);
      ts=it_a->ts;
      ct++; norm[ct]=0; t[ct]=ts->time;
   }
   p=g->data;
   n=sizeof_data(g);
   if (mask)
   {
      m=mask->data;
      while(n--)
      {
         if (*m++!=mask_val) norm[ct]=max(norm[ct],fabs(*p));
         p++;
      }
   }
   else
   {
      while(n--)
      {
         norm[ct]=max(norm[ct],fabs(*p));
         p++;
      }
   }

   ng=0;
   ret=1;
   save_s_iter(it_a);
   nxt_g=next_g(it_a);
   while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
   restore_s_iter(it_a);
   if (ret)
   {
      if (!(ng=galloc(1,COORD_CURVILINEAR,&numt,0.0,t,0,norm))) return 0;
      printf("%f\t%e\n",t[ct],norm[ct]);
      free(norm);
      free(t);
   }

   return ng;
}

grid *gf_min(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng,*nxt_g;
   int n,ret;
   double *p,*m,pt;
   static time_str *ts;
   static double *ext,*t;
   static int ct,numt;

   if (first_call)
   {
      printf("minumum of %s:\nt\t\tminimum\n",it_a->r->name);
      save_s_iter(it_a);
      nxt_g=g;
      pt=it_a->ts->time;
      numt=1;
      while(nxt_g) 
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            pt=it_a->ts->time;
         } 
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);
      if (!(ext=(double *)malloc(sizeof(double)*numt)) ||
          !(t=(double *)malloc(sizeof(double)*numt)) )
      {
         printf("gf_min: out of memory\n");
         return 0;
      }
      ts=it_a->ts;
      ct=0;
      ext[ct]=1e50;
      t[0]=ts->time;
   }

   if (!ext) return 0;
   if (it_a->ts!=ts)
   {
      printf("%f\t%e\n",t[ct],ext[ct]);
      ts=it_a->ts;
      ct++; ext[ct]=1e50; t[ct]=ts->time;
   }

   p=g->data;
   n=sizeof_data(g);
   if (mask)
   {
      m=mask->data;
      while(n--)
      {
         if (*m++!=mask_val) { if ((*p)<ext[ct]) ext[ct]=*p; }
         p++;
      }
   }
   else
   {
      while(n--)
      {
         if ((*p)<ext[ct]) ext[ct]=*p;
         p++;
      }
   }

   ng=0;
   ret=1;
   save_s_iter(it_a);
   nxt_g=next_g(it_a);
   while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
   restore_s_iter(it_a);
   if (ret)
   {
      if (!(ng=galloc(1,COORD_CURVILINEAR,&numt,0.0,t,0,ext))) return 0;
      printf("%f\t%e\n",t[ct],ext[ct]);
      free(ext);
      free(t);
   }

   return ng;
}

grid *gf_max(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng,*nxt_g;
   int n,ret,i,j,k,nx,ny,nz,ind;
   double *p,*m,pt,xi,yi,zi,dx,dy,dz,x0,y0,z0;
   static time_str *ts;
   static double *ext,*t,x,y,z;
   static int ct,numt;

   if (first_call)
   {
      printf("maximum of %s:\nt\t\tmaximum\tx\ty\tz\n",it_a->r->name);
      save_s_iter(it_a);
      nxt_g=g;
      pt=it_a->ts->time;
      numt=1;
      while(nxt_g) 
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            pt=it_a->ts->time;
         } 
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);
      if (!(ext=(double *)malloc(sizeof(double)*numt)) ||
          !(t=(double *)malloc(sizeof(double)*numt)) )
      {
         printf("gf_max: out of memory\n");
         return 0;
      }
      ts=it_a->ts;
      ct=0;
      ext[ct]=-1e50;
      t[0]=ts->time;
      x=y=z=xi=yi=zi=0;
   }

   if (!ext) return 0;

   if (it_a->ts!=ts)
   {
      printf("%f\t%e\t%e\t%e\t%e\n",t[ct],ext[ct],x,y,z);
      ts=it_a->ts;
      ct++; ext[ct]=-1e50; t[ct]=ts->time;
   }

   p=g->data;

   nx=ny=nz=1; 

   switch(g->dim)
   {
      case 3: dz=(g->coords[5]-g->coords[4])/(g->shape[2]-1);
              zi=g->coords[4]; nz=g->shape[2];
      case 2: dy=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
              yi=g->coords[2]; ny=g->shape[1];
      case 1: dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
              xi=g->coords[0]; nx=g->shape[0];
   }

   n=sizeof_data(g);
   if (mask) m=mask->data;
   for (i=0,x0=xi; i<nx; i++,x0+=dx)
      for (j=0,y0=yi; j<ny; j++,y0+=dy)
         for (k=0,z0=zi; k<nz; k++,z0+=dz)
         {
            ind=i+j*nx+k*nx*ny;
            if (mask)
            {
               if (m[ind]!=mask_val) { if (p[ind]>ext[ct]) { ext[ct]=p[ind]; x=x0; y=y0; z=z0;} }
            }
            else if (p[ind]>ext[ct]) { ext[ct]=p[ind]; x=x0; y=y0; z=z0;}
         }

   ng=0;
   ret=1;
   save_s_iter(it_a);
   nxt_g=next_g(it_a);
   while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
   restore_s_iter(it_a);
   if (ret)
   {
      if (!(ng=galloc(1,COORD_CURVILINEAR,&numt,0.0,t,0,ext))) return 0;
      printf("%f\t%e\t%e\t%e\t%e\n",t[ct],ext[ct],x,y,z);
      free(ext);
      free(t);
   }

   return ng;
}

grid *gf_extremum(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng,*nxt_g;
   int n,ret;
   double *p,*m,pt;
   static time_str *ts;
   static double *ext,*t;
   static int ct,numt;

   if (first_call)
   {
      printf("extremum of %s:\nt\t\textremum\n",it_a->r->name);
      save_s_iter(it_a);
      nxt_g=g;
      pt=it_a->ts->time;
      numt=1;
      while(nxt_g) 
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            pt=it_a->ts->time;
         } 
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);
      if (!(ext=(double *)malloc(sizeof(double)*numt)) ||
          !(t=(double *)malloc(sizeof(double)*numt)) )
      {
         printf("gf_max: out of memory\n");
         return 0;
      }
      ts=it_a->ts;
      ct=0;
      ext[ct]=0;
      t[0]=ts->time;
   }

   if (!ext) return 0;
   if (it_a->ts!=ts)
   {
      printf("%f\t%e\n",t[ct],ext[ct]);
      ts=it_a->ts;
      ct++; ext[ct]=0; t[ct]=ts->time;
   }

   p=g->data;
   n=sizeof_data(g);
   if (mask)
   {
      m=mask->data;
      while(n--)
      {
         if (*m++!=mask_val) { if (fabs(*p)>fabs(ext[ct])) ext[ct]=*p; }
         p++;
      }
   }
   else
   {
      while(n--)
      {
         if (fabs(*p)>fabs(ext[ct])) ext[ct]=*p;
         p++;
      }
   }

   ng=0;
   ret=1;
   save_s_iter(it_a);
   nxt_g=next_g(it_a);
   while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
   restore_s_iter(it_a);
   if (ret)
   {
      if (!(ng=galloc(1,COORD_CURVILINEAR,&numt,0.0,t,0,ext))) return 0;
      printf("%f\t%e\n",t[ct],ext[ct]);
      free(ext);
      free(t);
   }

   return ng;
}

//=============================================================================
// graxi specific --- computes r=sqrt(rho^2+z^2)
//=============================================================================
grid *gf_r(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i,j;
   double xi,dx,yi,dy,x,y;

   if (g->coord_type != COORD_UNIFORM && g->coord_type!=COORD_SEMI_UNIFORM)
   {
      printf("gf_r: non uniform coordinates not currently supported\n");
      return 0;
   }

   if (g->dim != 2)
   {
      printf("gf_r: spatial dimension must be 2\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;
   n=sizeof_data(ng);

   dy=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
   yi=g->coords[2];
   dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   xi=g->coords[0];

   for(i=0,x=xi;i<g->shape[0];i++,x+=dx)
      for(j=0,y=yi;j<g->shape[1];j++,y+=dy) ng->data[i+j*g->shape[0]]=pow(y*y+x*x,0.5);
   
   return ng;
}

//=============================================================================
// graxi specific --- computes (A*r)^p
//=============================================================================
grid *gf_multr_p(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i,j,ind;
   double xi,dx,yi,dy,x,y;
   static double p;

   get_single_farg(&p,args,first_call,1.0,"gf_multr_p");

   if (g->coord_type != COORD_UNIFORM && g->coord_type!=COORD_SEMI_UNIFORM)
   {
      printf("gf_multr_p: non uniform coordinates not currently supported\n");
      return 0;
   }

   if (g->dim != 2)
   {
      printf("gf_multr_p: spatial dimension must be 2\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;
   n=sizeof_data(ng);

   dy=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
   yi=g->coords[2];
   dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   xi=g->coords[0];

   for(i=0,x=xi;i<g->shape[0];i++,x+=dx)
      for(j=0,y=yi;j<g->shape[1];j++,y+=dy)
      {
         ind=i+j*g->shape[0];
         ng->data[ind]=pow(g->data[ind]*pow(y*y+x*x,0.5),p);
      }
   
   return ng;
}

//=============================================================================
// graxi specific --- computes Brill mass density
// -1.0*($D_0(psi,rho)+rho*$D_2(psi,rho)+rho*$D_2(psi,z)); (RNPL notation)
// (only first order accurate at boundaries)
//=============================================================================
grid *gf_mass_dens(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i,j,ind,ip1,jp1,im1,jm1;
   double rhoi,drho,zi,dz,rho,z,psi_rho,psi_rhorho,psi_zz;
   static double p;

   if (g->coord_type != COORD_UNIFORM && g->coord_type!=COORD_SEMI_UNIFORM)
   {
      printf("gf_mass_dens: non uniform coordinates not currently supported\n");
      return 0;
   }

   if (g->dim != 2)
   {
      printf("gf_mass_dens: spatial dimension must be 2\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;
   n=sizeof_data(ng);

   dz=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
   zi=g->coords[2];
   drho=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   rhoi=g->coords[0];

   for(i=1,rho=rhoi+drho;i<(g->shape[0]-1);i++,rho+=drho)
      for(j=1,z=zi+dz;j<(g->shape[1]-1);j++,z+=dz)
      {
         ind=i+j*g->shape[0];
         ip1=ind+1; 
         im1=ind-1; 
         jp1=ind+g->shape[0];
         jm1=ind-g->shape[0]; 
         psi_rhorho=(g->data[ip1]-2*g->data[ind]+g->data[im1])/drho/drho;
         psi_rho=(g->data[ip1]-g->data[im1])/2/drho;
         psi_zz=(g->data[jp1]-2*g->data[ind]+g->data[jm1])/dz/dz;
         ng->data[ind]=-1*(psi_rho+rho*(psi_rhorho+psi_zz));
      }

   for(i=1;i<(g->shape[0]-1);i++)
   {
      ng->data[i]=ng->data[i+g->shape[0]];
      ind=i+(g->shape[1]-1)*g->shape[0];
      ng->data[ind]=ng->data[ind-g->shape[0]];
   }
   for(j=0;j<g->shape[1];j++)
   {
      ind=j*g->shape[0];
      ng->data[ind]=ng->data[ind+1];
      ind=g->shape[0]-1+j*g->shape[0];
      ng->data[ind]=ng->data[ind-1];
   }
   
   return ng;
}

//=============================================================================
// graxi specific --- fix the "self-mask" of A -> for those functions 
// where I was too lazy to set the function=0 at certain places on the 
// boundary when excised: If (A==0) at rho=drho, then A is set to zero at rho=0.
//
// NOTE: A self-mask was implemented to make looking at several hundred MB
//       grids more efficient; i.e. so we don't need to load chr as well.
//       For similar reasons, the operation of gf_fix_sf is currently
//       *illegal* --- it modifies the source grid.
//       Eventually we may want to extend the definition of the data-vault
//       to allow for a set of 'safe' functions that can operate upon a grid 
//       in-place. A safe function would be one whose operation does 
//       not change the unique ordering of the grids within a register.
//=============================================================================
grid *gf_fix_sf(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g;
   int i,j,ind;

   if (g->coord_type != COORD_UNIFORM && g->coord_type!=COORD_SEMI_UNIFORM)
   {
      printf("gf_multr_p: non uniform coordinates not currently supported\n");
      return 0;
   }

   if (g->dim != 2)
   {
      printf("gf_multr_p: spatial dimension must be 2\n");
      return 0;
   }

   for(j=0;j<g->shape[1];j++)
   {
      ind=1+j*g->shape[0];
      if (g->data[ind]==0) g->data[ind-1]=0;
   }
   
   return 0;
}

//=============================================================================
// graxi specific. effectively does fix, t*ct, and (A*r)^p.
// OVERWRITES EXCISTING REGISTER
// fudge function to make manipulating some massive sdf's a little
// quicker with the small amount of RAM on the bh machines.
//=============================================================================
grid *gf_fix_mc_mr(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double ct,p;
   int i,ind,j;
   double dx,dy,yi,xi,y,x;

   if (first_call)
   {
      ct=1;
      p=0;
      if (args && strlen(args)>0) sscanf(args,"%lf,%lf",&ct,&p);
      if (ct==0) ct=1;
      printf("gf_fix_mc_mr: ct=%f, p=%f\n",ct,p);
   }

   if (g->dim>3) printf("gf_fix_mc_mr: dimensions >3 not yet supported\n");
   if (g->coord_type!=COORD_UNIFORM && g->coord_type!=COORD_SEMI_UNIFORM)
   {
      printf("gf_fix_mc_mr: non-uniform coordinates not yet supported\n");
      return 0;
   }

   it_a->ts->time*=ct;

   // do what fix_sf above does
  
   for(j=0;j<g->shape[1];j++)
   {
      ind=1+j*g->shape[0];
      if (g->data[ind]==0) g->data[ind-1]=0;
   }

   //  (A*r)^p
   
   if (p!=0)
   {
      dy=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
      yi=g->coords[2];
      dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
      xi=g->coords[0];

      for(i=0,x=xi;i<g->shape[0];i++,x+=dx)
         for(j=0,y=yi;j<g->shape[1];j++,y+=dy)
         {
            ind=i+j*g->shape[0];
            g->data[ind]=pow(g->data[ind]*pow(y*y+x*x,0.5),p);
         }
   }
   
   return 0;
}
//=============================================================================
// 2D clustering routine
//=============================================================================
#define MAX_CLS 100
#ifdef __
void dc_cls_2d__(int *chr, int *cls, int *no_cls, int *max_cls, int *minwidth,
                 int *buf_wid, double *min_eff, int *blk_fac, int *work, int *Nx,
                 int *Ny);
#else
void dc_cls_2d_(int *chr, int *cls, int *no_cls, int *max_cls, int *minwidth,
                int *buf_wid, double *min_eff, int *blk_fac, int *work, int *Nx,
                int *Ny);
#endif
grid *gf_dc_2dcls(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng,*ng2;
   static int minwidth,buf_wid,blk_fac;
   static double threshold,min_eff;
   int work[6*MAX_CLS],shape[2];
   int cls[4*MAX_CLS],no_cls,max_cls=MAX_CLS,*chr;
   int n,i,i1,j1;
   double dx,dy;
   float fthreshold,fmin_eff;

   if (g->dim!=2)
   {
      printf("gf_dc_2dcls: this function only works with 2D grids\n");
      return 0;
   }

   if (first_call) 
   { 
      if (args && strlen(args)>0)
      {
         sscanf(args,"%f,%i,%i,%f,%i",&fthreshold,&minwidth,&buf_wid,&fmin_eff,&blk_fac);
         threshold=fthreshold;
         min_eff=fmin_eff;
      }
      else
      {
         threshold=1; 
         minwidth=buf_wid=0;
         min_eff=0.5;
         blk_fac=1;
      }
      if (blk_fac<1) blk_fac=1;
      if (min_eff<0 || min_eff>1) min_eff=0.5;
      if (minwidth<0) minwidth=0;
      if (buf_wid<0) buf_wid=0;
      printf("gf_dc_2dcls: threshold=%f, minwidth=%i, buf_wid=%i, min_eff=%f, blk_fac=%i\n",
             threshold,minwidth,buf_wid,min_eff,blk_fac);
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);
   if (!(chr=(int *)malloc(sizeof(int)*n)))
   {
      printf("gf_dc_2dcls: out of memory\n");
      gfree(ng);
      return 0;
   }
   for(i=0;i<n;i++) if (ng->data[i]<threshold) ng->data[i]=chr[i]=0; else ng->data[i]=chr[i]=1;

   _dc_cls_2d__(chr,cls,&no_cls,&max_cls,&minwidth,
               &buf_wid,&min_eff,&blk_fac,work,&(ng->shape[0]),&(ng->shape[1]));

   free(chr);
   for(i=0; i<no_cls; i++)
   {
      shape[0]=cls[4*i+2]-cls[4*i]+1;
      shape[1]=cls[4*i+3]-cls[4*i+1]+1;
      if (!(ng2=galloc(g->dim,g->coord_type,shape,g->time,0,0,0)))
      {
         printf("gf_dc_2dcls: ERROR trying to allocate grid\n");
         gfree(ng);
         return 0;
      }
      dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
      dy=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
      ng2->coords[0]=g->coords[0]+dx*(cls[4*i]-1);
      ng2->coords[2]=g->coords[2]+dy*(cls[4*i+1]-1);
      ng2->coords[1]=g->coords[0]+dx*(cls[4*i+2]-1);
      ng2->coords[3]=g->coords[2]+dy*(cls[4*i+3]-1);
      n=sizeof_data(ng2);
      for(j1=0;j1<ng2->shape[1];j1++)
         for(i1=0;i1<ng2->shape[0];i1++)
           ng2->data[i1+j1*ng2->shape[0]]=ng->data[(cls[4*i]+i1-1)+(cls[4*i+1]+j1-1)*ng->shape[0]];
      if (!(add_grid_str(nf,ng2)))
         printf("gf_dc_2dcls: ERROR adding cluster grid\n");
   }

   return ng;
}

//=============================================================================
// graxi specific
//
// a is the function to smooth
// b is f_surf
//
// NOTE: using same memory for chr2 and fn!
//=============================================================================
#ifdef __
extern void apply_dis_ex_bdy__(double *f, double *chr, double *f_surf, double *chr2,
                        double *fn, double *eps,int *use_exact_tangent,double *rho,
                        double *z, int *Nrho, int *Nz);
#else
extern void apply_dis_ex_bdy_(double *f, double *chr, double *f_surf, double *chr2,
                        double *fn, double *eps,int *use_exact_tangent,double *rho,
                        double *z, int *Nrho, int *Nz);
#endif
grid *gf_apply_dis_ex_bdy(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, 
                          char *nf, char *args, int first_call)
{  
   int i,use_exact_tangent=0;
   static int num;
   double h,*cmask,*f,eps=0.5,*rho=0,*z=0,drho,dz;
   grid *g=it_a->g,*ng,*ng2;

   if (!mask)
   {
      printf ("gf_apply_dis_ex_bdy: mask required\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   if (!(ng2=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) {gfree(ng); return 0;}

   if (!(rho=(double *)malloc(sizeof(double)*g->shape[0])) ||
       !(z=(double *)malloc(sizeof(double)*g->shape[1])))
   {
      printf ("gf_apply_dis_ex_bdy: out of memory\n");
      gfree(ng); gfree(ng2);
      if (rho) free(rho);
      if (z) free(z);
      return 0;
   }

   drho=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   dz=(g->coords[3]-g->coords[2])/(g->shape[1]-1);

   for (i=0;i<g->shape[0];i++) rho[i]=drho*i+g->coords[0];
   for (i=0;i<g->shape[1];i++) z[i]=dz*i+g->coords[2];
      
   if (first_call) 
   { 
      if (args) num=atoi(args); 
      else num=1; 
      if (num<1) num=1;
   }

   for (i=0; i<num; i++)
      _apply_dis_ex_bdy__(ng->data,mask->data,it_b->g->data,ng2->data,ng2->data,&eps,
                       &use_exact_tangent,rho,z,&g->shape[0],&g->shape[1]);

   gfree(ng2);
   free(rho);
   free(z);
   return ng;
}

//=============================================================================
// graxi + other -specific
//=============================================================================
extern void extend_(double *f, int *k, double *chr, int *first_n, int *axis, double *rho, double *z, int *Nrho, int *Nz);
extern void dmdiss3d_(double *f,double *work, double *eps, int *do_bdy, 
                      int *phys_bdy_type, int *even, int *odd, double *mask, double *mask_off, 
                      int *nx, int *ny, int *nz, double *chr, double *ex, int *do_ex);
extern void dmrepop3d1_(double *f, double *chr, double *ex, int *io, int *nx, int *ny, int *nz);
extern void smooth_ah_r_(double *AH_R, double *AH_var, double *AH_w1, double *AH_eps, int *AH_Ntheta, int *AH_Nphi);
extern void cylm_coeff_(double *clm_r,double *clm_i,double *f_r,double *f_i,int *l,int *m,int *Ntheta,int *Nphi);
extern void cm2y2pm2_coeff_(double *clm_r,double *clm_i,double *f_r,double *f_i,int *l,int *m,int *Ntheta,int *Nphi);
extern void eval_m2ylm_l6_(double *q,double *x);
extern void cm2yl6m_coeff_(double *clm_r,double *clm_i,double *f_r,double *f_i,int *Ntheta,int *Nphi);
extern void fill_cylm_(double *f_r,double *f_i,int *l,int *m,int *Ntheta,int *Nphi);
extern void fill_cm2y2pm2_(double *f_r,double *f_i,int *l,int *m,int *Ntheta,int *Nphi);
#ifdef __
extern void fill_eps__(double *eps, double *eps_max, int *flag, double *chr, int *Nrho, int *Nz);
extern void diss_ko__(double *f_new,double *f_old, double *eps, int *k, int *axis, int *Nrho, int *Nz);
extern void diss_ko_axis__(double *f_new,double *f_old, double *eps, int *k, int *axis, int *Nrho, int *Nz);
extern void diss_ko2__(double *fn, double *fo, double *chr, double *mask_val, double *eps, int *k,
                       int *axis, int *Nrho, int *Nz);
extern void distance_2bh__(double *psi, double *mask, double *d, double *dz, int *Nrho, int *Nz);
extern void d_overrho__(double *f, double *df, double *mask, double *mask_val, double *rhomin, double *rhomax,
                        int *Nrho, int *Nz);
extern void d_overrho_d__(double *f, double *df, double *mask, double *mask_val, double *rhomin, double *rhomax,
                        int *Nrho, int *Nz);
extern void d_byrhosq__(double *f, double *df, double *mask, double *mask_val, double *rhomin, double *rhomax,
                        int *Nrho, int *Nz);    
extern void int_f_ds__(double *f, double *intf, double *r, double *one, double *p, int *l, 
                      double *fc, double *rhomin, double *rhomax, double *zmin, double *zmax, 
                      int *Nrho, int *Nz);
extern void uni_avg__(double *fn, double *fo, double *chr, double *mask_val, int *n,
                       int *axis, int *Nrho, int *Nz);
extern void separate_rb__(double *fo, double *fn, int *Nrho, int *Nz);
#else
extern void fill_eps_(double *eps, double *eps_max, int *flag, double *chr, int *Nrho, int *Nz);
extern void diss_ko_(double *f_new,double *f_old, double *eps, int *k, int *axis, int *Nrho, int *Nz);
extern void diss_ko_axis_(double *f_new,double *f_old, double *eps, int *k, int *axis, int *Nrho, int *Nz);
extern void distance_2bh_(double *psi, double *mask, double *d, double *dz, int *Nrho, int *Nz);
extern void d_overrho_(double *f, double *df, double *mask, double *mask_val, double *rhomin, double *rhomax,
                        int *Nrho, int *Nz);
extern void int_f_ds_(double *f, double *intf, double *r, double *one, double *p, int *l, 
                      double *fc, double *rhomin, double *rhomax, double *zmin, double *zmax, 
                      int *Nrho, int *Nz);
extern void d_overrho_d_(double *f, double *df, double *mask, double *mask_val, double *rhomin, double *rhomax,
                        int *Nrho, int *Nz);
extern void d_byrhosq_(double *f, double *df, double *mask, double *mask_val, double *rhomin, double *rhomax,
                        int *Nrho, int *Nz);    
extern void diss_ko2_(double *fn, double *fo, double *chr, double *mask_val, double *eps, int *k,
                       int *axis, int *Nrho, int *Nz);
extern void uni_avg_(double *fn, double *fo, double *chr, double *mask_val, int *n,
                       int *axis, int *Nrho, int *Nz);
extern void separate_rb_(double *fo, double *fn, int *Nrho, int *Nz);
extern void diss_ko_1d_(double *f, double *eps, int *k, int *num, int *N);
extern void fill_i4psi4_(double *i4psi4_r,double *i4psi4_i,double *r0,double *m1,double *x1,double *y1,double *z1,
                         double *m2,double *x2,double *y2,double *z2, int *Ntheta, int *Nphi);
#endif

//=============================================================================
// from Smarr's article in sources of grav. rad. 1979
//
// de/dt = (1/4pi) * integral ( p  * r^2 d omega ), r=constant
//
// p=(integral [0..t] (psi4 dt))^2
// 
// NOTE: I'm pretty sure the above definition must be for a tetrad
//       as defined in Teukolsky's (73) paper. i.e. with ell_{-} = (n - r)/2.
//       I typically use ell_{-} = (n-r)/sqrt(2) ... which therefore requires
//       division the above answer by 4.
//
// NOTE: JULY 11 2006 : CHANGED TO NOW BY DEFAULT DIV. BY 4
//=============================================================================
grid *gf_de_dt(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   static double r;
   int n,i,j,zero,ind;
   grid *g=it_a->g,*ng,*temp,*ng2;
   static grid *pg;
   static double dt,tot_e; 
   double *p,*q,res,one,test_one,fc,psi4r_c,dx,dy,xi,yi;
   static double *de_dt_data,*tot_e_data,*psi4r_data;
   static int num_t,ct;
   time_str *ts;
   static double t_coords[2];
     
   if (g->dim!=2)
   {
      printf("gf_de_dt: dim must be 2\n");
      return 0;
   }

   if (first_call)
   {
      pg=0; 
      tot_e=0; 
      ts=it_a->ts; num_t=0; ct=0;
      t_coords[0]=ts->time;
      while(ts) {  t_coords[1]=ts->time; num_t++; ts=ts->next; }
      de_dt_data=(double *)malloc(sizeof(double) *num_t);
      tot_e_data=(double *)malloc(sizeof(double) *num_t);
      psi4r_data=(double *)malloc(sizeof(double) *num_t);

      printf("\n\nNOTE:Expression for E assumes `symmetric' 1/sqrt(2) normalization.\n"
             "multiply by 4 for Teukolsky's (73) normalization of tetrads\n\n");

   }
   else
   {
      ct++;
   }
   
   if (first_call) printf("gf_de_dt: NOTE --- first order accurate in time only!\n");
   get_single_farg(&r,args,first_call,1.0,"gf_de_dt"); 

   if (it_a->ts->next) dt=(it_a->ts->next->time-it_a->ts->time);

   n=sizeof_data(g);
   if (pg)
   {
      if (gridcmp(pg,g))
      {
         printf("gf_de_dt: Error --- need indentical grid structure in time\n");
         return 0;
      }
      if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,pg->data))) return 0;
   }
   else
   {
      if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;
      i=n; p=ng->data; while(i--) *p++=0;
   }
   
   if (!(temp=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) { gfree(ng); return 0; }
   pg=ng;
   i=n; p=ng->data;q=g->data;
   while(i--) { (*p)+=(*q)*dt; p++; q++; }
   i=n; p=temp->data;q=ng->data;
   while(i--) { *p=(*q)*(*q); p++; q++; }

   zero=0;
   one=1.0;
   _int_f_ds__(temp->data,&res,&r,&test_one,&one,&zero,&fc,&(g->coords[0]),&(g->coords[1]),&(g->coords[2]),&(g->coords[3]),
             &(g->shape[0]),&(g->shape[1]));
   res/=(16*M_PI);
   tot_e+=res*dt;
   dy=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
   yi=g->coords[2];
   dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   xi=g->coords[0];
   i=(r-xi)/dx;
   j=(-yi)/dy;
   ind=i+j*g->shape[0];
   if (ind<n) psi4r_c=g->data[ind]*r; else psi4r_c=0;

   if (first_call) printf("t\t\tde_dt\t\ttotal de\tpsi4*r(z=0,rho=r)\t\tdt\t\t1\n");

   printf("%f\t%e\t%e\t%e\t%f\t%e\n",it_a->ts->time,res,tot_e,psi4r_c,dt,test_one);

   if (de_dt_data) de_dt_data[ct]=res;
   if (tot_e_data) tot_e_data[ct]=tot_e;
   if (psi4r_data) psi4r_data[ct]=psi4r_c;

   if (ct==(num_t-1))
   {
      if (de_dt_data)
      {
         if (!add_grid("de_dt(t)",0,1,COORD_UNIFORM,&num_t,t_coords,0,de_dt_data))
            printf("gf_de_dt: error adding grid to register <de_dt(t)>\n");
         free(de_dt_data); de_dt_data=0;
      }
      if (tot_e_data)
      {
         if (!add_grid("tot_e(t)",0,1,COORD_UNIFORM,&num_t,t_coords,0,tot_e_data))
            printf("gf_de_dt: error adding grid to register <tot_e(t)>\n");
         free(tot_e_data); tot_e_data=0;
      }
      if (psi4r_data)
      {
         if (!add_grid("psi4r(t,r0)",0,1,COORD_UNIFORM,&num_t,t_coords,0,psi4r_data))
            printf("gf_de_dt: error adding grid to register <psi4r(t)>\n");
         free(psi4r_data); psi4r_data=0;
      }
   }

   gfree(temp);

   return ng;

}
grid *gf_int_fds(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   static double r,p;
   static int l;
   float fr,fp;
   grid *g=it_a->g;
   double intf,one,fc;
     
   if (g->dim!=2)
   {
      printf("gf_int_fds: dim must be 2\n");
      return 0;
   }
   
   if (first_call) 
   { 
      if (args)
      {
         sscanf(args,"%f,%f,%i",&fr,&fp,&l);
         r=fr;p=fp;
      }
      else
      {
         r=1; 
         p=1;
         l=0;
      }
      printf("gf_int_fds: r=%f, p=%f, l=%i\n",r,p,l);
      printf("t\t\t int(%s) \t\t f(r,0) \t\t one\n:",it_a->r->name);
   }

   _int_f_ds__(g->data,&intf,&r,&one,&p,&l,&fc,&(g->coords[0]),&(g->coords[1]),&(g->coords[2]),&(g->coords[3]),
             &(g->shape[0]),&(g->shape[1]));

   printf("%f\t%e\t%e\t%f\t\n",it_a->ts->time,intf,fc,one);

   return 0;

}

grid *gf_extend(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   static int tnum,num,clip,kmax,kcell,axis,kmax_axis;
   static double eps_max;
   double *f,*rho=0,*z=0,drho,dz,*p,*q;
   float feps;
   grid *g=it_a->g,*ng=0,*ng2=0,*eps=0,*nmask=0,*tg;
   int i,j,n,k,l,flag,first_n;

   if (!mask)
   {
      printf ("gf_extend: mask required\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) goto cleanup;
   if (!(ng2=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) { gfree(ng); ng=0; goto cleanup; }
   if (!(eps=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) { gfree(ng); ng=0; goto cleanup; }
   if (!(nmask=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,mask->data))) { gfree(ng); ng=0; goto cleanup; }

   if (!(rho=(double *)malloc(sizeof(double)*g->shape[0])) ||
       !(z=(double *)malloc(sizeof(double)*g->shape[1])))
   {
      gfree(ng); ng=0; goto cleanup;
   }

   drho=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   dz=(g->coords[3]-g->coords[2])/(g->shape[1]-1);

   for (i=0;i<g->shape[0];i++) rho[i]=drho*i+g->coords[0];
   for (i=0;i<g->shape[1];i++) z[i]=dz*i+g->coords[2];
      
   if (first_call) 
   { 
      if (args)
      {
         sscanf(args,"%i,%i,%i,%i,%i,%i,%i,%f,%i",&tnum,&num,&axis,&clip,&kmax,&kmax_axis,&kcell,&feps,&first_n);
         eps_max=feps;
         if (axis<0 || axis>2)
             printf("gf_extend: illegal second argument %i, must be 0,1 or 2\nusing 2",axis);
      }
      else
      {
         num=1; 
         tnum=1;
         axis=2;
      }
      if (num<0) num=0;
      if (tnum<1) tnum=1;
      if (axis<0 || axis>2) axis=2;
      if (kmax<1) kmax=1;
      if (kmax_axis<1) kmax_axis=0;
      if (kcell<1) kcell=1;
      if (first_n<0) first_n=0;
      printf("gf_extend: tnum=%i, num=%i,axis=%i,clip=%i,kmax=%i,kmax_axis=%i,kcell=%i,eps=%f,first_n=%i\n",
             tnum,num,axis,clip,kmax,kmax_axis,kcell,eps_max,first_n);
   }

   n=sizeof_data(ng);
   for (l=0; l<tnum; l++)
   {
      for(j=0,p=ng->data,q=mask->data; j<n; j++,q++,p++) if ((*q)==mask_val) *p=0;
      for(j=0,p=nmask->data,q=mask->data; j<n; j++) *p++=*q++;
      extend_(ng->data,&kcell,nmask->data,&first_n,&axis,rho,z,&g->shape[0],&g->shape[1]);
      flag=1;
      _fill_eps__(eps->data,&eps_max,&flag,mask->data,&g->shape[0],&g->shape[1]);
      for (i=0; i<(num-1) && num; i++)
      {
         for (k=1; k<=kmax; k++)
         {
            _diss_ko__(ng2->data,ng->data,eps->data,&k,&axis,&g->shape[0],&g->shape[1]);
            for (j=0,p=ng->data,q=ng2->data; j<n; j++) *p++=*q++;
         }
         for (k=1; k<=kmax_axis; k++)
         {
            _diss_ko_axis__(ng2->data,ng->data,eps->data,&k,&axis,&g->shape[0],&g->shape[1]);
            for (j=0,p=ng->data,q=ng2->data; j<n; j++) *p++=*q++;
         }
      }
      if (num)
      {
         k=1;
         flag=0;
         _fill_eps__(eps->data,&eps_max,&flag,mask->data,&g->shape[0],&g->shape[1]);
         _diss_ko__(ng2->data,ng->data,eps->data,&k,&axis,&g->shape[0],&g->shape[1]);
         for (j=0,p=ng->data,q=ng2->data; j<n; j++) *p++=*q++;
      }
   }

   if (clip) for (j=0,p=ng->data,q=mask->data; j<n; j++,q++,p++) if ((*q)==mask_val) *p=0;

cleanup:
   if (rho) free(rho);
   if (z) free(z);
   if (nmask) gfree(nmask);
   if (ng2) gfree(ng2);
   if (eps) gfree(eps);

   if (!ng) printf ("gf_extend: out of memory\n");

   return ng;
}

grid *gf_diss_ko2(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   static int tnum,k,axis;
   static double eps;
   grid *g=it_a->g,*ng=0;
   double *maskd=0,*tmp,*q,*p,maskv;
   int l,n,j,ck;
   float feps;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)))
   {
      printf("gf_diss_ko2: out of memory\n");
      return 0;
   }

   if (first_call) 
   { 
      if (args)
      {
         sscanf(args,"%i,%f,%i,%i",&tnum,&feps,&k,&axis);
         eps=feps;
         if (axis<0 || axis>2)
            printf("gf_diss_ko2: illegal second argument %i, must be 0,1 or 2\nusing 2",axis);
      }
      else
      {
         tnum=1; 
         axis=2;
         eps=0.5;
         k=1;
      }
      if (tnum<1) tnum=1;
      if (axis<0 || axis>2) axis=2;
      if (k<1) k=1;
      printf("gf_diss_ko2: tnum=%i, eps=%f, k=%i, axis=%i\n",
             tnum,eps,k,axis);
   }

   n=sizeof_data(ng);
   if (!(tmp=(double *)malloc(sizeof(double)*n)))
   {
      printf("gf_diss_ko2: out of memory\n");
      gfree(ng);
      return 0;
   }
   if (mask)
   {
      maskd=mask->data;
      maskv=mask_val;
   }
   else
   {
      maskd=tmp;
      maskv=1.0e-10;
   }
   for (l=0; l<tnum; l++)
   {
      for (ck=1; ck<=k; ck++)
      {
         for(j=0,p=ng->data,q=tmp; j<n; j++) *q++=*p++;
         _diss_ko2__(ng->data,tmp,maskd,&maskv,&eps,&ck,&axis,&g->shape[0],&g->shape[1]);
      }
   }

   free(tmp);
   return ng;
}

//-----------------------------------------------------------------------------
// gh3d specific, calls dmdiss3d_()
//-----------------------------------------------------------------------------
grid *gf_dmdiss3d(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   static int tnum,do_bdy,phys_bdy[6],even=1,odd=2;
   static double eps;
   grid *g=it_a->g,*ng=0;
   double *maskd=0,*work,*tmp_mask,*q,*p,maskv;
   int n,j,do_ex=1;
   float feps;

   if (g->dim!=3)
   {
      printf("gf_dmdiss3d: this function requires a 3D grid\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)))
   {
      printf("gf_dmdiss3d: out of memory\n");
      return 0;
   }

   if (first_call) 
   {
      phys_bdy[0]=phys_bdy[1]=phys_bdy[2]=phys_bdy[3]=phys_bdy[4]=phys_bdy[5]=0;
      if (args)
      {
         sscanf(args,"%i,%f,%i,%i,%i,%i,%i,%i,%i",&tnum,&feps,&do_bdy,
                &phys_bdy[0],&phys_bdy[1],&phys_bdy[2],&phys_bdy[3],&phys_bdy[4],&phys_bdy[5]);
         eps=feps;
      }
      else
      {
         tnum=1; 
         eps=0.5;
         do_bdy=0;
      }
      if (tnum<1) tnum=1;
      printf("gf_dmdiss3d: tnum=%i, eps=%f, do_bdy=%im phys_bdy=[%i,%i,%i,%i,%i,%i]\n",
             tnum,eps,do_bdy,phys_bdy[0],phys_bdy[1],phys_bdy[2],phys_bdy[3],phys_bdy[4],phys_bdy[5]);
   }

   n=sizeof_data(ng);
   if (!(tmp_mask=(double *)malloc(sizeof(double)*n)))
   {
      printf("gf_dmdiss3d: out of memory\n");
      gfree(ng);
      return 0;
   }
   if (!(work=(double *)malloc(sizeof(double)*n)))
   {
      printf("gf_dmdiss3d: out of memory\n");
      free(tmp_mask);
      gfree(ng);
      return 0;
   }
   for (j=0; j<n; j++) tmp_mask[j]=mask_val+1;
   if (mask)
   {
      for (j=0; j<n; j++) if (mask->data[j]==mask_val) tmp_mask[j]=mask_val;
      do_ex=1;
   }
   else do_ex=0;
   for (j=0; j<tnum; j++)
   {
      dmdiss3d_(ng->data,work,&eps,&do_bdy,phys_bdy,&even,&odd,
                tmp_mask,&mask_val,&g->shape[0],&g->shape[1],&g->shape[2],tmp_mask,&mask_val,&do_ex);
   }

   free(tmp_mask);
   free(work);
   return ng;
}

//-----------------------------------------------------------------------------
// gh3d specific, calls dmrepop3d1_()
//-----------------------------------------------------------------------------
grid *gf_dmrepop3d(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   static int io;
   static double eps;
   grid *g=it_a->g,*ng=0;
   double *maskd=0,*tmp_mask,*q,*p,maskv;
   int n,j;

   if (g->dim!=3)
   {
      printf("gf_dmrepop3d: this function requires a 3D grid\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)))
   {
      printf("gf_dmrepop3d: out of memory\n");
      return 0;
   }

   if (first_call) 
   { 
      if (args)
      {
         sscanf(args,"%i",&io);
      }
      else
      {
         io=2; 
      }
      if (io<2 || io>4) io=2;
      printf("gf_dmrepop3d: io=%i\n",io);
   }

   n=sizeof_data(ng);
   if (!(tmp_mask=(double *)malloc(sizeof(double)*n)))
   {
      printf("gf_dmrepop3d: out of memory\n");
      gfree(ng);
      return 0;
   }

   for (j=0; j<n; j++) tmp_mask[j]=mask_val+1;
   if (mask)
      for (j=0; j<n; j++) if (mask->data[j]==mask_val) tmp_mask[j]=mask_val;

   dmrepop3d1_(ng->data,tmp_mask,&mask_val,&io,&g->shape[0],&g->shape[1],&g->shape[2]);

   free(tmp_mask);
   return ng;
}

//=============================================================================
// gh3d specific ... input is psi4_r in spherical polar coordinates i.e.,
// as produce by eval_r, and is expected to be uniform/unigrid. 
// Argument is R, and *must* be the same value used in eval_r to get the 
// correct answer. 
//
// from Smarr's article in sources of grav. rad. 1979
//
// de/dt = (1/4pi) * integral ( p  * r^2 d omega ), r=constant
//
// p=(integral [0..t] (psi4 dt))^2
//
// see note above for normalization
//
// JULY 11 2006: CHANGED TO NOW DIV. BY 4 !!!
//
// This function can also compute the components of net momentum flux 
// (assuming the energy flow is purely radial) via:
// 
// d(P^i)/dt = (1/4pi) * integral ( p rh^i * r^2 d omega ), r=constant
//
// where rh^i is a unit vector pointing in the radial direction (in 'normal'
// units would multiply this by a extra factor of c compared to dE/dt).
//
// in components rh^i=(cos(phi)*sin(theta),sin(phi)*cos(theta),cos(theta))
//
// arg is R,comp ... if comp=0, energy
//                          =1, px
//                          =2, py
//                          =3, pz
//
//
// currently need to use int_psi4_spec_r to compute Jz
//=============================================================================
grid *gf_int_psi4_r(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng=0,*nxt_g;
   static double R,tn,tnm1,tnm2,*p,pt,*p4n,*p4nm1,*p4nm2,*de_dt,E,coords[2];
   static int n,numt,dsize,Ntheta,Nphi,test,which_comp;
   int i,j,ij,ip1j,ijp1,ip1jp1;
   double st,ct,sp,cp,dtheta,dphi,weight;

   if (g->dim>2) {printf("gf_int_psi4_r: dim too large\n"); return 0;}

   if (first_call) 
   { 
      which_comp=0; R=-1.0; sscanf(args,"%lf,%i",&R,&which_comp);
      if (which_comp<0 || which_comp>3) { printf("gf_int_psi4_r: WARNING which_comp must be 0(energy),"
                                                 " 1..3 (component of momentum) ... setting to zero\n"); which_comp=0;} 
      if (which_comp!=0 && g->dim==1) { printf("gf_int_psi4_r: WARNING which_comp can only be 0(energy)"
                                               " in axisymmetry at this stage ... setting to zero\n"); which_comp=0;} 

      printf("\n\nNOTE:Expression for E assumes `symmetric' 1/sqrt(2) normalization.\n"
             "multiply by 4 for Teukolsky's (73) normalization of tetrads\n\n");
      test=0;
      if (R<=0) {printf("gf_int_psi4_r: positive R argument required ... running test-case with psi=sin(theta)\n"); test=1; R=1;}
      tn=0; tnm1=0; tnm2=0; E=0;
      p4n=p4nm1=p4nm2=0;
      save_s_iter(it_a);
      nxt_g=g;
      pt=it_a->ts->time;
      numt=1; n=1;
      while(nxt_g) 
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            pt=it_a->ts->time;
         } 
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);
      dsize=sizeof_data(g);
      Ntheta=g->shape[0];
      if (g->dim==2) Nphi=g->shape[1]; else Nphi=1;
      if (!(p=(double *)malloc(sizeof(double)*dsize))) 
         { printf("gf_int_psi4_r : error ... out of memory\n"); return 0; }
      if (!(de_dt=(double *)malloc(sizeof(double)*numt))) 
         { printf("gf_int_psi4_r : error ... out of memory\n"); free(p); return 0; }
      for (i=0; i<numt; i++) de_dt[i]=0;
      for (i=0; i<dsize; i++) p[i]=0;
      tnm2=0; tnm1=0; tn=it_a->ts->time; coords[0]=tn;
      p4n=g->data;
   }
   else
   {
      if (!p) return 0;
      n++;
      tnm2=tnm1; tnm1=tn; tn=it_a->ts->time;
      p4nm2=p4nm1; p4nm1=p4n; p4n=g->data;
      if (tn==tnm1 || sizeof_data(g)!=dsize) { printf("gf_int_psi4_r : error ... uniform/unigrid function expected\n"); return 0; }

      if (n>2 && !fuzz_eq(tn-tnm1,tnm1-tnm2,(tn-tnm1)/1e6)) printf("gf_int_psi4_r : WARNING uniform in time assumed!\n");

      dtheta=M_PI/(Ntheta-1);
      if (Nphi>1) dphi=2*M_PI/(Nphi-1); else dphi=0;

      if (test)
      {
         for (i=0; i<Ntheta; i++)
         {
            st=sin(i*dtheta);
            if (Nphi>1)
            {
               for (j=0; j<Nphi; j++)
               {
                  ij=i+j*g->shape[0];
                  p[ij]+=((tn-tnm1)*st*0.5*((tn-coords[0])+(tnm1-coords[0])));
               }
            }
            else
            {
               p[i]+=((tn-tnm1)*st*0.5*((tn-coords[0])+(tnm1-coords[0])));
            }
         }
      }
      else
      {
         for (i=0; i<dsize; i++) p[i]+=(0.5*(p4n[i]+p4nm1[i])*(tn-tnm1)); // simpson's rule
      }

      for (i=0; i<(Ntheta-1); i++)
      {
         st=sin(i*dtheta+dtheta/2);
         ct=cos(i*dtheta+dtheta/2);
         if (Nphi>1)
         {
            for (j=0; j<(Nphi-1); j++)
            {
               sp=sin(j*dphi+dphi/2);
               cp=cos(j*dphi+dphi/2);
               ij=i+j*g->shape[0];
               ip1j=i+1+j*g->shape[0];
               ijp1=i+(j+1)*g->shape[0];
               ip1jp1=i+1+(j+1)*g->shape[0];

               // don't include theta=0 or PI points ... tetrad is singular there
               // and results crap

               switch(which_comp)
               {
                  case 0: weight=1; break;
                  case 1: weight=cp*st; break;
                  case 2: weight=sp*st; break;
                  case 3: weight=ct; break;
               }

               if (i==0)
                  de_dt[n-1]+=((R*R*weight*st/16/M_PI*dtheta*dphi)*
                               ((p[ip1j]*p[ip1j]+p[ip1jp1]*p[ip1jp1])/2));
               else if (i==(Ntheta-2))
                  de_dt[n-1]+=((R*R*weight*st/16/M_PI*dtheta*dphi)*
                               ((p[ij]*p[ij]+p[ijp1]*p[ijp1])/2));
               else
                  de_dt[n-1]+=((R*R*weight*st/16/M_PI*dtheta*dphi)*
                               ((p[ij]*p[ij]+p[ip1j]*p[ip1j]+p[ijp1]*p[ijp1]+p[ip1jp1]*p[ip1jp1])/4));
            }
         }
         else
         {
            //printf("i=%i, R=%lf, st=%lf, dtheta=%lf, p[i]=%lf, p[i+1]=%lf\n",i,R,st,dtheta,p[i],p[i+1]);
            de_dt[n-1]+=((R*R*st/8*dtheta)*((p[i]*p[i]+p[i+1]*p[i+1])/2));
         }
      }
      E+=(0.5*(de_dt[n-1]+de_dt[n-2])*(tn-tnm1));
   }

   if (n==numt)
   {
      coords[1]=tn;
      if (!(ng=galloc(1,COORD_UNIFORM,&numt,0.0,coords,0,de_dt))) return 0;
      free(p); p=0;
      free(de_dt); de_dt=0;
      switch(which_comp)
      {
         case 0: printf("Total energy radiated : %16.12lf\n",E); break;
         case 1: printf("Total x-component of momentum radiated : %16.12lf\n",E); break;
         case 2: printf("Total y-component of momentum radiated : %16.12lf\n",E); break;
         case 3: printf("Total z-component of momentum radiated : %16.12lf\n",E); break;
      }
      if (test) printf("Test ... expect %16.12lf\n",pow(coords[1]-coords[0],5)/30);
   }

   return ng;
}

//=============================================================================
// 1D dissipation routine
//=============================================================================
grid *gf_diss_ko_1d(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng=0;
   static double eps;
   static int k_max,num;

   if (g->dim>1) {printf("gf_diss_ko_1d: only 1D at the moment\n"); return 0;}
   if (mask) {printf("gf_diss_ko_1d: a mask not yet supported\n"); return 0;}

   if (first_call) 
   { 
      eps=1; k_max=1; num=1;
      sscanf(args,"%lf,%i,%i",&eps,&k_max,&num);
      printf("gf_diss_ko_1d: eps=%lf, k_max=%i, num=%i\n",eps,k_max,num);
   }

   ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data);

   diss_ko_1d_(ng->data,&eps,&k_max,&num,g->shape);

   return ng;
}

//=============================================================================
// gh3d specific ... computes the complex ylm spherical harmonic
// coefficient of a UNIGRID function of theta,phi
// 
// it_a is real component, it_b is imaginary ... creates 1d in time grid
// functions
//
// range specifies a range of values to create a 2D array ng,
//
// where ng[l+range,m+range] stores the [l,m] real component
// and   ng[range-l,m+range] stores the [l,m] imaginary component
//
// if range, then also produce a 'filtered' version of the function
// which is sum of coefficient functions in the range
//
//=============================================================================
grid *gf_cylm(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g_r=it_a->g,*ng_r=0,*nxt_g_r,*ng;
   grid *g_i=it_b->g,*ng_i=0,*f_ng_r=0,*f_ng_i=0,*tmp_r,*tmp_i;
   static int l,m,numt,n,range,rshape[2];
   static double *clm_r,*clm_i,*r_clm_r,*r_clm_i,pt,dt,rcoords[4],coords[2];
   int i,j,ind,ltrace=1,test=0,ms,me,ls,le,m0,l0;
   double ta,tb,a1r,a1i,anr,ani,cr,ci;
   static char tag[256];
   char buf[256];

   if (g_r->dim!=2) {printf("gf_cylm: dim !=2\n"); return 0;}
   if (g_r->coord_type!=COORD_UNIFORM) {printf("gf_cylm: expects uniform coordinates\n"); return 0;}

   if (first_call) 
   { 
      sscanf(args,"%i,%i,%i,%s",&l,&m,&range,tag);
      if (range>1)
      {
         rshape[0]=rshape[1]=2*range+1;
         rcoords[0]=rcoords[2]=-range;
         rcoords[1]=rcoords[3]=range;
      }
      clm_r=0; clm_i=0;
      save_s_iter(it_a);
      nxt_g_r=g_r;
      pt=it_a->ts->time;
      numt=1;n=0;
      coords[0]=pt;
      while(nxt_g_r)
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            dt=it_a->ts->time-pt;
            pt=it_a->ts->time;
         } 
         nxt_g_r=next_g(it_a);
      }
      restore_s_iter(it_a);
      pt=coords[0];
      if (!(clm_r=(double *)malloc(sizeof(double)*numt)) ||
          !(clm_i=(double *)malloc(sizeof(double)*numt)))
      { 
         printf("gf_cylm : error ... out of memory\n"); 
         if (clm_r) free(clm_r); 
         if (clm_i) free(clm_i); 
         clm_r=clm_i=0;
         return 0; 
      }
      for (i=0; i<numt; i++) clm_r[i]=clm_i[i]=0;
      printf("gf_cylm: l=%i, m=%i, dt=%lf, numt=%i, range=%i\n\n",l,m,dt,numt,range);
   }


   if (!clm_r) return 0;
   n++;
   ta=it_a->ts->time;
   tb=it_b->ts->time;

   if (range)
   {
      if (!(ng=galloc(2,COORD_UNIFORM,rshape,ta,rcoords,0,0))) return 0;
      for (i=0; i<sizeof_data(ng); i++) ng->data[i]=0;
      ls=0,le=range;
      ms=-range; me=range;

      if (!(f_ng_r=galloc(g_r->dim,g_r->coord_type,g_r->shape,ta,g_r->coords,0,0))) return 0;
      if (!(f_ng_i=galloc(g_r->dim,g_r->coord_type,g_r->shape,ta,g_r->coords,0,0))) return 0;
      if (!(tmp_r=galloc(g_r->dim,g_r->coord_type,g_r->shape,ta,g_r->coords,0,0))) return 0;
      if (!(tmp_i=galloc(g_r->dim,g_r->coord_type,g_r->shape,ta,g_r->coords,0,0))) return 0;
      for (i=0; i<tmp_r->shape[0]*tmp_r->shape[1]; i++) f_ng_r->data[i]=f_ng_i->data[i]=0;
   }
   else
   {
      ms=me=m;
      ls=le=l;
      ng=0;
   }

   if (n>2 && fabs((ta-pt)/dt-1)>1.0e-6) printf("gf_cylm : WARNING uniform in time assumed for single time output, t=%lf!\n",ta);

   pt=ta;

   if (ta!=tb) { printf("gf_cylm : error ... time structures for real & imaginary components do not match\n"); return 0; }

   for (l0=ls; l0<=le; l0++)
   {
      for (m0=ms; m0<=me; m0++)
      {
         if (abs(m0)<=l0)
         {
            cylm_coeff_(&cr,&ci,g_r->data,g_i->data,&l0,&m0,&g_r->shape[0],&g_r->shape[1]);
            if (range)
            {
               ind=(l0+range)+(m0+range)*ng->shape[0];
               ng->data[ind]=cr;
               ind=(range-l0)+(m0+range)*ng->shape[0];
               ng->data[ind]=ci;

               fill_cylm_(tmp_r->data,tmp_i->data,&l0,&m0,&tmp_r->shape[0],&tmp_r->shape[1]);
               for (i=0; i<tmp_r->shape[0]*tmp_r->shape[1]; i++) 
               {
                  f_ng_r->data[i]+=(cr*tmp_r->data[i]-ci*tmp_i->data[i]);
                  f_ng_i->data[i]+=(ci*tmp_r->data[i]+cr*tmp_i->data[i]);
               }
            }
            if (l0==l && m0==m) {clm_r[n-1]=cr; clm_i[n-1]=ci;}
         }
      }
   }

   if (ltrace) printf("  t(%i)=%lf, clm_r=%lf, clm_i=%lf\n",n,ta,clm_r[n-1],clm_i[n-1]);

   if (n==numt)
   {
      coords[1]=ta;
      if (!(ng_r=galloc(1,COORD_UNIFORM,&numt,0.0,coords,0,clm_r))) return 0;
      if (!(ng_i=galloc(1,COORD_UNIFORM,&numt,0.0,coords,0,clm_i))) return 0;
      free(clm_r); clm_r=0;
      free(clm_i); clm_i=0;
      sprintf(buf,"%s_l_%i_m_%i_r",tag,l,m);
      add_grid_str(buf,ng_r);
      sprintf(buf,"%s_l_%i_m_%i_i",tag,l,m);
      add_grid_str(buf,ng_i);

      if (test)
      {
         if (!(ng_r=galloc(g_r->dim,g_r->coord_type,g_r->shape,0,g_r->coords,0,0))) return 0;
         if (!(ng_i=galloc(g_r->dim,g_r->coord_type,g_r->shape,0,g_r->coords,0,0))) return 0;
         fill_cylm_(ng_r->data,ng_i->data,&l,&m,&ng_r->shape[0],&ng_r->shape[1]);
         sprintf(buf,"cylm_test_l_%i_m_%i_r",l,m);
         add_grid_str(buf,ng_r);
         sprintf(buf,"cylm_test_l_%i_m_%i_i",l,m);
         add_grid_str(buf,ng_i);
      }

   }

   // saved 'filtered' version of the wave form
   if (range) 
   {
      sprintf(buf,"%s_filtered",it_a->r->name);
      add_grid_str(buf,f_ng_r);
      sprintf(buf,"%s_filtered",it_b->r->name);
      add_grid_str(buf,f_ng_i);
      gfree(tmp_r); gfree(tmp_i); 
   }

   return ng;
}

//=============================================================================
// computes the overlap between Y1=Y1r(t) + i* Y1i(t) and Y2=(Y2r(t-dt) + i* Y2i(t-dt)),
//
// n is the number of points about t=0 to calculate
//=============================================================================
void sf_overlap(char *args)
{
   int n=10;
   reg *y1r_r=0,*y2r_r=0,*y1i_r=0,*y2i_r=0;
   s_iter y1r_i,y2r_i,y1i_i,y2i_i;
   grid *y1r_g=0,*y2r_g=0,*y1i_g=0,*y2i_g=0;
   double *y1r=0,*y2r=0,*y1i=0,*y2i=0;
   double n1,n2,overlap_r,overlap_i,dt,overlap,delta;
   int i,j,j2,nt;

   if (args && (strlen(args)>0)) sscanf(args,"%i",&n);

   printf("sf_overlap: n=%i\n",n);

   if (!((y1r_r=find_reg("Y1r",1,1)) &&
         (y2r_r=find_reg("Y2r",1,1)) &&
         (y1i_r=find_reg("Y1i",1,1)) &&
         (y2i_r=find_reg("Y2i",1,1)) ))
   {
       printf("sf_overlap: error ... one or more of Y1r,Y2r,Y1i,Y2i do not exist\n");
       goto fin;
   }

   if (!(grid_str_eq(y1r_r,y2r_r) && grid_str_eq(y1r_r,y1i_r) && grid_str_eq(y1r_r,y2i_r)))
   {
       printf("sf_overlap: error ... grid structures of Y1r,Y2r,Y1i,Y2i are not identical\n");
       goto fin;
   }

   y1r_g=init_s_iter(&y1r_i,y1r_r,0);
   y2r_g=init_s_iter(&y2r_i,y2r_r,0);
   y1i_g=init_s_iter(&y1i_i,y1i_r,0);
   y2i_g=init_s_iter(&y2i_i,y2i_r,0);

   while(y1r_g && y2r_g && y1i_g && y2i_g)
   {
      if (y1r_g->dim!=1 && y1r_g->coord_type!=COORD_UNIFORM)
      {
          printf("sf_overlap: error ... expecting 1 dimensional uniform grids\n");
          goto fin;
      }
      y1r=y1r_g->data;
      y2r=y2r_g->data;
      y1i=y1i_g->data;
      y2i=y2i_g->data;

      nt=y1r_g->shape[0];

      for(i=-n; i<n; i++)
      {
         n1=n2=0;
         for (j=0; j<nt; j++)
         {
            j2=j+i;
            if (j2>=0 && j2<nt)
            {
               n1+=(y1r[j]*y1r[j]+y1i[j]*y1i[j]);
               n2+=(y2r[j2]*y2r[j2]+y2i[j2]*y2i[j2]);
            }
         }
         n1=sqrt(n1);
         n2=sqrt(n2);
         overlap_r=overlap_i=0;
         for (j=0; j<nt; j++)
         {
            j2=j+i;
            if (j2>=0 && j2<nt)
            {
               overlap_r+=(y1r[j]*y2r[j2] + y1i[j]*y2i[j2]);
               overlap_i+=(y1i[j]*y2r[j2] - y1r[j]*y2i[j2]);
            }
         }
         overlap_r/=(n1*n2);
         overlap_i/=(n1*n2);

         overlap=sqrt(overlap_r*overlap_r+overlap_i*overlap_i);
         dt=-i*(y2r_g->coords[1]-y2r_g->coords[0])/(y2r_g->shape[0]-1);
         if (overlap_r==0) 
         {
            if (overlap_i>0) delta=M_PI; else delta=-M_PI;
         }
         else
         {
            delta=atan(overlap_i/overlap_r);
            if (overlap_r<0 && overlap_i>0) delta+=M_PI;
            else if (overlap_r<0 && overlap_i<0) delta-=M_PI;
         }
         delta=2*M_PI-delta;
         if (delta>2*M_PI) delta-=2*M_PI;

         printf("dt (Y2) =%lf, overlap(r,i)=%7.4lf (%7.4lf,%7.4lf), delta phi (Y2)=%lf\n",dt,overlap,overlap_r,overlap_i,delta);
      }

      y1r_g=next_g(&y1r_i);
      y2r_g=next_g(&y2r_i);
      y1i_g=next_g(&y1i_i);
      y2i_g=next_g(&y2i_i);
   }

fin:
   if (y1r_r) FREE_REG(y1r_r);
   if (y2r_r) FREE_REG(y2r_r);
   if (y1i_r) FREE_REG(y1i_r);
   if (y2i_r) FREE_REG(y2i_r);
}

//=============================================================================
// computes undivided dV/d(phi) (i.e., just dV)
// 
// i.e., for each bin in phi, d(phi)=[phi_max-phi_min]/[n-1], computes
// the integrated spatial volume occupied by that portion of phi
// ignores phi<phi_min, if phi>phi_max, goes to phi[n-1];
//
// arguments: phi_min,phi_max,n,tag
//
// IF phi_max==0, then at each time uses current maximum of phi for phi_max
//
// needs registers:
// 
// "tag"_mag_phi
// "tag"_m_gb_det
// "tag"_alpha
//
//  produces:
//
//  "tag"_dV_dp
//  "tag"_V_p
//
//   V_p is the integral of dV_dp, starting from phi_max and integrating inwards
//
// NOTE: CURRENTLY ONLY WORKS WITH UNIFORM UNIGRID FUNCTIONS!
//=============================================================================
void sf_urbs_dV_dp(char *args)
{
   reg *phi_r=0,*gb_r=0,*alpha_r=0;
   s_iter phi_i,gb_i,alpha_i;
   grid *phi_g=0,*gb_g=0,*alpha_g=0,*dV_dp_g=0,*V_p_g=0;
   double *phi=0,*gb=0,*alpha=0,*dV_dp=0,*V_p=0,phi_min,phi_max0,pt,t,x0,y0,z0,dx,dy,dz,dphi;
   double phi0,rh0,dv0,Jx,Jy,Jz,alpha0,gb0,phi_max,x,y,z,r0;
   int i,j,k,nt,n,np,Nx,Ny,Nz,phi_ind,ind,do_test=0;
   char tag[256],phi_n[280],gb_n[280],alpha_n[280],dV_dp_n[280],V_p_n[280];
   double bbox[2];

   if (!args)
   {
      printf("urbs_dV_dp: need arguments\n");
      return;
   }

   sscanf(args,"%lf,%lf,%i,%s",&phi_min,&phi_max0,&n,tag);

   printf("urbs_dV_dp: phi_min=%lf,phi_max0=%lf,n=%i,tag=%s\n",phi_min,phi_max0,n,tag);

   if (n<2)
   {
      printf("urbs_dV_dp: need at least n=2\n");
      return;
   }

   sprintf(phi_n,"%s_mag_phi",tag);
   sprintf(gb_n,"%s_m_gb_det",tag);
   sprintf(alpha_n,"%s_alpha",tag);
   sprintf(dV_dp_n,"%s_dV_dp",tag);
   sprintf(V_p_n,"%s_V_p",tag);

   if (!((phi_r=find_reg(phi_n,1,1)) &&
         (gb_r=find_reg(gb_n,1,1)) &&
         (alpha_r=find_reg(alpha_n,1,1)) ))
   {
       printf("urbs_dV_dp:: error ... one of %s,%s or %s does not exist\n",phi_n,gb_n,alpha_n);
       goto fin;
   }

   if (!(grid_str_eq(phi_r,gb_r) && grid_str_eq(gb_r,alpha_r)))
   {
       printf("urbs_dV_dp: error ... grid structures of phi_n,gb_n,alpha_n are not identical\n");
       goto fin;
   }

   phi_g=init_s_iter(&phi_i,phi_r,0);
   alpha_g=init_s_iter(&alpha_i,alpha_r,0);
   gb_g=init_s_iter(&gb_i,gb_r,0);

   t=phi_g->time-1;

   while(phi_g && alpha_g && gb_g)
   {
      pt=t;
      t=phi_g->time;
      if (pt==t) printf("urbs_drh_dp: WARNING ... assumes serial/unigrid input!\n");

      if (phi_g->coord_type!=COORD_UNIFORM || phi_g->dim==1)
      {
          printf("urbs_dV_dp: error ... require uniform grids, and dim>1\n");
          goto fin;
      }
      alpha=alpha_g->data;
      phi=phi_g->data;
      gb=gb_g->data;

      if (!(V_p=(double *)malloc(sizeof(double)*n)) ||
          !(dV_dp=(double *)malloc(sizeof(double)*n)))
      { 
         printf("urbs_dV_dp: error ... out of memory\n"); 
         goto fin;
      }
      for(i=0; i<n; i++) dV_dp[i]=V_p[i]=0;

      Nx=phi_g->shape[0]; x0=phi_g->coords[0]; dx=(phi_g->coords[1]-x0)/(Nx-1);
      if (phi_g->dim>1) { Ny=phi_g->shape[1]; y0=phi_g->coords[2]; dy=(phi_g->coords[3]-y0)/(Ny-1); } else { Ny=1; y0=0; dy=dx; }
      if (phi_g->dim>2) { Nz=phi_g->shape[2]; z0=phi_g->coords[4]; dz=(phi_g->coords[5]-z0)/(Nz-1); } else { Nz=1; z0=0; dz=dx; }

      if (phi_max0>0) 
         phi_max=phi_max0; 
      else
      {
          phi_max=0; for(i=0;i<Nx*Ny*Nz;i++) if (phi[i]>phi_max) phi_max=phi[i];
      }

      dphi=(phi_max-phi_min)/(n-1);
      bbox[0]=phi_min;
      bbox[1]=phi_max;

      for (k=0, z=z0; k<Nz; k++, z+=dz)
      {
         Jz=M_PI/2/pow(cos(M_PI*z/2),2);
         for (j=0, y=y0; j<Ny; j++, y+=dy)
         {
            Jy=M_PI/2/pow(cos(M_PI*y/2),2);
            for (i=0, x=x0; i<Nx; i++, x+=dx)
            {
               Jx=M_PI/2/pow(cos(M_PI*x/2),2);
               ind=i+j*Nx+k*Nx*Ny;

               if (!strcmp(tag,"test") && do_test)
               {
                  // off-centered sphere ... choosing gb such that volume should be pi r^4
                  r0=sqrt(pow(tan(M_PI*x/2)-0.25,2)+pow(tan(M_PI*y/2),2)+pow(tan(M_PI*z/2),2));
                  phi[ind]=1-r0;
                  alpha[ind]=3;
                  gb[ind]=9*r0*r0;
               }

               phi0=phi[ind];
               alpha0=alpha[ind];
               gb0=gb[ind];
               if (alpha0>0 && gb0>0 && phi0>=phi_min)
               {
                  if (phi_g->dim==2)
                     dv0=2*M_PI*(tan(M_PI*y/2))*dx*dy*Jx*Jy; 
                  else
                     dv0=dx*dy*dz*Jx*Jy*Jz;

                  rh0=dv0*sqrt(gb0)/alpha0;
                  if (phi0>phi_max) phi_ind=n-1;
                  else phi_ind=(phi0-phi_min)/dphi; 

                  // printf("i,j,k,phi_ind,rh0=%i,%i,%i,%i,%lf\n",i,j,k,phi_ind,rh0);

                  dV_dp[phi_ind]+=rh0;
               }
            }
         }
      }

      for (i=n-2,V_p[n-1]=dV_dp[n-1]; i>=0; i--) V_p[i]=dV_dp[i]+V_p[i+1];

      if (!(dV_dp_g=galloc(1,COORD_UNIFORM,&n,t,bbox,0,dV_dp))) goto fin;
      free(dV_dp); dV_dp=0;
      add_grid_str(dV_dp_n,dV_dp_g);

      if (!(V_p_g=galloc(1,COORD_UNIFORM,&n,t,bbox,0,V_p))) goto fin;
      free(V_p); V_p=0;
      add_grid_str(V_p_n,V_p_g);

      phi_g=next_g(&phi_i);
      alpha_g=next_g(&alpha_i);
      gb_g=next_g(&gb_i);
   }

fin:
   if (phi_r) FREE_REG(phi_r);
   if (alpha_r) FREE_REG(alpha_r);
   if (gb_r) FREE_REG(gb_r);
}

//=============================================================================
// gh3d specific. partial embedding diagram...
// 
// given an axisymmetric slice in code coordinates, attaches a CC coordinate
// system where distance along the axis is proper distance (starting from
// x=0), and then along each x=constant line, distance in y is proper distance.
// i.e., sort of a "projected" embedding diagram
// 
// arguments: tag
//
// needs registers:
// 
// "tag"_mag_phi
// "tag"_gb_xx
// "tag"_gb_yy
//
//  produces:
//
//  "tag"_mag_phi_embed
//
// NOTE: CURRENTLY ONLY WORKS WITH UNIFORM UNIGRID FUNCTIONS!
//=============================================================================
void sf_p_embed(char *args)
{
   reg *phi_r=0,*gb_xx_r=0,*gb_yy_r=0;
   s_iter phi_i,gb_xx_i,gb_yy_i;
   grid *phi_g=0,*gb_xx_g=0,*gb_yy_g=0,*ng=0;
   double *phi=0,*gb_xx=0,*gb_yy=0,*phi_embed=0,*x_cc,*y_cc,pt,t,x0,y0,dx,dy;
   double Jx,Jy,x,y,r0;
   int i,j,k,nt,Nx,Ny,ind,do_test=0,ind_m1;
   char tag[256],phi_n[280],gb_xx_n[280],gb_yy_n[280],phi_embed_n[280];

   if (!args)
   {
      printf("sf_p_embed: need arguments\n");
      return;
   }

   sscanf(args,"%s",tag);

   printf("sf_p_embed: tag=%s\n",tag);

   sprintf(phi_n,"%s_mag_phi",tag);
   sprintf(gb_xx_n,"%s_gb_xx",tag);
   sprintf(gb_yy_n,"%s_gb_yy",tag);
   sprintf(phi_embed_n,"%s_mag_phi_embed",tag);

   if (!((phi_r=find_reg(phi_n,1,1)) &&
         (gb_xx_r=find_reg(gb_xx_n,1,1)) &&
         (gb_yy_r=find_reg(gb_yy_n,1,1)) ))
   {
       printf("sf_p_embed: error ... one of %s,%s or %s does not exist\n",phi_n,gb_xx_n,gb_yy_n);
       goto fin;
   }

   if (!(grid_str_eq(phi_r,gb_xx_r) && grid_str_eq(gb_xx_r,gb_yy_r)))
   {
       printf("sf_p_embed: error ... grid structures of phi,gb_xx,gb_yy are not identical\n");
       goto fin;
   }

   phi_g=init_s_iter(&phi_i,phi_r,0);
   gb_yy_g=init_s_iter(&gb_yy_i,gb_yy_r,0);
   gb_xx_g=init_s_iter(&gb_xx_i,gb_xx_r,0);

   t=phi_g->time-1;

   while(phi_g && gb_yy_g && gb_xx_g)
   {
      pt=t;
      t=phi_g->time;
      if (pt==t) printf("sf_p_embed: WARNING ... assumes serial/unigrid input!\n");

      if (phi_g->coord_type!=COORD_UNIFORM || phi_g->dim!=2)
      {
          printf("sf_p_embed: error ... require uniform grids, and dim==2\n");
          goto fin;
      }

      gb_yy=gb_yy_g->data;
      gb_xx=gb_xx_g->data;

      if (!(ng=galloc(phi_g->dim,COORD_SEMI_UNIFORM,phi_g->shape,phi_g->time,phi_g->coords,phi_g->ccoords,phi_g->data)))
      {
         printf("sf_p_embed: out of memory\n");
         goto fin;
      }

      Nx=phi_g->shape[0]; x0=phi_g->coords[0]; dx=(phi_g->coords[1]-x0)/(Nx-1);
      Ny=phi_g->shape[1]; y0=phi_g->coords[2]; dy=(phi_g->coords[3]-y0)/(Ny-1); 
      phi=ng->data;

      x_cc=ng->ccoords;
      y_cc=&ng->ccoords[Nx*Ny];

      x_cc[0]=tan(M_PI*x0/2); // should start with x0=y0=0, but if not assume Minkowski for i.c.
      for (i=0, x=x0; i<Nx; i++, x+=dx)
      {
         Jx=M_PI/2/pow(cos(M_PI*(x+dx/2)/2),2);  //mid-point ... integrating by trapezoidal rule
         y_cc[i]=tan(M_PI*y0/2);
         if (i>0) x_cc[i]=x_cc[i-1]+Jx*sqrt((gb_xx[i]+gb_xx[i-1])/2)*dx;

         for (j=1, y=y0+dy; j<Ny; j++, y+=dy)
         {
            Jy=M_PI/2/pow(cos(M_PI*(y+dy/2)/2),2);
            ind=i+j*Nx;
            ind_m1=i+(j-1)*Nx;

            if (!strcmp(tag,"test") && do_test)
            {
               // off-centered sphere ... choosing gb such that volume should be pi r^4
               r0=sqrt(pow(tan(M_PI*x/2)-0.25,2)+pow(tan(M_PI*y/2),2));
               phi[ind]=exp(-r0); if (j==1) phi[ind_m1]=phi[ind];
               if (fabs(tan(M_PI*x/2)-0.25)<0.5) gb_yy[ind]=1/(1-pow(tan(M_PI*x/2)-0.25,2)); else gb_yy[ind]=1;
               // gb_yy[ind]=1;
               // gb_xx[ind]=1;
            }

            x_cc[ind]=x_cc[ind_m1];
            y_cc[ind]=y_cc[ind_m1]+Jy*sqrt((gb_yy[ind]+gb_yy[ind_m1])/2)*dy;
         }
      }

      add_grid_str(phi_embed_n,ng);

      phi_g=next_g(&phi_i);
      gb_yy_g=next_g(&gb_yy_i);
      gb_xx_g=next_g(&gb_xx_i);
   }

fin:
   if (phi_r) FREE_REG(phi_r);
   if (gb_yy_r) FREE_REG(gb_yy_r);
   if (gb_xx_r) FREE_REG(gb_xx_r);
}

//=============================================================================
// takes psi4_eval_r, evaluates it at a given point, extrapolating if
// near axis, and double integrates in time to give h
//=============================================================================
#define PSI4_TO_H_EPS 0.5
#define PSI4_TO_H_KM 1
#define PSI4_TO_H_NUM 100
#define MAX_EXTREMES 1000
#define PSI4_TO_H_LIN 0
grid *gf_psi4_to_h(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*nxt_g,*ng,*psi40_g,*h0_g;
   static int ev_i,ev_j,numt,n,Nphi,Ntheta,extrap,do_eps,flip,shift;
   static double *psi40,*h0,pt,dt,dtc,*tc,m0,r0;
   static double dtheta,dphi;
   double ta,dp1,dp2,dp0,pdt,a0,b0;
   int i,i0,j;
   char buf[256];
   static char tag[256];
   int extremes[MAX_EXTREMES],num_extremes;
   double h0_ext[MAX_EXTREMES];
   int test=0,is,pass,ie;
   double eps0=PSI4_TO_H_EPS,omega;
   int e_k0=PSI4_TO_H_KM;
   int e_num=PSI4_TO_H_NUM;

   if (g->dim!=2) {printf("gf_psi4_to_h: dim !=2\n"); return 0;}
   if (g->coord_type!=COORD_UNIFORM) {printf("gf_psi4_to_h: expects uniform coordinates\n"); return 0;}

   if (first_call) 
   { 
      sscanf(args,"%i,%i,%i,%lf,%lf,%i,%i,%i,%s",&ev_i,&ev_j,&extrap,&m0,&r0,&do_eps,&flip,&shift,tag);
      Ntheta=g->shape[0];
      Nphi=g->shape[1]; 
      dphi=2*M_PI/(Nphi-1);
      dtheta=M_PI/(Ntheta-1);
      psi40=0; h0=0; 
      save_s_iter(it_a);
      nxt_g=g;
      pt=it_a->ts->time;
      numt=1;n=0;
      while(nxt_g)
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            dt=it_a->ts->time-pt;
            pt=it_a->ts->time;
         } 
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);
      pt=it_a->ts->time;
      if (!(psi40=(double *)malloc(sizeof(double)*numt)) ||
          !(h0=(double *)malloc(sizeof(double)*numt)) || 
          !(tc=(double *)malloc(sizeof(double)*numt)))
      { 
         printf("gf_psi4_to_h: error ... out of memory\n"); 
         if (psi40) free(psi40); 
         if (h0) free(h0); 
         if (tc) free(tc); 
         psi40=0; h0=0; tc=0;
         return 0; 
      }
      for (i=0; i<numt; i++) psi40[i]=h0[i]=tc[i]=0;
      printf("gf_psi4_to_h: ev_i=%i (theta=%lf), ev_j=%i (phi=%lf), m0=%lf, r0=%lf, do_eps=%i, flip=%i, shift=%i, extrap=%i (so many points away "
      "from axis will be extrapolated)\n\n",ev_i,(ev_i-1)*dtheta,ev_j,(ev_j-1)*dphi,m0,r0,do_eps,flip,shift,extrap);
      if (shift==1) printf("shifting using a linear function\n");
      else if (shift) printf("shifting using a piece-wise linear function\n");
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;

   if (!psi40) return 0;
   n++;
   ta=it_a->ts->time;
   tc[n-1]=ta;
   if (extrap && g->dim>1)
   {
      for (j=0; j<Nphi; j++)
      {
         i0=extrap;
         for (i=0; i<i0; i++)
         {
            dp1=ng->data[i0+j*Ntheta];
            dp2=ng->data[i0+extrap+j*Ntheta];
            dp0=(4*dp1-dp2)/3;
            ng->data[i+j*Ntheta]=dp0*(i-i0)*(i-2*i0)/(2.0e0*i0*i0) + dp1*(i)*(i-2*i0)/(-1.0e0*i0*i0) + dp2*(i)*(i-i0)/(2.0e0*i0*i0);

            dp1=ng->data[Ntheta-1-i0+j*Ntheta];
            dp2=ng->data[Ntheta-1-i0-extrap+j*Ntheta];
            dp0=(4*dp1-dp2)/3;
            ng->data[Ntheta-1-i+j*Ntheta]=dp0*(i-i0)*(i-2*i0)/(2.0e0*i0*i0) + dp1*(i)*(i-2*i0)/(-1.0e0*i0*i0) + dp2*(i)*(i-i0)/(2.0e0*i0*i0);
         }
      }
   }
   psi40[n-1]=ng->data[ev_i-1+(ev_j-1)*Ntheta];

   pt=ta;

   if (n==numt)
   {
      if (m0>0 && r0>0)
      {
         printf("scaling t by 1/m0 (%lf), and psi4 by r0*m0 (%lf)\n",m0,r0*m0);
         for (i=0; i<n; i++) { tc[i]/=m0; psi40[i]*=(r0*m0); }
      }

      if (do_eps) diss_ko_1d_(psi40,&eps0,&e_k0,&e_num,&numt);

      if (!test) { if (!(psi40_g=galloc(1,COORD_CURVILINEAR,&numt,0.0,tc,0,psi40))) return 0; }

      // integral = (-t^2 +t^3/3 - t^4/4); 
      if (test) 
      {
         for (i=numt-2; i>=0; i--) tc[i]=(tc[i]-tc[0])/tc[numt-1]; tc[numt-1]=1; 
         for (i=0; i<numt; i++) psi40[i]=(-2*tc[i]+tc[i]*tc[i]-tc[i]*tc[i]*tc[i])/(tc[numt-1]*tc[numt-1]*tc[numt-1]);
      }

      for (pass=1; pass<=2; pass++)
      {
         // seems like we get less 'drift' error if we integrate from tmin to tmax 
         // for one integral, then switch directions for the other ... if (flip) do this
         if (flip)
         {
            for (i=0;i<numt;i++) h0[i]=psi40[numt-i-1];
            for (i=0;i<numt;i++) psi40[i]=h0[i];
         }

         if (test) pass=3;

         // using some-or-other order integration from numerical recipes, but kick-starting
         // with lower order integration. If dt, changes, we start again
         is=0;
         h0[0]=0;
         while(is<(numt-1))
         {
            i0=is;
            dt=tc[is+1]-tc[is];
            pdt=dt;
            while(i0<(numt-1) && fabs(dt-pdt)<fabs(dt/1000))
            {
               i0++;
               dtc=tc[i0]-tc[i0-1];
               if ((i0-is)==1) h0[i0]=h0[is]+0.5e0*(psi40[i0]+psi40[i0-1])*dt;
               else if ((i0-is)==2) h0[i0]=h0[is]+(psi40[i0]+4*psi40[i0-1]+psi40[i0-2])*dt/3;
               else if ((i0-is)==3) h0[i0]=h0[is]+(5*psi40[i0]+13*psi40[i0-1]+13*psi40[i0-2]+5*psi40[i0-3])*dt/12;
               else h0[i0]=h0[i0-1]+(-psi40[i0-2]+8*psi40[i0-1]+5*psi40[i0])*dt/12;
               pdt=dt;
               if (i0<numt) dt=tc[i0+1]-tc[i0];
            }
            is=i0;
         }

         // integral = (3t -t^2 +t^3/3 - t^4/4)/(t_max^3); 
         if (test) for (i=0; i<numt; i++) psi40[i]=tc[i]*(-tc[i]+tc[i]*tc[i]/3-tc[i]*tc[i]*tc[i]/4)/(tc[numt-1]*tc[numt-1]*tc[numt-1]);

         if (test) if (!(psi40_g=galloc(1,COORD_CURVILINEAR,&numt,0.0,tc,0,psi40))) return 0;

         for (i=0; i<numt; i++) psi40[i]=h0[i];
      }

      if (flip)
      {
         // integrating backwords changed the sign of h0
         for (i=0; i<numt; i++) h0[i]=-h0[i];
      }
      if (shift)
      {
         // we get an arbitrary integration 'line', a0*t + b0 ... choose
         // initial conditions so that h0[0]=h0[numt]=0
         a0=(h0[numt-1]-h0[0])/(tc[numt-1]-tc[0]);
         b0=(h0[0]*tc[numt-1]-h0[numt-1]*tc[0])/(tc[numt-1]-tc[0]);
         for (i=0; i<numt; i++) h0[i]=(h0[i]-a0*tc[i]-b0); 
      }
      if (shift && shift!=1)
      {
         // this is maquillage, but shift along each cycle of the wave by a 
         // line a0*t + b0 so that h0[(t_min+t_max)/2]=0
         // checking adjacent and +-2 to avoid possible "noise"

         num_extremes=0; 
         i=3;
         while(i<(numt-3))
         {
            if ( fabs(h0[i])>fabs(h0[i-1]) && fabs(h0[i])>fabs(h0[i+1]) &&
                 fabs(h0[i])>fabs(h0[i-2]) && fabs(h0[i])>fabs(h0[i+2]) )
            {
               extremes[num_extremes++]=i;
               
               i+=2;
               if (num_extremes==MAX_EXTREMES) { printf("ERROR ... MAX_EXTREMES reached ... setting to zero\n"); num_extremes=0; }
            }
            i++;
         }
         printf("Extrema of h:\n");
         for (i=0; i<num_extremes; i++)
         {
            omega=0;
            if (i>0 && i<(num_extremes-1)) omega=2*M_PI/(tc[extremes[i+1]]-tc[extremes[i-1]]);
            printf("i=%i, t=%lf, omega=%lf\n",i,tc[extremes[i]],omega);
         }

         for (j=0; j<num_extremes; j++)
         {
            if (j==0) { is=0; h0_ext[j]=0; } else is=(extremes[j]+extremes[j-1])/2;  //HERE
            if (j==(num_extremes-1)) ie=numt-1; else ie=(extremes[j+1]+extremes[j])/2;
            

            a0=(h0[ie]-h0[is])/(tc[ie]-tc[is]);
            b0=(h0[is]*tc[ie]-h0[ie]*tc[is])/(tc[ie]-tc[is]);
            for (i=is; i<ie; i++) h0[i]=(h0[i]-a0*tc[i]-b0);
         }
      }

      if (!(h0_g=galloc(1,COORD_CURVILINEAR,&numt,0.0,tc,0,h0))) return 0;
      free(psi40); psi40=0;
      free(h0); h0=0;
      free(tc); tc=0;
      sprintf(buf,"%s_psi40",tag);
      add_grid_str(buf,psi40_g);
      sprintf(buf,"%s_h0",tag);
      add_grid_str(buf,h0_g);
   }

   return ng;
}

//=============================================================================
// gh3d specific ... the spin weight -2 complex ylm, l=2..6, 2<=|m|<=l 
// spherical harmonic coefficient of a UNIGRID function of theta,phi.
//
// it_a is real component, it_b is imaginary ... creates 1d in time grid
// functions "tag_m2_lm_r" functions
//
// => produces a coefficient function in the same format as cylm
//
// => produces a 'filtered' version of the function using functions in the
// specified range.
//
// => produces a function tag_phase which gives the phase of the
// waveform as measured by looking at the maximum of the real part of
// the filtered psi4. 
//
// NOTE: STUPIDLY THINGS ARE BEING HARD-CODED FOR L=6
//
// even more stupidly ... FOUR different mappings!!
// 
// map_lm_q: (l,m) to linear, packed format for storing pointers to the
//                 "numt" components clm_r,clm_i  ... q_max = (range-1)*range=30
// map_lm_cind: maps (l,m,range) to the index structure used by the spherical
//              harmonic ylm routines
// map_lm_cind6: maps (l,m,range=6) to the index structure used by cm2yl6m_coeff
// map_lm_spec: maps(l,m) to y coord in spectral image array
//=============================================================================
// spectral map:
// |m|=0..6, l>=2, so 13 sets + 'gaps' of width 2 between... 2*sum(1..5) + 5*3 +  (2*13)
// plus, doubling l=6,|m|=6 for visualization purposes
#define SIZE_SPEC 71

int map_lm_spec(int l, int m, int range)
{
   int i0,m0,l0,r0,i,offset;

   i0=0;
   m0=abs(m);
   if (m0<2) l0=l-2; else l0=l-m0;
   r0=range-1;
   i=0;

   while(m0) { i0+=r0; i0+=2; m0--; if (i>1) r0--; i++;}
   i0+=l0;

   offset=((SIZE_SPEC-1)/2-(range-2)/2); // sets m=0,l=4 at center of array

   if (m>=0) i0+=offset; else i0=(offset-(i0-(range-1))-1); // over-counted extra '0' for - m's

   if (i0<0 || i0>=SIZE_SPEC) { printf ("map_lm_spec: error ... i0 out of range: l=%i,m=%i,i0=%i\n",l,m,i0); i0=0; }

   return i0;
}

#define SIZE_Q 45
int map_lm_q(int l, int m)
{
   int s,ret;

   if (l<2 || abs(m)>l) {printf("map_lm_q:ERROR ... OUT OF RANGE . l=%i, m=%i\n",l,m); return -1; }
   s=(l-2)*(l-1)+3*(l-2);
   ret=(s+(m+l));

   return ret;
}

void map_lm_cind(int l, int m, int range, int *ind_r, int *ind_i)
{
   *ind_r=(l+range)+(m+range)*(2*range+1);
   *ind_i=(range-l)+(m+range)*(2*range+1);
}

int map_lm_cind6(int l, int m)
{
   return l-2+(m+l)*5; // range-1 = 5 elements in first fortran row
}

grid *gf_cm2yl6m(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g_r=it_a->g,*ng_r=0,*ng_m,*nxt_g_r,*ng;
   grid *g_i=it_b->g,*ng_i=0,*f_ng_r=0,*f_ng_i=0,*ng_phase,*ng_spec_r,*ng_spec_i;
   static int l,m,numt,n,range,filter_range,rshape[2],half_cycles,Nphi,Ntheta,q_max,test_l,test_m,test=0;
   static int spec_shape[2];
   static double *clm_r[SIZE_Q],*clm_i[SIZE_Q],*r_clm_r,*r_clm_i,pt,dt,rcoords[4],coords[2],prev_w0,w0,*phase,*tc;
   static double *spec_r,*spec_i,spec_coords[4],scale_m,scale_r,sc_t,sc_e;
   int i,j,ind,ind_r,ind_i,ltrace=1,ms,me,ls,le,m0,l0,jp1,jm1,j_max,q,qq,ind2,ii;
   double ta,tb,a1r,a1i,anr,ani,cr[5*13],ci[13*5],psi4r_max,*psi4r,psi4r0,psi4r_j,psi4r_jp1,psi4r_jm1,phi,dphi,frac,q0[13*5],x0;
   double dtheta,tmp_r,tmp_i,sp,cp,et;
   static char tag[256];
   char buf[256];

   if (g_r->dim!=2) {printf("gf_cm2yl6m: dim !=2\n"); return 0;}
   if (g_r->coord_type!=COORD_UNIFORM) {printf("gf_cm2yl6m: expects uniform coordinates\n"); return 0;}

   if (first_call) 
   { 
      prev_w0=-100000;
      half_cycles=0;
      w0=0;
      range=2; 
      l=2;
      // MUST BE 6 AT THIS STAGE!!!!!
      range=6;
      scale_m=scale_r=0;
      q_max=map_lm_q(range,range);
      if (q_max!=(SIZE_Q-1)) {printf("gf_cm2yl6m: error in SIZE_Q\n"); return 0;}
      filter_range=6;
      test_l=test_m=test=0;
      spec_r=spec_i=0;
      sprintf(tag,"cm2yl6m_");
      sscanf(args,"%i,%lf,%lf,%i,%i,%s",&filter_range,&scale_m,&scale_r,&test_l,&test_m,tag);
      if (filter_range<0)
      if (filter_range<2 || filter_range>6)  {printf("gf_cm2yl6m: invalid filter range\n"); return 0;}
      if (!(test_l<2 || test_l>6 || abs(test_m)>6))
      {
         test=1;
         printf("producing test function in filtered output with l=%i, m=%i\n",test_l,test_m);
      }
      if (range>1)
      {
         rshape[0]=rshape[1]=2*range+1;
         rcoords[0]=rcoords[2]=-range;
         rcoords[1]=rcoords[3]=range;
      }
      sc_t=sc_e=1;
      if (scale_m>0 && scale_r>0)
      {
         printf("scaling time and values using m=%lf, r=%lf\n",scale_m,scale_r);
         sc_t=1/scale_m;
         sc_e=scale_r*scale_m;
      }
      for (q=0; q<=q_max; q++) clm_r[q]=0; clm_i[q]=0;
      phase=0; tc=0;
      save_s_iter(it_a);
      nxt_g_r=g_r;
      pt=it_a->ts->time*sc_t;
      numt=0;n=0;
      coords[0]=pt;
      printf("numt=%i, nxt_g_r%i\n",n,numt);
      while(nxt_g_r)
      {
         if (!(fuzz_eq(it_a->ts->time*sc_t,pt,1e-12)) && it_a->selected!=GIV_OFF) 
         {
            numt++;
            dt=it_a->ts->time*sc_t-pt;
            pt=it_a->ts->time*sc_t;
         } 
         nxt_g_r=next_g(it_a);
      }
      restore_s_iter(it_a);
      et=pt;
      pt=coords[0];
      if (!(tc=(double *)malloc(sizeof(double)*numt)) || 
          !(phase=(double *)malloc(sizeof(double)*numt)) ||
          !(spec_r=(double *)malloc(sizeof(double)*numt*SIZE_SPEC)) ||
          !(spec_i=(double *)malloc(sizeof(double)*numt*SIZE_SPEC)))
      { 
         printf("gf_cylm : error ... out of memory\n"); 
         if (phase) free(phase); 
         if (tc) free(tc); 
         if (spec_r) free(spec_r); 
         if (spec_i) free(spec_i); 
         phase=tc=spec_r=spec_i=0;
         return 0; 
      }
      for (q=0; q<=q_max; q++)
      if (!(clm_r[q]=(double *)malloc(sizeof(double)*numt)) ||
          !(clm_i[q]=(double *)malloc(sizeof(double)*numt)))
      { 
         printf("gf_cylm : error ... out of memory\n"); 
         for (qq=0; qq<=q_max; qq++)
         {
            if (clm_r[qq]) free(clm_r[qq]); 
            if (clm_i[qq]) free(clm_i[qq]); 
            clm_r[qq]=clm_i[qq]=0;
         }
         if (phase) free(phase); 
         if (tc) free(tc); 
         if (spec_r) free(spec_r); 
         if (spec_i) free(spec_i); 
         phase=tc=spec_r=spec_i=0;
         return 0; 
      }
      for (i=0; i<numt; i++)
      {
         for (q=0; q<=q_max; q++) clm_r[q][i]=clm_i[q][i]=0;
         phase[i]=tc[i]=0;
         for (j=0; j<SIZE_SPEC; j++) spec_r[i+j*numt]=spec_i[i+j*numt]=0;
      }

      spec_shape[0]=numt;
      spec_shape[1]=SIZE_SPEC; 
      spec_coords[0]=it_a->ts->time*sc_t;
      spec_coords[1]=et;
      spec_coords[2]=-(et-it_a->ts->time*sc_t)/4; // just for a nice scale
      spec_coords[3]=(et-it_a->ts->time*sc_t)/4;

      printf("gf_cm2y2pm2: dt=%lf, numt=%i, range=%i, filter_range=%i, q_max=%i\n\n",dt,numt,range,filter_range,q_max);
   }


   if (!clm_r[0]) return 0;
   n++;
   ta=it_a->ts->time*sc_t;
   tb=it_b->ts->time*sc_t;
   tc[n-1]=ta;

   if (range)
   {
      if (!(ng=galloc(2,COORD_UNIFORM,rshape,ta,rcoords,0,0))) return 0;
      for (i=0; i<sizeof_data(ng); i++) ng->data[i]=0;
      ls=2,le=range;
      ms=-range; me=range;

      if (!(f_ng_r=galloc(g_r->dim,g_r->coord_type,g_r->shape,ta,g_r->coords,0,0))) return 0;
      if (!(f_ng_i=galloc(g_r->dim,g_r->coord_type,g_r->shape,ta,g_r->coords,0,0))) return 0;
      for (i=0; i<f_ng_r->shape[0]*f_ng_r->shape[1]; i++) f_ng_r->data[i]=f_ng_i->data[i]=0;
   }
   else
   {
      ms=me=m;
      ls=le=l;
      ng=0;
   }

   pt=ta;

   if (ta!=tb) { printf("gf_cylm : error ... time structures for real & imaginary components do not match\n"); return 0; }

   Ntheta=f_ng_r->shape[0];
   Nphi=f_ng_r->shape[1];
   dphi=2*M_PI/(Nphi-1);
   dtheta=M_PI/(Ntheta-1);

   cm2yl6m_coeff_(cr,ci,g_r->data,g_i->data,&Ntheta,&Nphi);
   for (l0=ls; l0<=le; l0++)
   {
      for (m0=ms; m0<=me; m0++)
      {
         if (abs(m0)<=l0)
         {
            ind2=map_lm_cind6(l0,m0);
            cr[ind2]*=sc_e;
            ci[ind2]*=sc_e;
            q=map_lm_q(l0,m0);
            clm_r[q][n-1]=cr[ind2]; clm_i[q][n-1]=ci[ind2]; 
            map_lm_cind(l0,m0,range,&ind_r,&ind_i);
            ng->data[ind_r]=cr[ind2];
            ng->data[ind_i]=ci[ind2];

            ind=map_lm_spec(l0,m0,range);
            ind=(n-1)+ind*numt;
            spec_r[ind]=cr[ind2]; 
            spec_i[ind]=ci[ind2]; 
            // double first and last rows so that masking doesn't get rid of 1-line row
            if (abs(m0)==range) 
            {
               spec_r[ind+numt*m0/abs(m0)]=cr[ind2];
               spec_i[ind+numt*m0/abs(m0)]=ci[ind2];
            }

            if ((!test && l0<=filter_range) || (test && l0==test_l && m0==test_m))
            {
               for (i=0; i<Ntheta; i++)
               {
                  x0=cos(i*dtheta);
                  eval_m2ylm_l6_(q0,&x0);
                    
                  for (j=0; j<Nphi; j++)
                  {
                     sp=sin(j*dphi*(m0));
                     cp=cos(j*dphi*(m0));

                     tmp_r=q0[ind2]*cp;
                     tmp_i=q0[ind2]*sp;

                     ind=i+j*Ntheta;

                     if (!test)
                     {
                        f_ng_r->data[ind]+=(cr[ind2]*tmp_r-ci[ind2]*tmp_i);
                        f_ng_i->data[ind]+=(ci[ind2]*tmp_r+cr[ind2]*tmp_i);
                     }
                     else
                     {
                        f_ng_r->data[ind]+=tmp_r;
                        f_ng_i->data[ind]+=tmp_i;
                     }
                  }
               }
            }
         }
      }
   }

   // find the phase angle of the waveform, searching on the orbital plane theta=0, 
   // and over phi=0..Pi.
   // half_cycles records the number of Pi jumps we've undergone,
   // to keep track of the net phase angle

   // on axis for the reconstructed wave is OK
   // i=(Ntheta-1)/2+1; 
   i=0;

   psi4r_max=0; j_max=0;
   psi4r=f_ng_r->data;
   for (j=0; j<((Nphi-1)/2+1); j++)
   {
      psi4r0=psi4r[i+j*Ntheta];
      if (psi4r_max<psi4r0)
      {
         psi4r_max=psi4r0;
         j_max=j;
      }
   }
   // interpolate to get a good enough approximation to be able to differentiate
   // (otherwise the function will be piece-wise constant)
   j=j_max;
   jp1=j+1; jm1=j-1; if (jm1==-1) jm1=Nphi-2;
   psi4r_j=psi4r[i+j*Ntheta];
   psi4r_jp1=psi4r[i+jp1*Ntheta];
   psi4r_jm1=psi4r[i+jm1*Ntheta];
   if ((psi4r_j-psi4r_jm1)>(psi4r_j-psi4r_jp1))
   {
      frac=(psi4r_jp1-psi4r_jm1)/(psi4r_j-psi4r_jm1);
      w0=j*dphi+frac/2*dphi;
   }
   else
   {
      frac=(psi4r_jm1-psi4r_jp1)/(psi4r_j-psi4r_jp1);
      w0=j*dphi-frac/2*dphi;
   }
   if ((frac<0) || (frac>1)) printf("  WARNING ... frac=%lf\n",frac);
   if (prev_w0>-10000)
   {
      if (fabs(prev_w0-w0)>(M_PI/2))
      {
         if (w0<(M_PI/2)) half_cycles++; else half_cycles--;
      }
   }

   prev_w0=w0;
   phase[n-1]=w0+half_cycles*M_PI;

   q=map_lm_q(2,2);
   if (ltrace) printf("  t(%i)=%lf, clm_r(2,2)=%lf, clm_i(2,2)=%lf, phase=%lf\n",n,ta,clm_r[q][n-1],clm_i[1][n-1],phase[n-1]);

   printf("n,numt=%i,%i\n",n,numt);

   if (n==numt)
   {
      coords[1]=ta;

      for (l0=ls; l0<=le; l0++)
      {
         for (m0=ms; m0<=me; m0++)
         {
            if (abs(m0)<=l0)
            {
               q=map_lm_q(l0,m0);

               if (clm_r[q]==0) {printf("error in q's ... l0=%i,m0=%i,q=%i\n",l0,m0,q); return 0;}

	       printf("l0=%i,m0=%i,filter_range=%i\n",l0,m0,filter_range);

               if (l0<=filter_range && (abs(m0)==0 || abs(m0)==6 || abs(m0)==2 || abs(m0)==4))
               {
                  if (!(ng_r=galloc(1,COORD_CURVILINEAR,&numt,0.0,tc,0,clm_r[q]))) return 0;
                  if (!(ng_i=galloc(1,COORD_CURVILINEAR,&numt,0.0,tc,0,clm_i[q]))) return 0;
                  if (!(ng_m=galloc(1,COORD_CURVILINEAR,&numt,0.0,tc,0,0))) return 0;
                  sprintf(buf,"%s-2_l_%i_m_%i_r",tag,l0,m0);
		  printf("about to add %s\n",buf);
                  add_grid_str(buf,ng_r);
                  sprintf(buf,"%s-2_l_%i_m_%i_i",tag,l0,m0);
		  printf("about to add %s\n",buf);
                  add_grid_str(buf,ng_i);
		  for (ii=0;ii<numt;ii++) ng_m->data[ii]=log10(sqrt(pow(ng_r->data[ii],2)+pow(ng_i->data[ii],2)));
                  sprintf(buf,"log_mag_%s-2_l_%i_m_%i",tag,l0,m0);
		  printf("about to add %s\n",buf);
                  add_grid_str(buf,ng_m);
               }
               free(clm_r[q]); clm_r[q]=0;
               free(clm_i[q]); clm_i[q]=0;
            }
         }
      }

      if (!(ng_phase=galloc(1,COORD_CURVILINEAR,&numt,0.0,tc,0,phase))) return 0;
      free(phase); phase=0;
      free(tc); tc=0;
      sprintf(buf,"%s_phase",tag,l,m);
      add_grid_str(buf,ng_phase);
      if (!(ng_spec_r=galloc(2,COORD_UNIFORM,spec_shape,0.0,spec_coords,0,spec_r))) return 0;
      if (!(ng_spec_i=galloc(2,COORD_UNIFORM,spec_shape,0.0,spec_coords,0,spec_i))) return 0;
      sprintf(buf,"%s_spec_r",tag);
      add_grid_str(buf,ng_spec_r);
      sprintf(buf,"%s_spec_i",tag);
      add_grid_str(buf,ng_spec_i);
      free(spec_r); spec_r=0;
      free(spec_i); spec_i=0;

      printf("\nNOTE: only saving |m|=even 1D files at the moment\n");
   }

   // saved 'filtered' version of the wave form
   if (range) 
   {
      sprintf(buf,"%s_filtered",it_a->r->name);
      add_grid_str(buf,f_ng_r);
      sprintf(buf,"%s_filtered",it_b->r->name);
      add_grid_str(buf,f_ng_i);
   }

   return ng;
}

//=============================================================================
// gh3d specific ... input is cylm(psi4_r,psi4_i) as produced by the cylm
// routine, constructed from psi4_r/i in spherical polar coordinates i.e.,
// as produce by eval_r, and is expected to be uniform/unigrid. 
// Argument is R, and *must* be the same value used in eval_r to get the 
// correct answer. 
//
// assuming a spherical harmonic expansion of the complex function psi4:
//
// psi4 = sum(l,m) clm(t)*Ylm(theta,phi)
//
// and defining
//
// zlm(t) = int[0..t] { clm(t') dt' }
//
// xlm(t) = int[0..t] { zlm(t') dt' }
// 
// we get the following
//
// de/dt = (R^2/4pi) * sum(l,m) (zlm(t) * bar[zlm(t)])
//
// dJz/dt = (R^2/4pi) * sum(l,m) { m * ( Im[zlm(t)]*Re[xlm(t)] - Re[zlm(t)]*Im[xlm(t)] ) }
//
// format of input register is as given above for output function 
//
//=============================================================================
grid *gf_int_psi4_spec_r(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng=0,*nxt_g,*ng_E,*ng_Jz,*ng_b=0,*ng_dE_dt,*ng_dJz_dt;
   static double R,tn,tnm1,*z,znm1_i,pt,*cn,*cnm1,*E,*de_dt_nm1,*de_dt_n,t_de_dt_n,t_de_dt_nm1,*x,*dE_dt,*dJz_dt,*tc;
   static double *Jz,*dJz_dt_nm1,*dJz_dt_n;
   static int dsize,Nl,Nm,numt,n,t0;
   int i,range,ind,ls,le,ms,me,l0,m0;
   double E0,E0_c,x_r,x_i,z_r,z_i,Jz0,Jz0_c;

   get_single_farg(&R,args,first_call,-1.0,"gf_int_psi4_spec_r");

   if (g->dim!=2) {printf("gf_int_psi4_spec_r: dim!=2\n"); return 0;}
   if (g->coord_type!=COORD_UNIFORM) {printf("gf_int_psi4_spec_r: dim!=2\n"); return 0;}

   if (first_call) 
   { 
      printf("\n\nNOTE:Expression for E assumes `symmetric' 1/sqrt(2) normalization.\n"
             "multiply by 4 for Teukolsky's (73) normalization of tetrads\n\n");

      tn=tnm1=0;
      cn=cnm1=0;
      de_dt_n=de_dt_nm1=0;
      dJz_dt_n=dJz_dt_nm1=0;
      dE_dt=0; dJz_dt=0; tc=0;
      t_de_dt_n=t_de_dt_nm1=0;
      dsize=sizeof_data(g);
      Nl=g->shape[0];
      Nm=g->shape[1];
      E=z=x=Jz=0;
      znm1_i=0;
      if (!(z=(double *)malloc(sizeof(double)*dsize))) 
         { printf("gf_int_psi4_spec_r: error ... out of memory\n"); return 0; }
      if (!(E=(double *)malloc(sizeof(double)*dsize))) 
         { printf("gf_int_psi4_spec_r: error ... out of memory\n"); return 0; }
      if (!(x=(double *)malloc(sizeof(double)*dsize))) 
         { printf("gf_int_psi4_spec_r: error ... out of memory\n"); return 0; }
      if (!(Jz=(double *)malloc(sizeof(double)*dsize))) 
         { printf("gf_int_psi4_spec_r: error ... out of memory\n"); return 0; }
      for (i=0; i<dsize; i++) E[i]=z[i]=Jz[i]=x[i]=0;
      tn=it_a->ts->time;
      cn=g->data;

      save_s_iter(it_a);
      nxt_g=g;
      pt=it_a->ts->time;
      t0=pt;
      numt=1; 
      n=1;
      while(nxt_g) 
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            pt=it_a->ts->time;
         } 
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);

      if (!(dE_dt=(double *)malloc(sizeof(double)*numt))) 
         { printf("gf_int_psi4_spec_r: error ... out of memory\n"); return 0; }
      if (!(dJz_dt=(double *)malloc(sizeof(double)*numt))) 
         { printf("gf_int_psi4_spec_r: error ... out of memory\n"); return 0; }
      if (!(tc=(double *)malloc(sizeof(double)*numt))) 
         { printf("gf_int_psi4_spec_r: error ... out of memory\n"); return 0; }

      for (i=0; i<numt; i++) dE_dt[i]=dJz_dt[i]=tc[i]=0;

      tc[0]=t0;
   }
   else
   {
      if (Nl!=Nm) { printf("gf_int_psi4_spec_r : error ... shape info wrong"); return 0; }
      range=(Nm-1)/2;
      ls=2,le=range;
      ms=-range; me=range;

      n++;
      if (!z) return 0;
      tnm1=tn; tn=it_a->ts->time; tc[n-1]=tn;
      if (tn==tnm1 || sizeof_data(g)!=dsize) { printf("gf_int_psi4_spec_r : error ... uniform/unigrid function expected\n"); return 0; }

      if (!(ng=galloc(g->dim,g->coord_type,g->shape,tn+(tnm1-tn)/2,g->coords,g->ccoords,g->data))) { printf("gf_int_psi4_r: out of memory\n"); return 0;}
      t_de_dt_nm1=t_de_dt_n; t_de_dt_n=tn+(tnm1-tn)/2;

      de_dt_nm1=de_dt_n; de_dt_n=ng->data;
      cnm1=cn; cn=g->data;

      if (!(ng_b=galloc(g->dim,g->coord_type,g->shape,tn+(tnm1-tn)/2,g->coords,g->ccoords,g->data))) { printf("gf_int_psi4_r: out of memory\n"); return 0;}
      dJz_dt_nm1=dJz_dt_n; dJz_dt_n=ng_b->data;

      for (i=0; i<dsize; i++) 
      {
         znm1_i=z[i];
         z[i]+=(0.5*(cn[i]+cnm1[i])*(tn-tnm1)); // simpson's rule
         x[i]+=(0.5*(z[i]+znm1_i)*(tn-tnm1)); // simpson's rule
         
         de_dt_n[i]=(R*R/16/M_PI)*z[i]*z[i];
         dE_dt[n-1]+=de_dt_n[i];
         if (de_dt_nm1) E[i]+=(0.5*(de_dt_n[i]+de_dt_nm1[i])*(t_de_dt_n-t_de_dt_nm1));
         dJz_dt_n[i]=0;
      }

      for (l0=ls; l0<=le; l0++)
      {
         for (m0=ms; m0<=me; m0++)
         {
            if (abs(m0)<=l0 && abs(m0)>=2)
            {
               // [l+range,m+range] stores the [l,m] real component (only this non-zero for Jz)
               // [range-l,m+range] stores the [l,m] imaginary component
               
               ind=(l0+range)+(m0+range)*g->shape[0];
               z_r=z[ind]; x_r=x[ind];
               ind=(range-l0)+(m0+range)*g->shape[0];
               z_i=z[ind]; x_i=x[ind];
               //store in 'real' part of dJz_dt_n
               ind=(l0+range)+(m0+range)*g->shape[0];
               dJz_dt_n[ind]=-(R*R/16/M_PI)*m0*(z_i*x_r-z_r*x_i);
               if (dJz_dt_nm1) Jz[ind]+=(0.5*(dJz_dt_n[ind]+dJz_dt_nm1[ind])*(t_de_dt_n-t_de_dt_nm1));
               dJz_dt[n-1]+=dJz_dt_n[ind];
            }
         }
      }
   }

   if (n==numt)
   {
      if (!(ng_dE_dt=galloc(1,COORD_CURVILINEAR,&numt,0.0,tc,0,dE_dt))) return 0;
      add_grid_str("dE_dt_psi4_spec_r",ng_dE_dt);
      if (!(ng_dJz_dt=galloc(1,COORD_CURVILINEAR,&numt,0.0,tc,0,dJz_dt))) return 0;
      add_grid_str("dJz_dt_psi4_spec_r",ng_dJz_dt);
      free(dE_dt); dE_dt=0;
      free(dJz_dt); dJz_dt=0;
      free(tc); tc=0;

      if (Nm>=2 && Nm==Nl)
      {
         E0=0; 
         E0_c=0; 
         Jz0=0;
         Jz0_c=0;

         printf("Total energy, angular momentum in all components:\n");
         for (l0=ls; l0<=le; l0++)
         {
            for (m0=ms; m0<=me; m0++)
            {
               if (abs(m0)<=l0 && abs(m0)>=2)
               {
                  ind=(l0+range)+(m0+range)*g->shape[0];
               
                  E0+=E[ind]; Jz0+=Jz[ind];
                  
                  printf("  l=%i, m=%i: E(real) = %lf, de_dt(N)=%lf, Jz=%lf,  dJz_dt(N)=%lf",l0,m0,E[ind],de_dt_n[ind],Jz[ind],dJz_dt_n[ind]);
                  E0_c+=de_dt_n[ind]*(t_de_dt_n-t0)/2;
                  Jz0_c+=dJz_dt_n[ind]*(t_de_dt_n-t0)/2;
               
                  ind=range-l0+(m0+range)*Nl; E0+=E[ind];
                  printf("  E(imag) = %lf, de_dt(N)=%lf\n",E[ind],de_dt_n[ind]);
                  E0_c+=de_dt_n[ind]*(t_de_dt_n-t0)/2;
               }
            }
         }
         printf("\n E total=%lf, correction=%lf, adjusted total=%lf\n\n",E0,E0_c,E0-E0_c);
         printf("\n Jz total=%lf, correction=%lf, adjusted total=%lf\n\n",Jz0,Jz0_c,Jz0-Jz0_c);
      }
      if (!(ng_E=galloc(g->dim,g->coord_type,g->shape,t_de_dt_nm1+(t_de_dt_n-t_de_dt_nm1)/2,g->coords,0,E))) return 0;
      add_grid_str("E_int_psi4_spec_r",ng_E);
      free(E); E=0;
      free(z); z=0;
      free(x); x=0;
      if (!(ng_Jz=galloc(g->dim,g->coord_type,g->shape,t_de_dt_nm1+(t_de_dt_n-t_de_dt_nm1)/2,g->coords,0,Jz))) return 0;
      add_grid_str("Jz_int_psi4_spec_r",ng_Jz);
      free(Jz); Jz=0;
   }

   if (ng_b) add_grid_str("dJz_dt_components",ng_b);
   return ng;
}


//-----------------------------------------------------------------------------
// gh3d specific, multiplies by r^p
//-----------------------------------------------------------------------------
grid *gf_multbyrp(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   grid *g=it_a->g,*ng=0;
   int i,j,k,ind,Nx,Ny,Nz;
   double x,y,z,x0,y0,z0,dx,dy,dz;
   static double p;

   get_single_farg(&p,args,first_call,1.0,"gf_multbyrp");

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)))
   {
      printf("gf_multbyrp: out of memory\n");
      return 0;
   }

   if (!(g->coord_type==COORD_UNIFORM || g->coord_type==COORD_SEMI_UNIFORM))
   {
      printf("gf_multbyrp: only (semi)uniform coordinates supported at this stage\n");
      return 0;
   }

   Nx=ng->shape[0]; x0=ng->coords[0]; dx=(ng->coords[1]-x0)/(Nx-1);
   if (ng->dim>1) { Ny=ng->shape[1]; y0=ng->coords[2]; dy=(ng->coords[3]-y0)/(Ny-1); } else { Ny=1; y0=dy=0; }
   if (ng->dim>2) { Nz=ng->shape[2]; z0=ng->coords[4]; dz=(ng->coords[5]-z0)/(Nz-1); } else { Nz=1; z0=dz=0; }

   for (i=0, x=x0; i<Nx; i++, x+=dx)
   {
      for (j=0, y=y0; j<Ny; j++, y+=dy)
      {
         for (k=0, z=z0; k<Nz; k++, z+=dz)
         {
            ind=i+j*Nx+k*Nx*Ny;
            if ((1-fabs(x))>dx/2 && (1-fabs(y))>dy/2 && (1-fabs(z))>dz/2)
            {
               ng->data[ind]*=pow(pow(tan(M_PI*x/2),2)+pow(tan(M_PI*y/2),2)+pow(tan(M_PI*z/2),2),p/2);
            }
         }
      }
   }

   return ng;
}

//-----------------------------------------------------------------------------
// gh3d specific ... ah smooth function, multiplies by r^p
//-----------------------------------------------------------------------------
grid *gf_smooth_ah_r(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   grid *g=it_a->g,*ng=0,*var=0,*w1=0;
   static double save_var;
   double eps=0.5;

   get_single_farg(&save_var,args,first_call,0.0,"gf_smooth_ah_r");

   if ((g->dim!=2))
   {
      printf("gf_smooth_ah_r: 2D function required\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)) ||
       !(var=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)) ||
       !(w1=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)) )
   {
      printf("gf_smooth_ah_r: out of memory\n");
      if (ng) gfree(ng);
      if (var) gfree(var);
      if (w1) gfree(w1);
      return 0;
   }

   smooth_ah_r_(ng->data,var->data,w1->data,&eps,&g->shape[0],&g->shape[1]);

   gfree(w1); if (save_var>1e-8) { gfree(ng); ng=var; } else gfree(var);

   return ng;
}

//-----------------------------------------------------------------------------
// gh3d specific ... attaches a curvilinear coordinate system with
// the uncompactified coordinates
//-----------------------------------------------------------------------------
grid *gf_uncompact(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   grid *g=it_a->g,*ng=0;
   int i,j,k,ind,Nx,Ny,Nz;
   double x,y,z,x0,y0,z0,dx,dy,dz,xb0,yb0,zb0;

   if (!(g->coord_type==COORD_UNIFORM || g->coord_type==COORD_SEMI_UNIFORM))
   {
      printf("gf_uncompact: only (semi)uniform coordinates supported at this stage\n");
      return 0;
   }

   if (!(ng=galloc(g->dim,COORD_SEMI_UNIFORM,g->shape,g->time,g->coords,g->ccoords,g->data)))
   {
      printf("gf_uncompact: out of memory\n");
      return 0;
   }

   Nx=ng->shape[0]; x0=ng->coords[0]; dx=(ng->coords[1]-x0)/(Nx-1);
   if (ng->dim>1) { Ny=ng->shape[1]; y0=ng->coords[2]; dy=(ng->coords[3]-y0)/(Ny-1); } else { Ny=1; y0=dy=0; }
   if (ng->dim>2) { Nz=ng->shape[2]; z0=ng->coords[4]; dz=(ng->coords[5]-z0)/(Nz-1); } else { Nz=1; z0=dz=0; }

   for (i=0, x=x0; i<Nx; i++, x+=dx)
   {
      if (x<(-1+dx/2)) xb0=tan(M_PI*(-1+dx/2)/2);
      else if (x>(1-dx/2)) xb0=tan(M_PI*(1-dx/2)/2);
      else xb0=tan(M_PI*x/2);
      for (j=0, y=y0; j<Ny; j++, y+=dy)
      {
         if (y<(-1+dy/2)) yb0=tan(M_PI*(-1+dy/2)/2);
         else if (y>(1-dy/2)) yb0=tan(M_PI*(1-dy/2)/2);
         else yb0=tan(M_PI*y/2);
         for (k=0, z=z0; k<Nz; k++, z+=dz)
         {
            if (z<(-1+dz/2)) zb0=tan(M_PI*(-1+dz/2)/2);
            else if (z>(1-dz/2)) zb0=tan(M_PI*(1-dz/2)/2);
            else zb0=tan(M_PI*z/2);
            ind=i+j*Nx+k*Nx*Ny;
            ng->ccoords[ind]=xb0;
            if (ng->dim>1) ng->ccoords[ind+Nx*Ny*Nz]=yb0;
            if (ng->dim>2) ng->ccoords[ind+2*Nx*Ny*Nz]=zb0;
         }
      }
   }

   return ng;
}

grid *gf_uni_avg(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   static int sn,axis;
   grid *g=it_a->g,*ng=0;
   double *maskd=0,*tmp,*q,*p,maskv;
   int l,n,j,ck;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)))
   {
      printf("gf_uni_avg: out of memory\n");
      return 0;
   }

   if (first_call) 
   { 
      if (args)
      {
         sscanf(args,"%i,%i",&sn,&axis);
         if (axis<0 || axis>2)
            printf("gf_uni_avg: illegal second argument %i, must be 0,1 or 2\nusing 2",axis);
      }
      else
      {
         sn=1; 
         axis=2;
      }
      if (sn<0) sn=1;
      if (axis<0 || axis>2) axis=2;
      printf("gf_uni_avg: n=%i, axis=%i\n",sn,axis);
   }

   n=sizeof_data(ng);
   if (!(tmp=(double *)malloc(sizeof(double)*n)))
   {
      printf("gf_diss_ko2: out of memory\n");
      gfree(ng);
      return 0;
   }
   if (mask)
   {
      maskd=mask->data;
      maskv=mask_val;
   }
   else
   {
      maskd=tmp;
      maskv=1.0e-10;
   }
   for(j=0,p=ng->data,q=tmp; j<n; j++) *q++=*p++;
   _uni_avg__(ng->data,tmp,maskd,&maskv,&sn,&axis,&g->shape[0],&g->shape[1]);

   free(tmp);
   return ng;
}

grid *gf_separate_rb(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   grid *g=it_a->g,*ng=0;

   if (g->dim!=2)
   {
      printf("gf_separate_rb: only 2D grids supported\n");
      return 0;
   }
   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)))
   {
      printf("gf_separate_rb: out of memory\n");
      return 0;
   }

   _separate_rb__(g->data,ng->data,&g->shape[0],&g->shape[1]);

   return ng;
}

//=============================================================================
// graxi specific:
// computes the z-axis proper distance between two excised regions.
// The input grid is assumed to be psi.
//=============================================================================
grid *gf_distance_2BH(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   double d,dz;
   grid *g=it_a->g;

   if (!mask)
   {
      printf ("gf_distance_2BH: mask required\n");
      return 0;
   }

   dz=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
   _distance_2bh__(g->data,mask->data,&d,&dz,&(g->shape[0]),&(g->shape[1]));

   printf("distance_2bh__: t=%f, d=%f\n",it_a->ts->time,d);
   return 0;
}

//=============================================================================
// graxi specific: computes d(f/rho)/drho
//=============================================================================
grid *gf_d_overrho(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{  
   double c_mask_val;
   grid *g=it_a->g,*ng;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;

   if (!mask)
   {
      c_mask_val=-1.0e10;
      _d_overrho__(g->data,ng->data,g->data,&c_mask_val,&g->coords[0],&g->coords[2],
                  &g->shape[0],&g->shape[1]);
   }
   else
      _d_overrho__(g->data,ng->data,mask->data,&mask_val,&g->coords[0],&g->coords[2],
                  &g->shape[0],&g->shape[1]);

   return ng;
}
//=============================================================================
// graxi specific: computes d((df/drho)/rho)/drho
//============================================================================= 
grid *gf_d_overrho_d(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   double c_mask_val;
   grid *g=it_a->g,*ng;
 
   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
 
   if (!mask)
   {
      c_mask_val=-1.0e10;
      _d_overrho_d__(g->data,ng->data,g->data,&c_mask_val,&g->coords[0],&g->coords[1],
                  &g->shape[0],&g->shape[1]);
   }
   else
// Don't ferget that leading underscore if yer going to have an embedded underscore!
      _d_overrho_d__(g->data,ng->data,mask->data,&mask_val,&g->coords[0],&g->coords[1],
                  &g->shape[0],&g->shape[1]);
   return ng;
} 

//=============================================================================
// graxi specific: computes d(f)/d(rho^2)
//============================================================================= 
grid *gf_d_byrhosq(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   double c_mask_val;
   grid *g=it_a->g,*ng;
 
   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
 
   if (!mask)
   {
      c_mask_val=-1.0e10;
      _d_byrhosq__(g->data,ng->data,g->data,&c_mask_val,&g->coords[0],&g->coords[1],
                  &g->shape[0],&g->shape[1]);
   }
   else
      _d_byrhosq__(g->data,ng->data,mask->data,&mask_val,&g->coords[0],&g->coords[1],
                  &g->shape[0],&g->shape[1]);
 
   return ng;
} 

//=============================================================================
// graxi specific: computes the lth spectral coefficient of a function,
// (produced by compact[null] ---> after to_uniform)
//
// first coordinate is expected to be theta
//
// uses trapezoidal rule integration:
//
// [-(2*l+1)*f(x)*P_l(x)dx/2] over x=[-1,1]   , x=cos(theta)
//
//============================================================================= 
grid *gf_spec_coeff(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   static double l;
   grid *g=it_a->g,*ng;
   double dtheta,res,theta,x,dx,res_t0,res_t1,res_t2,res_t3,res_t4;
   int ell,i,j,test=1;

   get_single_farg(&l,args,first_call,0,"gf_spec_coeff");
   if (l<0) l=0;
   ell=l+0.5;
   l=ell;

   dtheta=(g->coords[1]-g->coords[0])/(g->shape[0]-1);

   if (g->dim!=2 || g->coord_type!=COORD_UNIFORM || 
       !(fuzz_eq(g->coords[0],0,dtheta/10)) || 
       !(fuzz_eq(g->coords[1],M_PI,dtheta/10))) 
   {
      printf("gf_spec_coeff: input grid must be uniform 2D, with x coord ranging from 0 to pi\n");
      return 0;
   }

   if (first_call) printf("gf_spec_coeff: l=%i\n",ell);

   if (!(ng=galloc(1,COORD_UNIFORM,&g->shape[1],g->time,&g->coords[2],0,0))) return 0;

   for (j=0; j<g->shape[1]; j++)
   {
      res=0;
      if (test && first_call) res_t1=res_t2=res_t3=res_t4=0;
      for (i=0,theta=0; i<(g->shape[0]-1); i++,theta+=dtheta)
      {
         dx=cos(theta+dtheta)-cos(theta);
         x=cos(theta+dtheta/2);
         res+=(g->data[i+j*g->shape[0]]+g->data[i+1+j*g->shape[0]])/2*
              Pl_x(ell,x)*dx;
         if (test && first_call && j==0)
         {
            res_t0+=Pl_x(0,x)*Pl_x(ell,x)*dx;
            res_t1+=Pl_x(1,x)*Pl_x(ell,x)*dx;
            res_t2+=Pl_x(2,x)*Pl_x(ell,x)*dx;
            res_t3+=Pl_x(3,x)*Pl_x(ell,x)*dx;
            res_t4+=Pl_x(4,x)*Pl_x(ell,x)*dx;
         }
      }
      res*=-(2*l+1)/2;
      ng->data[j]=res;
      if (test && first_call && j==0)
      {
         res_t0*=-(2*l+1)/2;
         res_t1*=-(2*l+1)/2;
         res_t2*=-(2*l+1)/2;
         res_t3*=-(2*l+1)/2;
         res_t4*=-(2*l+1)/2;
         printf("test: res_t0,1,2,3,4: %lf,%lf,%lf,%lf,%lf\n",res_t0,res_t1,res_t2,res_t3,res_t4);
      }
   }
         
   return ng;
} 

//============================================================================= 
// rescales r to [ln(r+c)-ln(c)], via conversion of the grid to a curvilinear one
//============================================================================= 
grid *gf_r_to_lnr(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double c;
   int n,i,j,ind;
   static int flip;
   double xi,dx,yi,dy,x,y,r,lnr,*xc,*yc,xn,yn;

   if (g->dim!=2 && g->coord_type!=COORD_UNIFORM)
   {
      printf("gf_r_to_lnr: input grid must be a 2D, uniform grid");
      return 0;
   }

   if (!(ng=galloc(g->dim,COORD_SEMI_UNIFORM,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);
   xc=ng->ccoords;
   yc=&ng->ccoords[n];

   dy=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
   yi=g->coords[2];
   dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   xi=g->coords[0];

   if (first_call)
   {
      flip=0;
      sscanf(args,"%lf,%i",&c,&flip);
      printf("%s: arg=%f [flip=%i]\n","gf_r_to_lnr",c,flip);
   }

   for(i=0,x=xi;i<g->shape[0];i++,x+=dx)
      for(j=0,y=yi;j<g->shape[1];j++,y+=dy)
      {
         r=sqrt(x*x+y*y);
         lnr=log(r+c)-log(c);
         if (r==0) { xn=yn=0; }
         else
         {
            xn=lnr*x/r; if (flip) xn*=-1;
            yn=lnr*y/r;
         }
         xc[i+j*g->shape[0]]=xn;
         yc[i+j*g->shape[0]]=yn;
      }
   
    return ng;
}

//============================================================================= 
// attaches a cartesian curvilinear grid to a uniform coordinate system 
// assumed to be spherical polar
//============================================================================= 
grid *gf_rt_to_xy(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i,j,ind;
   double dtheta,theta0,dr,r0,r,theta,*xc,*yc,*rc=0,*tc=0;

   if (g->dim!=2 && !(g->coord_type==COORD_UNIFORM || g->coord_type==COORD_SEMI_UNIFORM))
   {
      printf("gf_rt_to_xy: input grid must be a 2D, uniform or semi-uniform grid");
      return 0;
   }

   if (!(ng=galloc(g->dim,COORD_SEMI_UNIFORM,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);
   xc=ng->ccoords;
   yc=&ng->ccoords[n];

   dtheta=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
   theta0=g->coords[2];
   dr=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   r0=g->coords[0];
   if (g->coord_type==COORD_SEMI_UNIFORM)
   {
      rc=g->ccoords;
      tc=&g->ccoords[n];
   }

   if (r0<0 && first_call) 
      printf("gf_rt_to_xy: warning ... expecting first coordinate to be a radial\n" 
             "coordinate, but negative values encountered\n");
   if ((g->coords[3]-g->coords[2])>(2*M_PI) && first_call) 
      printf("gf_rt_to_xy: warning ... expecting second coordinate to be an angular\n"
             "coordinate, but size of domain extends beyond 2*PI\n");

   for(i=0,r=r0;i<g->shape[0];i++,r+=dr)
      for(j=0,theta=theta0;j<g->shape[1];j++,theta+=dtheta)
      {
         ind=i+j*g->shape[0];
         if (rc)
         {
            xc[ind]=rc[ind]*sin(tc[ind]);
            yc[ind]=rc[ind]*cos(tc[ind]);
         }
         else
         {
            xc[ind]=r*sin(theta);
            yc[ind]=r*cos(theta);
         }
      }
   
    return ng;
}

//============================================================================= 
// gh3d specific. maps the AH function into a sphere
//
// read_txyz is a utility routine that reads info from a gh3d
// output file, used by gf_rtp_to_zxy and other functions
// returns a pointer q to a memory block of reals of size (1+qnum)*sizet
// 
// t is assumed to be in col 1, and [x,y,z] are 3 columns starting at p_col
// 
// q[0]=t vec
// if (qnum>0) q[sizet]=x vec
// if (qnum>1) q[2*sizet]=y vec
// if (qnum>2) q[3*sizet]=z vec
//
//============================================================================= 
double *read_txyz(char *pfile,int p_col,int *sizet, int qnum)
{
   double *t=0,*x0=0,*y0=0,*z0=0;
   char buffer[256],c;
   int b_pos,t_pos,col;
   FILE *file;

   if (qnum<1 || qnum>3) {printf("read_txyz:error, qnum is out of bounds\n"); return 0;}

   printf("reading position information from file %s;\n"
          "t is assumed to be in column 1, position info starts in column %i\n",pfile,p_col);
   if (!(file=fopen(pfile,"r")))
   {
      printf("error opening file %s\n",pfile);
   }
   else
   {
      (*sizet)=0;
      while((c=fgetc(file))!=EOF) {if (c=='\n') (*sizet)++;}
      fclose(file);
      if (!(*sizet)) 
      {
         printf("error ... file is empty\n");
      }
      else if (!(t=(double *)malloc((1+qnum)*sizeof(double)*(*sizet))))
      {
         printf("error ... out of memory\n");
      }
      else
      {
         if (qnum>0) x0=&t[(*sizet)];
         if (qnum>1) y0=&t[2*(*sizet)];
         if (qnum>2) z0=&t[3*(*sizet)];
         if (!(file=fopen(pfile,"r")))
         {
            printf("error re-opening file %s\n",pfile);
         }
         else
         {
            b_pos=0;
            t_pos=0;
            while((buffer[b_pos]=fgetc(file))!=EOF && b_pos<256 && t_pos<(*sizet)) 
            {
               if (buffer[b_pos]=='\n')
               {
                  b_pos=0;
                  col=1;
                  while(buffer[b_pos]==' ') b_pos++;
                  sscanf(&buffer[b_pos],"%lf",&t[t_pos]);
                  while(buffer[b_pos]!='\n' && col<p_col)
                  {
                     if (buffer[b_pos]==' ') { while(buffer[b_pos]==' ') b_pos++; col++; }
                     else b_pos++;
                  }
                  switch (qnum)
                  {
                     case 1:
                        sscanf(&buffer[b_pos],"%lf",&x0[t_pos]);
                        printf("t=%lf, p=[%lf]\n",t[t_pos],x0[t_pos]);
                        break;
                     case 2:
                        sscanf(&buffer[b_pos],"%lf %lf",&x0[t_pos],&y0[t_pos]);
                        printf("t=%lf, p=[%lf,%lf]\n",t[t_pos],x0[t_pos],y0[t_pos]);
                        break;
                     case 3:
                        sscanf(&buffer[b_pos],"%lf %lf %lf",&x0[t_pos],&y0[t_pos],&z0[t_pos]);
                        printf("t=%lf, p=[%lf,%lf,%lf]\n",t[t_pos],x0[t_pos],y0[t_pos],z0[t_pos]);
                        break;
                  }
                  t_pos++;
                  b_pos=0;
               }
               else
               {
                  b_pos++;
               }
            }
            fclose(file);
         }
      }
   }

   return t;
}

#ifdef __
extern void gh3d_ah_best_fit__(double *AH_r, int *AH_Ntheta, int *AH_Nphi);
#else
extern void gh3d_ah_best_fit_(double *AH_r, int *AH_Ntheta, int *AH_Nphi);
#endif
grid *gf_rtp_to_zxy(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng,*nxt_g;
   int n,i,j,ind,ret,p_shape[2];
   double dtheta,theta0,dphi,phi0,phi,theta,*xc,*yc,*z,*r,dx,dy,dz,p_bbox[2];
   static double csphere;
   static double *t,*x0,*y0,*z0;
   static int sizet,particles;
   char pfile[64];
   int p_col,t_pos;

   if (g->dim!=2 && g->coord_type!=COORD_UNIFORM)
   {
      printf("gf_rtp_to_zxy: input grid must be a 2D, uniform grid");
      return 0;
   }

   if (first_call)
   {
      csphere=0;
      pfile[0]=0;
      t=x0=y0=z0=0;
      sizet=0;
      particles=0;
      if (args && strlen(args)>0) sscanf(args,"%lf,%i,%i,%s",&csphere,&particles,&p_col,pfile);
      if (csphere!=0) printf("gf_rtp_to_zxy: transforming to a compactified coordinate sphere\n");
      if (particles!=0) printf("gf_rtp_to_zxy: generating a particle register\n");
      if (pfile[0]) 
      {
         t=read_txyz(pfile,p_col,&sizet,3);
         if (t) 
         {
            x0=&t[sizet];
            y0=&t[2*sizet];
            z0=&t[3*sizet];
         }
      }
      if (csphere<0) printf("csphere<0 ... test ... finding 'best-fit' ellipse\n");
   }

   if (!particles) 
   {
      if (!(ng=galloc(g->dim,COORD_SEMI_UNIFORM,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0; 
      n=sizeof_data(ng);
      xc=ng->ccoords;
      yc=&ng->ccoords[n];
      z=ng->data;
      if (csphere<0) gh3d_ah_best_fit_(ng->data,&ng->shape[0],&ng->shape[1]);
      r=ng->data;
   }
   else 
   {
      p_shape[0]=g->shape[0]*g->shape[1]; p_shape[1]=3;
      p_bbox[0]=p_bbox[2]=-1; p_bbox[1]=p_bbox[3]=1;
      
      if (!(ng=galloc(2,COORD_UNIFORM,p_shape,g->time,p_bbox,0,0))) return 0; 

      n=p_shape[0];
      xc=ng->data;
      yc=&ng->data[n];
      z=&ng->data[2*n];
      r=g->data;
   }

   dtheta=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   theta0=g->coords[0];
   dphi=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
   phi0=g->coords[2];

   if ((g->coords[3]-g->coords[2])>(2*M_PI) && first_call) 
      printf("gf_rt_to_xy: warning ... expecting second coordinate to be an azimuthal\n" 
             "coordinate, but size of domain extends beyond 2*PI\n");
   if ((g->coords[1]-g->coords[0])>(M_PI) && first_call) 
      printf("gf_rtp_to_zxy:: warning ... expecting first coordinate to be a latitudinal\n"
             "coordinate, but size of domain extends beyond PI\n");

   dx=dy=dz=0;
   if (t)
   {
      t_pos=0;
      while(t_pos<(sizet-1) && t[t_pos+1]<=it_a->ts->time) t_pos++;
      dx=tan(x0[t_pos]*M_PI/2);
      dy=tan(y0[t_pos]*M_PI/2);
      dz=tan(z0[t_pos]*M_PI/2);
   }

   for(i=0,theta=theta0;i<g->shape[0];i++,theta+=dtheta)
      for(j=0,phi=phi0;j<g->shape[1];j++,phi+=dphi)
      { 
         ind=i+j*g->shape[0];
         if (csphere!=0)
         {
            xc[ind]=2*atan(r[ind]*cos(phi)*sin(theta)+dx)/M_PI;
            yc[ind]=2*atan(r[ind]*sin(phi)*sin(theta)+dy)/M_PI;
            z[ind]=2*atan(r[ind]*cos(theta)+dz)/M_PI;
         }
         else
         {
            xc[ind]=r[ind]*cos(phi)*sin(theta)+dx;
            yc[ind]=r[ind]*sin(phi)*sin(theta)+dy;
            z[ind]=r[ind]*cos(theta)+dz;
         }
      }

   if (t)
   {
      ret=1;
      save_s_iter(it_a);
      nxt_g=next_g(it_a);
      while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
      restore_s_iter(it_a);
      if (ret) free(t);
   }
   
   return ng;
}

//-----------------------------------------------------------------------------
// transforms the time of a BBH merger to coordinate orbital freq
// input is assumed to be a 1d function, with the 'x' coordinate equal
// to time.
//-----------------------------------------------------------------------------
grid *gf_bbh_t_to_v(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng,*nxt_g,*ng2;
   int t_pos,i,ret;
   static double *t,*x0,*y0,*z0,delta_theta,MT,r0;
   static int sizet,scale;
   char pfile[64];
   int p_col;
   int is,ie,nshape;
   double t0,dt,t_is,t_ie,theta0,dtheta,p_theta,theta,xb,yb,frac,*omega=0;

   if (first_call)
   {
      pfile[0]=0;
      t=x0=y0=z0=0;
      sizet=0;
      delta_theta=0;
      if (args && strlen(args)>0) sscanf(args,"%lf,%lf,%i,%i,%s",&MT,&r0,&scale,&p_col,pfile);
      if (pfile[0]) 
      {
         t=read_txyz(pfile,p_col,&sizet,3);
         if (t) 
         {
            x0=&t[sizet];
            y0=&t[2*sizet];
            z0=&t[3*sizet];
            printf("using MT=%lf\n",MT);
            printf("shifting t by r0=%lf\n",r0);
            for (i=0; i<sizet; i++) t[i]+=r0;
            if (MT<=0) printf("converting t->theta\n"); 
            else printf("converting t->v=(MT*omega)^(1/3), and if (%i) scaling by -2/5 v^10\n",scale);
         }
      }
      else printf("gf_bbh_t_to_v: need a file name!\n");
   }

   if (!t) return 0;

   if (g->dim!=1 || g->coord_type!=COORD_UNIFORM)
   {
      printf("gf_bbh_t_to_v: input grid must be a 1D, uniform grid");
      return 0;
   }

   // find range of times in data set that match input grid
   dt=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   printf("g->coords[0],g->coords[1],t[0],t[sizet-1],sizet:%lf,%lf,%lf,%lf,%i\n",g->coords[0],g->coords[1],t[0],t[sizet-1],sizet);
   if (g->coords[0]>=t[sizet-1] || g->coords[1]<=t[0]) {printf("no times in input file match data set times(x coord)\n"); return 0;}

   t0=g->coords[0];
   is=0;
   if (g->coords[0]<t[0]) for (is=0; t0<=t[0]; t0+=dt) is++;
   t_is=t0;

   t0=g->coords[1];
   ie=g->shape[0]-1;
   if (g->coords[1]>t[sizet-1]) for (ie=g->shape[0]-1, t0=g->coords[1]; t0>=t[sizet-1]; t0-=dt) ie--;
   t_ie=t0;
   
   nshape=ie-is+1;

   if (!(ng=galloc(g->dim,COORD_CURVILINEAR,&nshape,g->time,0,0,&g->data[is]))) return 0; 
   if (!(ng2=galloc(g->dim,COORD_UNIFORM,&nshape,g->time,0,0,&g->data[is]))) return 0; 
   ng2->coords[0]=t_is/MT;
   ng2->coords[1]=t_ie/MT;

   // first define the new coords to be theta
   t_pos=0;
   delta_theta=theta0=dtheta=p_theta=theta=0;
   for (i=0, t0=t_is; i<nshape; i++, t0+=dt)
   {
      while(t_pos<(sizet-1) && t[t_pos+1]<t0) t_pos++;
      frac=(t0-t[t_pos])/(t[t_pos+1]-t[t_pos]);
      if (frac<0 || frac>1) printf("gf_bbh_t_to_v: error ... frac is out of bounds\n");
      xb=x0[t_pos]*(1-frac)+x0[t_pos+1]*frac; xb=tan(M_PI*xb/2);
      yb=y0[t_pos]*(1-frac)+y0[t_pos+1]*frac; yb=tan(M_PI*yb/2);
      if (xb==0 && yb<0) theta0=M_PI;
      else if (xb==0 && yb>0) theta0=0;
      else theta0=atan(yb/xb);

      if ((p_theta-delta_theta)>(M_PI/4) && theta0<0) delta_theta+=M_PI;
      else if ((p_theta-delta_theta)<(-M_PI/4) && theta0>0) delta_theta-=M_PI;
      p_theta=theta;
      theta=theta0+delta_theta;
      ng->coords[i]=theta;
   }

   if (!(omega=(double *)malloc(sizeof(double)*nshape))) {printf("error ... out of memory\n"); return 0;}

   // now ... v  = (MT Omega)^(1/3)
   for (i=0; i<nshape; i++)
   {
      if (i==0) omega[i]=(-3*ng->coords[0]+4*ng->coords[1]-ng->coords[2])/2/dt;
      else if (i==(nshape-1)) omega[i]=(3*ng->coords[i]-4*ng->coords[i-1]+ng->coords[i-2])/2/dt;
      else omega[i]=(ng->coords[i+1]-ng->coords[i-1])/2/dt;
      ng2->data[i]=MT*omega[i];
      omega[i]=pow(fabs(MT*omega[i]),1.0/3.0);
   }
   if (MT>0) 
   {
      for (i=0; i<nshape; i++) 
      {
         ng->coords[i]=omega[i];
         if(scale) ng->data[i]=ng->data[i]/(0.4*pow(omega[i],10));
      }
   }

   add_grid_str("MT*omega(t/MT)",ng2);
   free(omega);

   ret=1;
   save_s_iter(it_a);
   nxt_g=next_g(it_a);
   while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
   restore_s_iter(it_a);
   if (ret) free(t);
   
   return ng;
}

//-----------------------------------------------------------------------------
// given input files m1(t),m2(t) as produced by gh3d, produces 
//
// N1*J([t-N2]/N3) 
//
// assuming these describe point-particle emitting radiation via the
// quadrupole formula,where J is the 4-times in time integrated psi4*r0
// 
// if scale=0, N1=1,N2=0,N3=1
// if scale=1, N1=1/M0**3, N2=0, N3=M0
// if scale=2, N1=1/M0**3, N2=r0, N3=M0
//
// and M0=m10+m20
//
// i.e. ... so even if scale is 0, results have been multiplied by r0
//
// Also produces phase angle phi(t), and coordinate distance d(t)
// 
//-----------------------------------------------------------------------------
void gf_bbh_quad(char *args)
{
   grid *ng_r,*ng_i,*ng_p,*ng_d,*ng_vr,*ng_w,*ng_L,*ng_E,*ng_a,*ng_e;
   int t_pos,i,j;
   double *t1_x,*t1_m,*x1,*y1,*z1,*m1,*x2,*y2,*z2,*m2,r0,m0,*t2_x,*t2_m,m10,m20,d0;
   double *r,*w,*p,*vr,*t,*E,*L,f,E2,*a,*e,t1;
   double delta_theta,theta0,dtheta,p_theta,theta,xb,yb;
   int scale,p_col,m_col,sizet1,sizet2,Ntheta,Nphi;
   char pfile1[64],pfile2[64];
   int is,ie,tnum,nshape[2];
   double t0,dt,t_is,t_ie,coords[4];

   pfile1[0]=pfile2[0]=0;
   t1_x=t1_m=x1=y1=z1=m1=0;
   t2_x=t2_m=x2=y2=z2=m2=0;
   sizet1=sizet2=0;
   if (args && strlen(args)>0) sscanf(args,"%lf,%lf,%lf,%i,%i,%i,%i,%i,%s%s",&r0,&m10,&m20,&scale,&p_col,&m_col,&Ntheta,&Nphi,pfile1,pfile2);
   m0=m10+m20;
   printf("file1,2:%s,%s\n",pfile1,pfile2);
   if (pfile1[0] && pfile2[0]) 
   {
      t1_x=read_txyz(pfile1,p_col,&sizet1,3);
      if (t1_x) 
      {
         x1=&t1_x[sizet1];
         y1=&t1_x[2*sizet1];
         z1=&t1_x[3*sizet1];
         // uncompactify
         for (i=0; i<3*sizet1; i++) x1[i]=(tan(M_PI/2*x1[i]));
      }
      t1_m=read_txyz(pfile1,m_col,&sizet1,1);
      if (t1_m) m1=&t1_m[sizet1];

      t2_x=read_txyz(pfile2,p_col,&sizet2,3);
      if (t2_x) 
      {
         x2=&t2_x[sizet2];
         y2=&t2_x[2*sizet2];
         z2=&t2_x[3*sizet2];
         for (i=0; i<3*sizet2; i++) x2[i]=(tan(M_PI/2*x2[i]));
      }
      t2_m=read_txyz(pfile2,m_col,&sizet2,1);
      if (t2_m) m2=&t2_m[sizet2];
   } else printf("gf_quad: need file names!\n");

   if (sizet1!=sizet2) printf("gf_quad: warning ... files don't have same number of times\n");

   printf("r0=%lf, m0,1,2=%lf,%lf,%lf, scale=%i, p_col=%i, m_col=%i, Ntheta=%i, Nphi=%i\n",r0,m0,m10,m20,scale,p_col,m_col,Ntheta,Nphi);

   if (!t1_x || !t1_m || !t2_x || !t2_m) return;

   is=0;
   ie=sizet1-1;

   tnum=ie-is+1;

   nshape[0]=Ntheta;
   nshape[1]=Nphi;
   coords[0]=0;
   coords[1]=M_PI;
   coords[2]=0;
   coords[3]=2*M_PI;
   for (i=0; i<tnum; i++)
   {
      t0=t1_x[i];
      if (!(ng_r=galloc(2,COORD_UNIFORM,nshape,t0,coords,0,0))) return; 
      if (!(ng_i=galloc(2,COORD_UNIFORM,nshape,t0,coords,0,0))) return; 

      fill_i4psi4_(ng_r->data,ng_i->data,&r0,&m10,&x1[i],&y1[i],&z1[i],&m20,&x2[i],&y2[i],&z2[i],&nshape[0],&nshape[1]);

      for (j=0; j<nshape[0]*nshape[1]; j++)
      {
         switch(scale)
         {
            case 0: break;
            case 1: ng_r->data[j]/=(pow(m0,3)); ng_i->data[j]/=(pow(m0,3));
                    if (j==0) { ng_r->time=(ng_r->time)/m0; ng_i->time=(ng_i->time)/m0; }
                    break;
            case 2: ng_r->data[j]/=(pow(m0,3)); ng_i->data[j]/=(pow(m0,3));
                    if (j==0) { ng_r->time=(ng_r->time+r0)/m0; ng_i->time=(ng_i->time+r0)/m0; }
                    break;
         }
      }

      add_grid_str("bbh_quad_i4_rpsi4_r",ng_r);
      add_grid_str("bbh_quad_i4_rpsi4_i",ng_i);
   }

   // compute coordinate phase and distance
   
   if (scale) for (i=0; i<tnum; i++) t1_x[i]*=(1/m0);

   if (!(ng_p=galloc(1,COORD_CURVILINEAR,&tnum,0,t1_x,0,0))) return; 
   if (!(ng_d=galloc(1,COORD_CURVILINEAR,&tnum,0,t1_x,0,0))) return; 
   if (!(ng_vr=galloc(1,COORD_CURVILINEAR,&tnum,0,t1_x,0,0))) return; 
   if (!(ng_w=galloc(1,COORD_CURVILINEAR,&tnum,0,t1_x,0,0))) return; 
   if (!(ng_L=galloc(1,COORD_CURVILINEAR,&tnum,0,t1_x,0,0))) return; 
   if (!(ng_E=galloc(1,COORD_CURVILINEAR,&tnum,0,t1_x,0,0))) return; 
   if (!(ng_a=galloc(1,COORD_CURVILINEAR,&tnum,0,t1_x,0,0))) return; 
   if (!(ng_e=galloc(1,COORD_CURVILINEAR,&tnum,0,t1_x,0,0))) return; 

   t=ng_p->coords;
   p=ng_p->data;
   r=ng_d->data;
   vr=ng_vr->data;
   w=ng_w->data;
   L=ng_L->data;
   E=ng_E->data;
   a=ng_a->data;
   e=ng_e->data;

   delta_theta=theta0=dtheta=p_theta=theta=0;
   for (i=0; i<tnum; i++)
   {
      xb=x2[i]-x1[i];
      yb=y2[i]-y1[i];
      if (xb==0 && yb<0) theta0=M_PI;
      else if (xb==0 && yb>0) theta0=0;
      else theta0=atan(yb/xb);

      if ((p_theta-delta_theta)>(M_PI/4) && theta0<0) delta_theta+=M_PI;
      else if ((p_theta-delta_theta)<(-M_PI/4) && theta0>0) delta_theta-=M_PI;
      p_theta=theta;
      theta=theta0+delta_theta;
      p[i]=theta;
      r[i]=sqrt(xb*xb+yb*yb);
      if (scale) r[i]*=(1/m0);
   }

   for (i=0; i<tnum; i++)
   {
      if (i==0)
      {
         vr[0]=(-3*r[0]+4*r[1]-r[2])/(t[2]-t[0]);
         w[0]=(-3*p[0]+4*p[1]-p[2])/(t[2]-t[0]);
      }
      else if (i==(tnum-1))
      {
         vr[i]=(3*r[i]-4*r[i-1]+r[i-2])/(t[i]-t[i-2]);
         w[i]=(3*p[i]-4*p[i-1]+p[i-2])/(t[i]-t[i-2]);
      }
      else
      {
         vr[i]=(r[i+1]-r[i-1])/(t[i+1]-t[i-1]);
         w[i]=(p[i+1]-p[i-1])/(t[i+1]-t[i-1]);
      }

      // E,L and f from particle orbit in standard Schwarschild coords
      
      if (scale) f=1-2/r[i]; else f=1-2*m0/r[i];
      E2=f*f*f/(f*f-vr[i]*vr[i]-r[i]*r[i]*w[i]*w[i]*f);
      if (E2<0) printf("Warning ... t=%lf, E^2=%lf<0\n",t1_x[i],E2);
      E[i]=sqrt(fabs(E2));
      L[i]=w[i]*E[i]*r[i]*r[i]/f;

      // semi-major axis and eccentricity of equivalent Newtonian orbit EOB
      if (scale) a[i]=r[i]/(2-r[i]*(vr[i]*vr[i]+r[i]*r[i]*w[i]*w[i]));
      else a[i]=m0*r[i]/(2*m0-r[i]*(vr[i]*vr[i]+r[i]*r[i]*w[i]*w[i]));
      t1=pow(r[i]*r[i]*w[i],2)/a[i];
      if (!scale) t1=t1/m0;
      t1=1-t1;
      if (t1<0) printf("NOTE: ... e^2=%lf<0\n",t1);
      e[i]=sqrt(fabs(t1));
   }

   add_grid_str("bbh_quad_phi",ng_p);
   add_grid_str("bbh_quad_r",ng_d);
   add_grid_str("bbh_quad_vr",ng_vr);
   add_grid_str("bbh_quad_w",ng_w);
   add_grid_str("bbh_quad_E",ng_E);
   add_grid_str("bbh_quad_L",ng_L);
   add_grid_str("bbh_quad_a",ng_a);
   add_grid_str("bbh_quad_e",ng_e);

   free(t1_x); t1_x=0;
   free(t1_m); t1_m=0;
   free(t2_x); t2_x=0;
   free(t2_m); t2_m=0;

   printf("Notes: multiplied by r, and if scale=1 then use '1' for r in energy integration\n");
   printf("       for phase and distance, *assuming* orbit is in z=0. Also, \n");
   printf("       if (scale), scaling units by %lf\n",1/m0);
   
   return;
}

//-----------------------------------------------------------------------------
// visualize a plus-polarized GW (amplitude A, angular frequency omega)`
// moving along (t-z) through a lattice of particles,
// Nxy points in the x & y directions, Nz in the z, with corresponding 
// sizes of the domains Dxy, Dz. z is time in register, and
// using semi-uniform grid to get TT perturbation, and z is time. 
//
// arguments [A,omega,Nxy,Nz,Dxy,Dz,name]
//-----------------------------------------------------------------------------
void gf_gw_vis(char *args)
{
   grid *ng;

   int Nxy,Nz,N,i,j,k,l,ind;
   double A,omega,Dxy,Dz,dxy,dz,x0,y0,t0,z0,*x,*y,h,L,L0,ct,st;

   char reg_name[256]="gw_vis";

   double *data,*ccoords;
   int shape[2];
   double coords[4];

   A=.1; omega=4*M_PI; Nxy=20; Nz=20; Dxy=1; Dz=1;
   if (args && strlen(args)>0) sscanf(args,"%lf,%lf,%i,%i,%lf,%lf,%s",&A,&omega,&Nxy,&Nz,&Dxy,&Dz,reg_name);
   printf("gf_gw_vis : A=%lf, omega=%lf, Nxy=%i, Nz=%i, Dxy=%lf, Dz=%lf, reg_name=%s\n\n",A,omega,Nxy,Nz,Dxy,Dz,reg_name);

   dxy=Dxy/(Nxy-1);
   dz=Dz/(Nz-1);

   shape[0]=Nxy;
   shape[1]=Nxy;
   N=Nxy*Nxy;
   coords[0]=-Dxy/2;
   coords[1]=Dxy/2;
   coords[2]=-Dxy/2;
   coords[3]=Dxy/2;
   for (k=0; k<Nz; k++)
   {
      t0=z0=k*dz; 
      if (!(ng=galloc(2,COORD_SEMI_UNIFORM,shape,t0,coords,0,0))) return;
      data=ng->data;
      x=ng->ccoords;
      y=&ng->ccoords[N];
      for (l=0;l<N;l++) data[l]=z0; 
      for (i=0,x0=coords[0]; i<Nxy; i++,x0+=dxy)
      {
         for (j=0,y0=coords[2]; j<Nxy; j++,y0+=dxy)
         {
            ind=i+j*Nxy;
            //---------------------------------------------------------------------------------------------------------
            // h=A*cos(omega*(z-t)) = A*cos(omega*z); t=0;
            // compute x0,y0 so that the proper distance in the flat (x,y) coordinate system away from (0,0)
            // is the same as in
            //
            // ds^2 = -dt^2 + (1+h) dx^2 + (1-h) dy^2 + dz^2 
            //---------------------------------------------------------------------------------------------------------
            h=A*cos(omega*z0);
            L0=sqrt(x0*x0+y0*y0);
            x[ind]=y[ind]=0;
            if (L0!=0)
            {
               L=L0*sqrt(1+h*(x0*x0-y0*y0)/L0/L0);
               ct=sqrt(1+h)*x0/L;
               st=sqrt(1-ct*ct); if (y0<0) st=-st;
               x[ind]=L*ct;
               y[ind]=L*st;
            }
         }
      }
      add_grid_str(reg_name,ng);
   }

   return;
}

//-----------------------------------------------------------------------------
// gh3d specific: evaluates a 3D or 2D axisymmetric function on an
// un-compactified sphere of radius R 
//
// NOTE: assumes levels sorted from lowest res to highest res
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// gh3d specific: evaluates a 3D or 2D axisymmetric function on an
// un-compactified sphere of radius R ... option arguments specify
// a range of R ... resultant grid will be 1 dim higher then
//
// NOTE: assumes levels sorted from lowest res to highest res
//
// LR: commented out to replace with version that evaluates a 3D or 2D 
// axisymmetric function on a sphere of COMPACTIFIED radius R. Implemented below
//-----------------------------------------------------------------------------
//grid *gf_eval_r(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
//{
//   grid *g=it_a->g,*ret_g,*nxt_g;
//   grid *ng;
//   int i,j,k,ret,ind;
//   static time_str *ts;
//   static double R,*evalr,coords[6],R1,dR;
//   static int error,dim,numt,Ntheta,Nphi,Ntot,shape[3],ndim,nR;
//   double f,x0[3],theta,phi,R0;
//
//   ret_g=0;
//   if (first_call)
//   {
//      error=0;
//      ts=0;
//      dim=g->dim;
//      R1=0;
//      nR=1;
//      if (args && strlen(args)>0) sscanf(args,"%lf,%i,%i,%lf,%i", &R,&Ntheta,&Nphi,&R1,&nR);
//      else
//      {
//         printf("gf_eval_r: arguments required\n"); error=1; return 0;
//      }
//      ndim=dim;
//      if (R1>R && nR>1) { ndim=dim+1; dR=(R1-R)/(nR-1); } else { nR=1; R1=R; dR=1;}
//      shape[0]=Ntheta;
//      coords[0]=0;
//      coords[1]=M_PI;
//      switch(dim)
//      {
//         case 2: printf("gf_eval_r: evaluating gf at R=%lf, Ntheta=%i (R1=%lf, nR=%i) \n",R,Ntheta,R1,nR); 
//                 Ntot=Ntheta;
//                 if (ndim==3) { shape[1]=nR; coords[2]=R; coords[3]=R1; Ntot*=nR; }
//                 break;
//         case 3: printf("gf_eval_r: evaluating gf at R=%lf, Ntheta=%i,Nphi=%i (R1=%lf, nR=%i)\n",R,Ntheta,Nphi,R1,nR); 
//                 Ntot=Ntheta*Nphi;
//                 shape[1]=Nphi;
//                 coords[2]=0;
//                 coords[3]=2*M_PI;
//                 if (ndim==4) { shape[2]=nR; coords[4]=R; coords[5]=R1; Ntot*=nR;}
//                 break;
//      }
//      if (!(evalr=(double *)malloc(sizeof(double)*Ntot)))
//      {
//         printf("gf_eval_r: out of memory\n");
//         return 0;
//      }
//      for (i=0; i<Ntot; i++) evalr[i]=0;
//      ts=it_a->ts;
//   }
//
//   if (!evalr) return 0;
//
//   ng=0;
//
//   if (it_a->ts!=ts)
//   {
//      if (!(ng=galloc(ndim-1,COORD_UNIFORM,shape,ts->time,coords,0,evalr))) return 0;
//      for (i=0; i<Ntot; i++) evalr[i]=0;
//      ts=it_a->ts;
//   }
//
//   if (g->dim!=dim)
//   {
//      printf("gf_eval_r: error ... multiple grid dimensions in register\n");
//      printf("   -> grid of dimension %i will be dropped\n",g->dim);
//   }
//   else
//   {
//      for (R0=R,k=0; k<nR; R0+=dR,k++) switch(dim)
//      {
//         case 2: 
//            for (i=0; i<Ntheta; i++)
//            {
//               theta=i*M_PI/(Ntheta-1);
//               x0[0]=2*atan(R0*cos(theta))/M_PI;
//               x0[1]=2*atan(R0*sin(theta))/M_PI;
//               ind=i+k*Ntheta;
//               ret=eval_2d(x0[0],x0[1],g,mask,mask_val,&evalr[ind]); 
//            }
//            break;
//         case 3: 
//            for (i=0; i<Ntheta; i++)
//            {
//               for (j=0; j<Nphi; j++)
//               {
//                  theta=i*M_PI/(Ntheta-1);
//                  phi=j*2*M_PI/(Nphi-1);
//                  ind=i+j*Ntheta+k*Ntheta*Nphi;
//                  x0[0]=2*atan(R0*cos(phi)*sin(theta))/M_PI;
//                  x0[1]=2*atan(R0*sin(phi)*sin(theta))/M_PI;
//                  x0[2]=2*atan(R0*cos(theta))/M_PI;
//                  ret=eval_3d(x0[0],x0[1],x0[2],g,mask,mask_val,&evalr[ind]); 
//               }
//            }
//            break;
//      }
//   }
//
//   ret=1;
//   save_s_iter(it_a);
//   nxt_g=next_g(it_a);
//   while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
//   restore_s_iter(it_a);
//   if (ret) 
//   { 
//      if (!ng) {if (!(ng=galloc(ndim-1,COORD_UNIFORM,shape,ts->time,coords,0,evalr))) return 0;}
//      free(evalr); 
//   }
//
//   return ng;
//}


//-----------------------------------------------------------------------------
// Implemented by LR
// gh3d specific: evaluates a 3D or 2D axisymmetric function on a
// a sphere of compactified radius R ... option arguments specify
// a range of R ... resultant grid will be 1 dim higher then
//
// NOTE: assumes levels sorted from lowest res to highest res
//-----------------------------------------------------------------------------
grid *gf_eval_r(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ret_g,*nxt_g;
   grid *ng;
   int i,j,k,ret,ind;
   static time_str *ts;
   static double R,*evalr,coords[6],R1,dR;
   static int error,dim,numt,Ntheta,Nphi,Ntot,shape[3],ndim,nR;
   double f,x0[3],theta,phi,R0;

   ret_g=0;
   if (first_call)
   {
      error=0;
      ts=0;
      dim=g->dim;
      R1=0;
      nR=1;
      if (args && strlen(args)>0) sscanf(args,"%lf,%i,%i,%lf,%i", &R,&Ntheta,&Nphi,&R1,&nR);
      else
      {
         printf("gf_eval_r: arguments required\n"); error=1; return 0;
      }
      ndim=dim;
      if (R1>R && nR>1) { ndim=dim+1; dR=(R1-R)/(nR-1); } else { nR=1; R1=R; dR=1;}
      shape[0]=Ntheta;
      coords[0]=0;
      coords[1]=M_PI;
      switch(dim)
      {
         case 2: printf("gf_eval_r: evaluating gf at compactified radius R=%lf, Ntheta=%i (R1=%lf, nR=%i) \n",R,Ntheta,R1,nR); 
                 Ntot=Ntheta;
                 if (ndim==3) { shape[1]=nR; coords[2]=R; coords[3]=R1; Ntot*=nR; }
                 break;
         case 3: printf("gf_eval_r: evaluating gf at compactified radius R=%lf, Ntheta=%i,Nphi=%i (R1=%lf, nR=%i)\n",R,Ntheta,Nphi,R1,nR); 
                 Ntot=Ntheta*Nphi;
                 shape[1]=Nphi;
                 coords[2]=0;
                 coords[3]=2*M_PI;
                 if (ndim==4) { shape[2]=nR; coords[4]=R; coords[5]=R1; Ntot*=nR;}
                 break;
      }
      if (!(evalr=(double *)malloc(sizeof(double)*Ntot)))
      {
         printf("gf_eval_r: out of memory\n");
         return 0;
      }
      for (i=0; i<Ntot; i++) evalr[i]=0;
      ts=it_a->ts;
   }

   if (!evalr) return 0;

   ng=0;

   if (it_a->ts!=ts)
   {
      if (!(ng=galloc(ndim-1,COORD_UNIFORM,shape,ts->time,coords,0,evalr))) return 0;
      for (i=0; i<Ntot; i++) evalr[i]=0;
      ts=it_a->ts;
   }

   if (g->dim!=dim)
   {
      printf("gf_eval_r: error ... multiple grid dimensions in register\n");
      printf("   -> grid of dimension %i will be dropped\n",g->dim);
   }
   else
   {
      for (R0=R,k=0; k<nR; R0+=dR,k++) switch(dim)
      {
         case 2: 
            for (i=0; i<Ntheta; i++)
            {
               theta=i*M_PI/(Ntheta-1);
               x0[0]=R0*cos(theta);
               x0[1]=R0*sin(theta);
               ind=i+k*Ntheta;
               ret=eval_2d(x0[0],x0[1],g,mask,mask_val,&evalr[ind]); 
            }
            break;
         case 3: 
            for (i=0; i<Ntheta; i++)
            {
               for (j=0; j<Nphi; j++)
               {
                  theta=i*M_PI/(Ntheta-1);
                  phi=j*2*M_PI/(Nphi-1);
                  ind=i+j*Ntheta+k*Ntheta*Nphi;
                  x0[0]=R0*cos(phi)*sin(theta);
                  x0[1]=R0*sin(phi)*sin(theta);
                  x0[2]=R0*cos(theta);
                  ret=eval_3d(x0[0],x0[1],x0[2],g,mask,mask_val,&evalr[ind]); 
               }
            }
            break;
      }
   }

   ret=1;
   save_s_iter(it_a);
   nxt_g=next_g(it_a);
   while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
   restore_s_iter(it_a);
   if (ret) 
   { 
      if (!ng) {if (!(ng=galloc(ndim-1,COORD_UNIFORM,shape,ts->time,coords,0,evalr))) return 0;}
      free(evalr); 
   }

   return ng;
}

//============================================================================= 
// takes a 1D function and prints the local maxima of it
//============================================================================= 
grid *gf_local_max(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   double tau,*tau_p,*f,p_max=0,dtau;
   int i;

   if (first_call)
   {
      printf("gf_local_max: reg=%s\n",it_a->r->name);
   }

   if (g->dim!=1)
   {
      printf("gf_ln_tps_m_tp: input grid must be a 1D grid");
      return 0;
   }

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;

   printf("t=%lf\n==========================\ntau\t\tf(tau)\n",g->time);

   if (g->coord_type==COORD_CURVILINEAR) tau_p=g->coords;
   else if (g->coord_type==COORD_SEMI_UNIFORM) tau_p=g->ccoords;
   else
   {
      tau_p=0;
      tau=g->coords[0];
      dtau=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   }

   f=g->data;
   tau+=dtau;
   ng->data[0]=p_max;
   for (i=1; i<(sizeof_data(g)-1); i++)
   {
      if (tau_p) tau=tau_p[i];
      if (f[i]>f[i-1] && f[i]>f[i+1])
      {
         printf("%lf\t%lf\n",tau,f[i]);
         p_max=f[i];
      }
      tau+=dtau;
      ng->data[i]=p_max;
   }
   ng->data[sizeof_data(g)-1]=p_max;

   return ng;
}

//============================================================================= 
// takes a 1D uniform function of (tp), and changes to coordinates
// -(ln(tp*-tp)).
//
// if (trace=0) prints extremum
// if (trace=1) prints x
// if (trace=-1) prints nothing
//
// if (search_n>0), then use the first search_n (skipping the first skip_n) 
// maxima to search for the 'best-fit' tp*, using the argument tp* as the initial guess
//============================================================================= 
grid *gf_ln_tps_m_tp(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   static double tps;
   int n,i,num_ext,k,sum_n;
   static int trace,search_n,skip_n;
   double *x,*tp,*ltp,lntp,dlntp,t_ext_m1,t_ext_m2,delta,fp,fpp;
   double norm_i[100],delta_n[100],tps_i[100];
   int max_iter,iter,max_iter0=10;

   if (first_call)
   {
      tps=0;
      trace=0;
      search_n=0;
      skip_n=0;
      if (args && strlen(args)>0) sscanf(args,"%lf,%i,%i,%i",&tps,&search_n,&skip_n,&trace);
      printf("gf_ln_tps_m_tp: reg=%s, tps=%lf, search_n=%i, skip_n=%i, trace=%i\n",it_a->r->name,tps,search_n,skip_n,trace);
   }

   if (search_n >0) max_iter=3*max_iter0; else max_iter=1;

   if (g->dim!=1 && g->coord_type!=COORD_CURVILINEAR)
   {
      printf("gf_ln_tps_m_tp: input grid must be a 1D, curvilinear grid");
      return 0;
   }

   if (trace==1) printf("-ln(%lf-tp)\t\t%s\n",tps,it_a->r->name);

   if (!(ng=galloc(g->dim,COORD_CURVILINEAR,g->shape,g->time,g->coords,0,g->data))) return 0;
   n=sizeof_coords(ng);
   x=ng->data;
   ltp=ng->coords;
   tp=g->coords;

   for(iter=0; iter<max_iter; iter++)
   {
      dlntp=1;
      lntp=0;
      t_ext_m1=t_ext_m2=0;
      num_ext=0;
      tps_i[iter]=tps;
      for(i=0; i<n; i++)
      {
         if (tp[i]>tps)
         {
            lntp=lntp+dlntp;
            ltp[i]=lntp;
         }
         else
         {
            ltp[i]=-log(tps-tp[i]);
            dlntp=ltp[i]-lntp;
            lntp=ltp[i];
         }
         if (i>0 && i<(n-1))
         {
            if (tp[i]<tps && ((x[i]>x[i-1] && x[i]>x[i+1]) || (x[i]<x[i-1] && x[i]<x[i+1])))
            {
               num_ext++;
               if (num_ext>2) delta=ltp[i]-t_ext_m2; else delta=0;
               if (trace==0 && !(iter % 3)) printf("local extrema at lntp=%lf : %lf \t Delta=%lf\n",ltp[i],x[i],delta);
               t_ext_m2=t_ext_m1;
               t_ext_m1=ltp[i];
               delta_n[num_ext-1]=delta;
            }
         }
         if (trace==1) printf("%lf\t\t%lf\n",ltp[i],x[i]);
      }
      if (num_ext < 4 || search_n < 4)
      {  
         if (search_n > 0) printf("number of extrema or search_n too small for a search\n");
         max_iter=1;
      }
      if (max_iter>1)
      {
         delta=0;
         sum_n=min(num_ext,search_n);
         for (k=2+skip_n; k<sum_n; k++) delta+=delta_n[k]/(sum_n-skip_n-2); // average
         norm_i[iter]=0;
         for (k=2+skip_n; k<sum_n; k++) norm_i[iter]+=pow(delta_n[k]-delta,2)/(sum_n-skip_n-2)/delta;

         // use the secant method to search for the maxima ... after iterations
         // 0,1, 3,4, 6,7 ...  perturb tp* by a bit to compute d(norm)/(dtp*)
         if ((iter % 3)==0)
         {
            if (iter==0) tps+=1.0e-8;
            else tps+=(tps_i[iter]-tps_i[iter-3])/100;
         }
         else if ((iter % 3)==1)
         {
            if (iter==1) tps-=2*1.0e-8;
            else tps-=2*(tps_i[iter-1]-tps_i[iter-4])/100;
         }
         else
         {
            // iter's:
            // 0 - tps         (iter-2)
            // 1 - tps + delta (iter-1)
            // 2 - tps - delta (iter)
            // ...
            fp=(norm_i[iter-1]-norm_i[iter])/(tps_i[iter-1]-tps_i[iter]);
            fpp=(norm_i[iter-1]+norm_i[iter]-2*norm_i[iter-2])/pow(tps_i[iter-1]-tps_i[iter-2],2);
            tps=tps_i[iter-2]-fp/fpp;
            if (trace==0)
                printf("\n iter %i \t tps=%14.10lf <delta>=%lf\n \t norm[-1]=%lf \t f'=%lf \t f''=%lf\n\n",
                                 iter,tps,delta,norm_i[iter-1],fp,fpp);
         }
      }
   }   
   return ng;
}

//=============================================================================
// 2:1 coarsen operation 
// designed for grids with odd-number of points in each dimension,
// FORTRAN style array indexing, and mask ignored ... grids with an even
// number of points will have their physical size reduced by 1 fine point
//=============================================================================
grid *gf_coarsen(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *ng,*g=it_a->g;
   double *c[MAX_DIM],*nc[MAX_DIM];
   double *a,*d,*m,dx;
   int i,j,k,n;
   int nshape[MAX_DIM];

   if (g->coord_type==COORD_CURVILINEAR && first_call)
   {
      printf("coarsen:: warning ... curvilinear grids not fully supported yet \n");
   }

   for (i=0;i<g->dim;i++)
   {
      nshape[i]=(g->shape[i]-1)/2+1;
      if (nshape[i]<1)
      {
         printf("coarsen:: grid too small to coarsen\n");
         return 0;
      }
   }

   if (!(ng=galloc(g->dim,g->coord_type,nshape,g->time,g->coords,0,0))) return 0;

   for (i=0;i<g->dim;i++) 
   {
      if (g->coord_type==COORD_CURVILINEAR) 
      {
         c[i]=&g->coords[i*sizeof_data(g)]; 
         nc[i]=&ng->coords[i*sizeof_data(ng)];
         dx=(c[i])[1]-(c[i])[0];
      }
      else if (g->coord_type==COORD_SEMI_UNIFORM)
      {
         c[i]=&g->ccoords[i*sizeof_data(g)]; 
         nc[i]=&ng->ccoords[i*sizeof_data(ng)];
         dx=(g->coords[2*i+1]-g->coords[2*i])/(g->shape[i]-1);
      }
      else
      {
         c[i]=nc[i]=0;
         dx=(g->coords[2*i+1]-g->coords[2*i])/(g->shape[i]-1);
      }
      if (((ng->shape[i]-1)*2+1)!=g->shape[i]) ng->coords[2*i+1]-=dx;
   }

   switch(g->dim)
   {
      case 1:
         for (i=0; i<ng->shape[0]; i++) 
         {
            ng->data[i]=g->data[2*i];
            if (nc[0]) (nc[0])[i]=(c[0])[2*i];
         }
         break;
      case 2:
         for (i=0; i<ng->shape[0]; i++)
           for (j=0; j<ng->shape[1]; j++)
           {
              ng->data[i+j*ng->shape[0]]=g->data[2*i+2*j*g->shape[0]];
              if (nc[0]) (nc[0])[i+j*ng->shape[0]]=(c[0])[2*i+2*j*g->shape[0]];
              if (nc[1]) (nc[1])[i+j*ng->shape[0]]=(c[1])[2*i+2*j*g->shape[0]];
           }
         break;
      case 3:
         for (i=0; i<ng->shape[0]; i++)
           for (j=0; j<ng->shape[1]; j++)
              for (k=0; k<ng->shape[2]; k++)
              {
                 ng->data[i+ng->shape[0]*(j+k*ng->shape[1])]=
                  g->data[2*(i+g->shape[0]*(j+k*g->shape[1]))];
                 if (nc[0]) (nc[0])[i+ng->shape[0]*(j+k*ng->shape[1])]=(c[0])[2*(i+g->shape[0]*(j+k*g->shape[1]))];
                 if (nc[1]) (nc[1])[i+ng->shape[0]*(j+k*ng->shape[1])]=(c[1])[2*(i+g->shape[0]*(j+k*g->shape[1]))];
                 if (nc[2]) (nc[2])[i+ng->shape[0]*(j+k*ng->shape[1])]=(c[2])[2*(i+g->shape[0]*(j+k*g->shape[1]))];
              }
         break;
      default:
         gfree(ng);
         printf("coarsen:: dimension %i not yet supported\n",g->dim);
         return 0;
   }
   return ng;
}
   
//=============================================================================
// bounds an array by [min,max], arg='min,max'
//=============================================================================
grid *gf_bound(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *ng,*g=it_a->g;
   double *p,*q,*m;
   float fmin,fmax;
   static double min,max;
   int i,n;

   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;

   if (first_call) 
   { 
      if (args && strlen(args))
      {
         sscanf(args,"%f,%f",&fmin,&fmax);
         min=fmin;
         max=fmax;
      }
      else
      {
         min=-1;
         max=1;
      }
      printf("gf_bound: min=%f, max=%f\n",min,max);
   }


   p=ng->data;
   q=g->data;
   n=sizeof_data(ng);
   if (mask)
   {
      m=mask->data;
      for (; n; n--,p++,m++,q++)
         if (*m==mask_val) *p=0;
         else
         {
            if (*q<=max && *q>=min) *p=*q; // test first to weed out nan's
            else if (*q<=min) *p=min;
            else *p=max;
         }
   }
   else
   {
      for (; n; n--,p++,q++)
      {
         if (*q<=max && *q>=min) *p=*q; // test first to weed out nan's
         else if (*q<=min) *p=min;
         else *p=max;
      }
   }

   return ng;
}

//=============================================================================
// rho:1 interpolation operation 
//=============================================================================
void dmi4q1_(double *uf,double *uc,int *nxc,int *nyc,int *nxf,int *nyf,
               int *if_co,int *jf_co,int *irho);
grid *gf_2dinterpolate(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *ng,*g=it_a->g;
   int nshape[MAX_DIM];
   static double rhod;
   int rho,one=1;

   if (g->coord_type!=COORD_UNIFORM || g->dim!=2)
   {
      printf("coarsen:: only 2D uniform grids supported\n");
      return 0;
   }

   get_single_farg(&rhod,args,first_call,2.0,"gf_2dinterpolate");
   rho=rhod;
   if (rho<2) {printf("gf_2dinterpolate: fine:coarse ratio must be >=2\n"); return 0;}

   nshape[0]=(g->shape[0]-1)*rho+1;
   nshape[1]=(g->shape[1]-1)*rho+1;

   if (!(ng=galloc(g->dim,g->coord_type,nshape,g->time,g->coords,g->ccoords,0))) return 0;

   dmi4q1_(ng->data,g->data,&g->shape[0],&g->shape[1],&ng->shape[0],&ng->shape[1],
             &one,&one,&rho);

   return ng;
}
   
//=============================================================================
// gf_to_uniform_unigrid converts a register to a uniform, unigrid structure.
// 
// currently oworks for 1D,2D and 3D grids, but only tested for uniform 3D grids, 
// and interpolation is linear
//
// argument is 'x1,x2,Nx,y1,y2,Ny,z1,z2,Nz'
//
// !NOTE: This function currently works by assuming that the grids are 
// passed in order of ascending level!!
//
// eval_1d and eval_2d are the interpolation functions used by 
// gf_to_uniform_unigrid (return 0 if point is outside of the domain of the
// grid (or masked), otherwise 1 ; f is the result)
// They both assume that the grids are 1d and 2d, as appropriate
//
// R_INT_FUZZ set to 2, to extrapolate half a cell width off the domain if
// needed.
//=============================================================================
#define R_INT_FUZZ 2.0e0
int eval_1d(double x,grid *g,grid *mask,double mask_val, double *f) 
{
   int ret=0,i,N,di;
   double dx,frac,fuzz,fuzzn,dxn,*xc;

   N=g->shape[0];
   if (N<2) return 0;
   switch(g->coord_type)
   {
      case COORD_UNIFORM: 
         dx=(g->coords[1]-g->coords[0])/(N-1);
         if (dx<0) di=1; else di=0;
         fuzz=dx/R_INT_FUZZ;
         if (!(fuzz_lt(x,g->coords[0+di],fuzz) || fuzz_gt(x,g->coords[1-di],fuzz)))
         {
            i=(x-g->coords[0])/dx;
            if (i>=(N-1)) i=N-2; if (i<0) i=0;
            frac=(x-(i*dx+g->coords[0]))/dx;
            if (!mask || (mask->data[i]!=mask_val && mask->data[i+1]!=mask_val))
            {
               *f=frac*g->data[i+1]+(1-frac)*g->data[i];
               ret=1;
            }
         }
         break;
      case COORD_CURVILINEAR: case COORD_SEMI_UNIFORM:
         if (g->coord_type==COORD_CURVILINEAR)
            xc=&g->coords[0]; 
         else
            xc=&g->ccoords[0]; 
         dx=xc[1]-xc[0];
         if (dx<0) di=N-1; else di=0;
         dxn=(xc[N-1]-xc[N-2]);
         fuzz=dx/R_INT_FUZZ;
         fuzzn=dxn/R_INT_FUZZ;
         if (!(fuzz_lt(x,xc[0+di],fuzz) || fuzz_gt(x,xc[N-1-di],fuzzn)))
         {
            i=1;
            while(x>xc[i]) i++;
            if (i>(N-1)) i=N-1;
            if (!mask || (mask->data[i]!=mask_val && mask->data[i-1]!=mask_val))
            {
               frac=(x-xc[i-1])/(xc[i]-xc[i-1]);
               *f=frac*g->data[i]+(1-frac)*g->data[i-1];
               ret=1;
            }
         }
         break;
      default: printf("eval_1d: error ... unsupported coordinate type %i\n",g->coord_type); ret=0;
   }

   return ret;
}

int eval_2d(double x,double y,grid *g,grid *mask,double mask_val, double *f) 
{
   int ret=0,i,j,Nx,Ny,n,di,dj;
   double dx,dy,frac_x,frac_y;
   double *xc,*yc,fuzzx,fuzzy;
   int ltrace=0;

   Nx=g->shape[0]; Ny=g->shape[1];
   if (Nx<2 || Ny<2) return 0;
   n=Nx*Ny;
   switch(g->coord_type)
   {
      case COORD_UNIFORM: 
         IFL printf("eval_2d: uniform case; x,y=%lf,%lf,  bbox=[%lf,%lf,%lf,%lf]\n",
                    x,y,g->coords[0],g->coords[1],g->coords[2],g->coords[3]);
         dx=(g->coords[1]-g->coords[0])/(Nx-1);
         dy=(g->coords[3]-g->coords[2])/(Ny-1);
         if (dx<0) di=1; else di=0;
         if (dy<0) dj=1; else dj=0;
         fuzzx=dx/R_INT_FUZZ;
         fuzzy=dy/R_INT_FUZZ;
         if (!(fuzz_lt(x,g->coords[0+di],fuzzx) || fuzz_gt(x,g->coords[1-di],fuzzx) || 
               fuzz_lt(y,g->coords[2+dj],fuzzy) || fuzz_gt(y,g->coords[3-dj],fuzzy)))
         {
            i=(x-g->coords[0])/dx;
            if (i>=(Nx-1)) i=Nx-2; if (i<0) i=0;
            j=(y-g->coords[2])/dy;
            if (j>=(Ny-1)) j=Ny-2; if (j<0) j=0;
            frac_x=(x-(i*dx+g->coords[0]))/dx;
            frac_y=(y-(j*dy+g->coords[2]))/dy;
            if (!mask || (mask->data[i+j*Nx]!=mask_val && 
                          mask->data[i+1+j*Nx]!=mask_val &&
                          mask->data[i+(j+1)*Nx]!=mask_val &&
                          mask->data[i+1+(j+1)*Nx]!=mask_val))
            {
               *f=(1-frac_x)*(1-frac_y)*g->data[i+j*Nx]+(  frac_x)*(1-frac_y)*g->data[i+1+j*Nx]+
                  (1-frac_x)*(  frac_y)*g->data[i+(j+1)*Nx]+(  frac_x)*(  frac_y)*g->data[i+1+(j+1)*Nx];
               ret=1;
               IFL printf("eval_2d: i,j,frac_x,frac_y=%i,%i,%lf,%lf\n",i,j,frac_x,frac_y);
            }
         }
         break;
      //
      // NOTE!!: in the following case, the total gf_to_uniform_unigrid algorithm
      // becomes an order Ns*Nd algorithm! (compared to O(Nd) for unigrid source),
      // where Ns=number for points in source, Nd=number of points in
      // destination. If needed, could implement a bisection-type search
      // strategy to convert to a ln(Ns)*Nd algorithm.
      //
      case COORD_CURVILINEAR: case COORD_SEMI_UNIFORM:
         IFL printf("eval_2d: non-uniform case\n");
         if (g->coord_type==COORD_CURVILINEAR)
         {
            xc=&g->coords[0]; yc=&g->coords[n];
         }
         else
         {
            xc=&g->ccoords[0]; yc=&g->ccoords[n];
         }
         for (i=0; i<(Nx-1); i++)
            for (j=0; j<(Ny-1); j++)
            {
               if (!mask || (mask->data[i+j*Nx]!=mask_val &&
                             mask->data[i+1+j*Nx]!=mask_val &&
                             mask->data[i+(j+1)*Nx]!=mask_val &&
                             mask->data[i+1+(j+1)*Nx]!=mask_val))
               {
                  frac_x=(x-xc[i+j*Nx])/(xc[i+1+j*Nx]-xc[i+j*Nx]);
                  // non-uniform stuff does not seem to work well with any
                  // extrapolation
                  // if (frac_x>=(-1/R_INT_FUZZ) && frac_x<=(1+1/R_INT_FUZZ))
                  if (frac_x>=(0) && frac_x<=(1))
                  {
                     frac_y=(y-yc[i+j*Nx])/(yc[i+(j+1)*Nx]-yc[i+j*Nx]);
                     // if (frac_y>=(-1/R_INT_FUZZ) && frac_y<=(1+1/R_INT_FUZZ))
                     if (frac_y>=(0) && frac_y<=(1))
                     {
                        *f=(1-frac_x)*(1-frac_y)*g->data[i+j*Nx]+(  frac_x)*(1-frac_y)*g->data[i+1+j*Nx]+
                           (1-frac_x)*(  frac_y)*g->data[i+(j+1)*Nx]+(  frac_x)*(  frac_y)*g->data[i+1+(j+1)*Nx];
                        ret=1;
                        // don't return if we extrapolate, for only if at the end
                        // that's the best we can do, we use the extrapolated value
                        if (frac_x>=0 && frac_x<=1 && frac_y>=0 && frac_y<=1) return 0;
                     }
                  }
               }
            }
         break;
      default: printf("eval_1d: error ... unsupported coordinate type %i\n",g->coord_type); ret=0;
   }

   return ret;
}

int eval_3d(double x,double y,double z,grid *g,grid *mask,double mask_val,double *f) 
{
   int ret=0,i,j,k,Nx,Ny,Nz,n,di,dj,dk;
   double dx,dy,dz,frac_x,frac_y,frac_z;
   double *xc,*yc,*zc,fuzzx,fuzzy,fuzzz;
   int ltrace=0;

   Nx=g->shape[0]; Ny=g->shape[1]; Nz=g->shape[2];
   if (Nx<2 || Ny<2 || Nz<2) return 0;
   n=Nx*Ny*Nz;
   switch(g->coord_type)
   {
      case COORD_UNIFORM: 
         IFL printf("eval_3d: uniform case; x,y,z=%lf,%lf,%lf  bbox=[%lf,%lf,%lf,%lf,%lf,%lf]\n",
                    x,y,z,g->coords[0],g->coords[1],g->coords[2],g->coords[3],
                    g->coords[4],g->coords[5]);
         dx=(g->coords[1]-g->coords[0])/(Nx-1);
         dy=(g->coords[3]-g->coords[2])/(Ny-1);
         dz=(g->coords[5]-g->coords[4])/(Nz-1);
         if (dx<0) di=1; else di=0;
         if (dy<0) dj=1; else dj=0;
         if (dz<0) dk=1; else dk=0;
         fuzzx=dx/R_INT_FUZZ;
         fuzzy=dy/R_INT_FUZZ;
         fuzzz=dz/R_INT_FUZZ;
         if (!(fuzz_lt(x,g->coords[0+di],fuzzx) || fuzz_gt(x,g->coords[1-di],fuzzx) || 
               fuzz_lt(y,g->coords[2+dj],fuzzy) || fuzz_gt(y,g->coords[3-dj],fuzzy) || 
               fuzz_lt(z,g->coords[4+dk],fuzzy) || fuzz_gt(z,g->coords[5-dk],fuzzy)))
         {
            i=(x-g->coords[0])/dx;
            if (i>=(Nx-1)) i=Nx-2; if (i<0) i=0;
            j=(y-g->coords[2])/dy;
            if (j>=(Ny-1)) j=Ny-2; if (j<0) j=0;
            k=(z-g->coords[4])/dz;
            if (k>=(Nz-1)) k=Nz-2; if (k<0) k=0;
            frac_x=(x-(i*dx+g->coords[0]))/dx;
            frac_y=(y-(j*dy+g->coords[2]))/dy;
            frac_z=(z-(k*dz+g->coords[4]))/dz;
            if (!mask || (mask->data[i+j*Nx+k*Nx*Ny]!=mask_val && 
                          mask->data[i+1+j*Nx+k*Nx*Ny]!=mask_val &&
                          mask->data[i+(j+1)*Nx+k*Nx*Ny]!=mask_val &&
                          mask->data[i+1+(j+1)*Nx+k*Nx*Ny]!=mask_val &&
                          mask->data[i+j*Nx+(k+1)*Nx*Ny]!=mask_val && 
                          mask->data[i+1+j*Nx+(k+1)*Nx*Ny]!=mask_val &&
                          mask->data[i+(j+1)*Nx+(k+1)*Nx*Ny]!=mask_val &&
                          mask->data[i+1+(j+1)*Nx+(k+1)*Nx*Ny]!=mask_val))
            {
               *f=(1-frac_x)*(1-frac_y)*(1-frac_z)*g->data[i+j*Nx+k*Nx*Ny]+
                  (  frac_x)*(1-frac_y)*(1-frac_z)*g->data[i+1+j*Nx+k*Nx*Ny]+
                  (1-frac_x)*(  frac_y)*(1-frac_z)*g->data[i+(j+1)*Nx+k*Nx*Ny]+
                  (  frac_x)*(  frac_y)*(1-frac_z)*g->data[i+1+(j+1)*Nx+k*Nx*Ny]+
                  (1-frac_x)*(1-frac_y)*(  frac_z)*g->data[i+j*Nx+(k+1)*Nx*Ny]+
                  (  frac_x)*(1-frac_y)*(  frac_z)*g->data[i+1+j*Nx+(k+1)*Nx*Ny]+
                  (1-frac_x)*(  frac_y)*(  frac_z)*g->data[i+(j+1)*Nx+(k+1)*Nx*Ny]+
                  (  frac_x)*(  frac_y)*(  frac_z)*g->data[i+1+(j+1)*Nx+(k+1)*Nx*Ny];
               ret=1;
               IFL printf("eval_3d: i,j,k,frac_x,frac_y,frac_z=%i,%i,%i,%lf,%lf,%lf\n",i,j,k,frac_x,frac_y,frac_z);
            }
         }
         break;
      //
      // NOTE!!: in the following case, the total gf_to_uniform_unigrid algorithm
      // becomes an order Ns*Nd algorithm! (compared to O(Nd) for unigrid source),
      // where Ns=number for points in source, Nd=number of points in
      // destination. If needed, could implement a bisection-type search
      // strategy to convert to a ln(Ns)*Nd algorithm.
      //
      case COORD_CURVILINEAR: case COORD_SEMI_UNIFORM:
         IFL printf("eval_2d: non-uniform case\n");
         if (g->coord_type==COORD_CURVILINEAR)
         {
            xc=&g->coords[0]; yc=&g->coords[n]; zc=&g->coords[2*n];
         }
         else
         {
            xc=&g->ccoords[0]; yc=&g->ccoords[n]; zc=&g->ccoords[2*n];
         }
         for (i=0; i<(Nx-1); i++)
            for (j=0; j<(Ny-1); j++)
               for (k=0; k<(Nz-1); k++)
               {
                  if (!mask || (mask->data[i+j*Nx+k*Nx*Ny]!=mask_val && 
                                mask->data[i+1+j*Nx+k*Nx*Ny]!=mask_val &&
                                mask->data[i+(j+1)*Nx+k*Nx*Ny]!=mask_val &&
                                mask->data[i+1+(j+1)*Nx+k*Nx*Ny]!=mask_val &&
                                mask->data[i+j*Nx+(k+1)*Nx*Ny]!=mask_val && 
                                mask->data[i+1+j*Nx+(k+1)*Nx*Ny]!=mask_val &&
                                mask->data[i+(j+1)*Nx+(k+1)*Nx*Ny]!=mask_val &&
                                mask->data[i+1+(j+1)*Nx+(k+1)*Nx*Ny]!=mask_val))
                  {
                     frac_x=(x-xc[i+j*Nx+k*Nx*Ny])/(xc[i+1+j*Nx+k*Nx*Ny]-xc[i+j*Nx+k*Nx*Ny]);
                     // non-uniform stuff does not seem to work well with any
                     // extrapolation
                     // if (frac_x>=(-1/R_INT_FUZZ) && frac_x<=(1+1/R_INT_FUZZ))
                     if (frac_x>=(0) && frac_x<=(1))
                     {
                        frac_y=(y-yc[i+j*Nx+k*Nx*Ny])/(yc[i+(j+1)*Nx+k*Nx*Ny]-yc[i+j*Nx+k*Nx*Ny]);
                        // if (frac_y>=(-1/R_INT_FUZZ) && frac_y<=(1+1/R_INT_FUZZ))
                        if (frac_y>=(0) && frac_y<=(1))
                        {
                           frac_z=(z-zc[i+j*Nx+k*Nx*Ny])/(zc[i+j*Nx+(k+1)*Nx*Ny]-zc[i+j*Nx+k*Nx*Ny]);
                           // if (frac_z>=(-1/R_INT_FUZZ) && frac_z<=(1+1/R_INT_FUZZ))
                           if (frac_z>=(0) && frac_z<=(1))
                           {
                              *f=(1-frac_x)*(1-frac_y)*(1-frac_z)*g->data[i+j*Nx+k*Nx*Ny]+
                                 (  frac_x)*(1-frac_y)*(1-frac_z)*g->data[i+1+j*Nx+k*Nx*Ny]+
                                 (1-frac_x)*(  frac_y)*(1-frac_z)*g->data[i+(j+1)*Nx+k*Nx*Ny]+
                                 (  frac_x)*(  frac_y)*(1-frac_z)*g->data[i+1+(j+1)*Nx+k*Nx*Ny]+
                                 (1-frac_x)*(1-frac_y)*(  frac_z)*g->data[i+j*Nx+(k+1)*Nx*Ny]+
                                 (  frac_x)*(1-frac_y)*(  frac_z)*g->data[i+1+j*Nx+(k+1)*Nx*Ny]+
                                 (1-frac_x)*(  frac_y)*(  frac_z)*g->data[i+(j+1)*Nx+(k+1)*Nx*Ny]+
                                 (  frac_x)*(  frac_y)*(  frac_z)*g->data[i+1+(j+1)*Nx+(k+1)*Nx*Ny];
                              ret=1;
                              // don't return if we extrapolate, for only if at the end
                              // that's the best we can do, we use the extrapolated value
                              if (frac_x>=0 && frac_x<=1 && frac_y>=0 && frac_y<=1 && frac_z>=0 && frac_z<=1) return 0;
                           }
                        }
                     }
                  }
               }
         break;
      default: printf("eval_3d: error ... unsupported coordinate type %i\n",g->coord_type); ret=0;
   }

   return ret;
}

grid *gf_to_uniform_unigrid(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ret_g,*nxt_g;
   static grid *ng;
   int i,j,k,n,ret;
   static time_str *curr_ts;
   static double bb[6],dx[3];
   static int sh[3],error,dim;
   double x,y,z,f;
   
   ret_g=0;
   if (first_call)
   {
      error=0;
      curr_ts=0;
      ng=0;
      dim=g->dim;
      sh[0]=sh[1]=sh[2]=1;
      if (args && strlen(args)>0) 
         sscanf(args,"%lf,%lf,%i,%lf,%lf,%i,%lf,%lf,%i",
                &bb[0],&bb[1],&sh[0],&bb[2],&bb[3],&sh[1],&bb[4],&bb[5],&sh[2]);
      else
      {
         printf("gf_to_uniform_unigrid: arguments required\n"); error=1; return 0;
      }
      if (sh[0]<2 || (dim>1 && sh[1]<2) || (dim>2 && sh[2]<2)) 
      {
         printf("gf_to_uniform_unigrid: grid size must be >=2 ... Nx=%i,Ny=%i,Nz=%i\n",sh[0],sh[1],sh[2]); 
         error=1; return 0;
      }
      switch(dim)
      {
         case 1: printf("gf_to_uniform_unigrid: creating a grid of size %i, with [x]=[%lf,%lf]\n",
                 sh[0],bb[0],bb[0]); break;
         case 2: printf("gf_to_uniform_unigrid: creating a grid of size %ix%i, with [x,y]=[%lf,%lf,%lf,%lf]\n",
                 sh[0],sh[1],bb[0],bb[1],bb[2],bb[3]); break;
         case 3: printf("gf_to_uniform_unigrid: creating a grid of size %ix%ix%i,"
                 " with [x,y,z]=[%lf,%lf,%lf,%lf,%lf,%lf]\n",
                 sh[0],sh[1],sh[2],bb[0],bb[1],bb[2],bb[3],bb[4],bb[5]); break;
      }
      dx[0]=(bb[1]-bb[0])/(sh[0]-1);
      dx[1]=(bb[3]-bb[2])/(sh[1]-1);
      dx[2]=(bb[5]-bb[4])/(sh[2]-1);
   }
   else if(error) { if(ng) gfree(ng); ng=0; return 0; }

   if (!ng)
   {
      if (dim>=2) printf("allocating uniform grid at t=%lf\n",g->time);
      if (!(ng=galloc(dim,COORD_UNIFORM,sh,g->time,bb,0,0)))
      {
         printf("gf_to_uniform_unigrid: error allocating grid\n"); error=1; return 0;
      }
      n=sizeof_data(ng); i=0;
      while(i<n) ng->data[i++]=0;
   }
   curr_ts=it_a->ts;

   if (g->dim!=dim)
   {
      printf("gf_to_uniform_unigrid: error ... multiple grid dimensions in register\n");
      printf("   -> grid of dimension %i will be dropped\n",g->dim);
   }
   else
   {
      switch(dim)
      {
         case 1: 
            for (x=bb[0],i=0; i<sh[0]; i++, x+=dx[0])
            {
               ret=eval_1d(x,g,mask,mask_val,&f); 
               if (ret) ng->data[i]=f;
            }
            break;
         case 2: 
            printf("   scanning in grid %ix%i ...\n",g->shape[0],g->shape[1]);
            for (x=bb[0],i=0; i<sh[0]; i++, x+=dx[0])
            {
               for (y=bb[2],j=0; j<sh[1]; j++, y+=dx[1])
               {
                  ret=eval_2d(x,y,g,mask,mask_val,&f); 
                  if (ret) ng->data[i+j*sh[0]]=f;
               }
            }
            break;
         case 3: 
            printf("   scanning in grid %ix%ix%i ...\n",g->shape[0],g->shape[1],g->shape[2]);
            for (x=bb[0],i=0; i<sh[0]; i++, x+=dx[0])
            {
               for (y=bb[2],j=0; j<sh[1]; j++, y+=dx[1])
               {
                  for (z=bb[4],k=0; k<sh[2]; k++, z+=dx[2])
                  {
                     ret=eval_3d(x,y,z,g,mask,mask_val,&f); 
                     if (ret) ng->data[i+j*sh[0]+k*sh[0]*sh[1]]=f;
                  }
               }
            }
            break;
      }
   }
          
   save_s_iter(it_a);
   nxt_g=next_g(it_a);
   if (!nxt_g || it_a->ts!=curr_ts) { ret_g=ng; ng=0; }
   restore_s_iter(it_a);

   return ret_g;
}

//-----------------------------------------------------------------------------
// evaluates a function at a point over time.
//-----------------------------------------------------------------------------

grid *gf_eval(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*nxt_g;
   grid *ng;
   int i,j,k,n,ret;
   static time_str *ts;
   static double x0[3],x1[3],*evalr,*t,dx[3],bbox[2];
   static int error,dim,numt,ct,l_eval,N;
   static grid *new_g;
   double f,pt;
   double xf[3];
   
   ng=0;
   if (first_call)
   {
      error=0;
      ts=it_a->ts;
      dim=g->dim;
      N=1;
      new_g=0;
      if (args && strlen(args)>0) sscanf(args,"%lf,%lf,%lf,%lf,%lf,%lf,%i", &x0[0],&x0[1],&x0[2],&x1[0],&x1[1],&x1[2],&N);
      else
      {
         printf("gf_eval: arguments required\n"); error=1; return 0;
      }
      if (N==1) 
      {
         switch(dim)
         {
            case 1: printf("gf_eval: evaluating gf at x=%lf\n",x0[0]); break;
            case 2: printf("gf_eval: evaluating gf at x=[%lf,%lf]\n",x0[0],x0[1]); break;
            case 3: printf("gf_eval: evaluating gf at x=[%lf,%lf,%lf]\n",x0[0],x0[1],x0[2]); break;
         }
         save_s_iter(it_a);
         nxt_g=g;
         pt=it_a->ts->time;
         numt=1;
         while(nxt_g) 
         {
            if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
            {
               numt++;
               pt=it_a->ts->time;
            } 
            nxt_g=next_g(it_a);
         }
         restore_s_iter(it_a);
         if (!(evalr=(double *)malloc(sizeof(double)*numt)) ||
             !(t=(double *)malloc(sizeof(double)*numt)) )
         {
            printf("gf_eval: out of memory\n");
            error=1; return 0;
         }
         l_eval=0;
         ct=0;
         evalr[ct]=0;
         t[ct]=ts->time;
      }
      else if (N>1)
      {
         printf("gf_eval: evaluating gf at %i points along a line between ",N);
         switch(dim)
         {
            case 1: printf("%lf and %lf\n",x0[0],x1[0]); 
                    bbox[0]=x0[0]; bbox[1]=x1[0];
                    printf("gf_eval: evaluating along a line with dim=1 not tested. Use uniform/unigrid function instead\n");
                           error=1; return 0;
                    break;
            case 2: printf("[%lf,%lf] and [%lf,%lf]\n",x0[0],x0[1],x1[0],x1[1]); 
                    bbox[0]=0; bbox[1]=sqrt(pow(x1[0]-x0[0],2)+pow(x1[1]-x0[1],2));
                    printf("Returning a register with bbox=[%lf,%lf]\n",bbox[0],bbox[1]);
                    break;
            case 3: printf("[%lf,%lf,%lf] and [%lf,%lf,%lf]\n",x0[0],x0[1],x0[2],x1[0],x1[1],x1[2]); 
                    bbox[0]=0; bbox[1]=sqrt(pow(x1[0]-x0[0],2)+pow(x1[1]-x0[1],2)+pow(x1[2]-x0[2],2));
                    printf("Returning a register with bbox=[%lf,%lf]\n",bbox[0],bbox[1]);
                    printf("gf_eval: evaluating along a line with dim=3 not tested. Comment out this line"
                           " in the code, recompile, and proceed with skepticism\n"); error=1; return 0;
                    break;
         }
         dx[0]=(x1[0]-x0[0])/(N-1);
         dx[1]=(x1[1]-x0[1])/(N-1);
         dx[2]=(x1[2]-x0[2])/(N-1);
         if (!(new_g=galloc(1,COORD_UNIFORM,&N,ts->time,bbox,0,0))) { error=1; return 0; }
         ng=new_g;
      }
      else
      {
         printf("gf_eval: N must be >=1\n"); error=1; return 0;
      } 
   }

   if (error) return 0;

   if (it_a->ts!=ts)
   {
      if (N==1) printf("%f\t%e (l=%i)\n",ts->time,evalr[ct],l_eval);
      else printf("t=%f\n",ts->time);
      ts=it_a->ts;
      ct++; 
      if (N==1) { evalr[ct]=0; t[ct]=ts->time; }
      else 
      {
         if (!(new_g=galloc(1,COORD_UNIFORM,&N,ts->time,bbox,0,0))) {error=1; return 0;}
         ng=new_g;
      }
   }

   if (g->dim!=dim)
   {
      printf("gf_eval: error ... multiple grid dimensions in register\n");
      printf("   -> grid of dimension %i will be dropped\n",g->dim);
   }
   else
   {
      for (n=0, xf[0]=x0[0], xf[1]=x0[1], xf[2]=x0[2]; n<N; n++, xf[0]+=dx[0], xf[1]+=dx[1],xf[2]+=dx[2]) 
      {
         ret=0;
         switch(dim)
         {
            case 1: 
               ret=eval_1d(xf[0],g,mask,mask_val,&f); break;
            case 2: 
               ret=eval_2d(xf[0],xf[1],g,mask,mask_val,&f); break;
            case 3: 
               ret=eval_3d(xf[0],xf[1],xf[2],g,mask,mask_val,&f); break;
         }
         if (ret) 
         { 
            if (N==1) { evalr[ct]=f; t[ct]=ts->time; l_eval=it_a->l->ln;} 
            else new_g->data[n]=f;
         }
      }
   }

   if (N==1)
   {
      ret=1;
      save_s_iter(it_a);
      nxt_g=next_g(it_a);
      while(nxt_g && ret) { if (it_a->selected!=GIV_OFF) ret=0; nxt_g=next_g(it_a); }
      restore_s_iter(it_a);
      if (ret)
      {
         if (!(ng=galloc(1,COORD_CURVILINEAR,&numt,0.0,t,0,evalr))) return 0;
         printf("%f\t%e (l=%i)\n",t[ct],evalr[ct],l_eval);
         free(evalr);
         free(t);
      }
   }

   return ng;
}

//=============================================================================
// differentiates register f wrt time, using an optional mask.
// currently first order accurate.
//
// Note: this is not a function implemented using the apply...gf() scheme.
//
// returns 1 on success/ 0 on failure
//=============================================================================
int t_diff(char *f, char *mask, double mask_val)
{
   int x,n,i,ret=0;
   reg *rf,*rmask;
   s_iter it_t,it_tp1,it_mt,it_mtp1;
   grid *g_t,*g_tp1,*gm_t,*gm_tp1;
   double h;

   if (mask && !(strlen(mask))) mask=0;

   if (mask && !strcmp(f,mask))
   {
      printf ("t_diff:: register and mask register must be different --- %s\n",f);
      return 0;
   }
   
   if (!(rf=find_reg(f,1,!mask)))
   {
      printf ("t_diff:: cannot find register %s\n",f);
      if (mask) FREE_REG_LIST;
      return 0;
   }

   if (mask)
   {
      if (!(rmask=find_reg(mask,0,1)))
      {
         printf ("t_diff:: cannot find register %s\n",mask);
         FREE_REG(rf);
         return 0;
      }
   }
   else rmask=0;

   if (mask && !(grid_str_eq(rf,rmask)))
   {
      printf ("t_diff:: register %s and mask %s do not have the same grid structure\n",f,mask);
      goto cleanup;
   }
      
   if (grid_str_eq(rf,rf)!=2)
   {
      printf ("diff:: register %s does not have the same grid structure in time; cannot compute d/dt\n",f);
      goto cleanup;
   }

   g_t=init_s_iter(&it_t,rf,0);   
   init_s_iter(&it_tp1,rf,0);
   if (!(next_ts(&it_tp1)))
   {
      printf("diff:: need at least 2 times in %s to differentiate in t\n",f);
      goto cleanup;
   }
   g_tp1=it_tp1.g;
   if (mask) 
   {
      gm_t=init_s_iter(&it_mt,rmask,0); 
      init_s_iter(&it_mtp1,rmask,0);
      next_ts(&it_mtp1);
      gm_tp1=it_mtp1.g;
   }
   while(g_tp1)
   {
      h=it_tp1.ts->time-it_t.ts->time;
      for(n=1,i=0;i<g_t->dim;i++) n*=g_t->shape[i];
      for(i=0;i<n;i++)
      {
         if (mask && ((gm_tp1 && gm_tp1->data[i]==mask_val) ||
                      (gm_t && gm_t->data[i]==mask_val)))
            g_t->data[i]=0;
         else
            g_t->data[i]=(g_tp1->data[i]-g_t->data[i])/h;

         if (!(it_tp1.ts->next)) g_tp1->data[i]=g_t->data[i];
      }
      g_t=next_g(&it_t);
      g_tp1=next_g(&it_tp1);
      if (mask)
      {
         gm_t=next_g(&it_mt);
         gm_tp1=next_g(&it_mtp1);
      }
   }

   ret=1;
cleanup:
   FREE_REG(rf);
   if (mask) FREE_REG(rmask);
   return ret;
} 

//=============================================================================
// bs specific --- computes d(A)/dz, assuming periodic in z
//=============================================================================
grid *gf_bs_ddz(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i,j,Nr,Nz;
   double dz;
 
   if (g->coord_type != COORD_UNIFORM && g->coord_type != COORD_SEMI_UNIFORM)
   {
      printf("gf_bs_ddz: non uniform coordinates not currently supported\n");
      return 0;
   }
 
   if (g->dim != 2)
   {
      printf("gf_bs_ddz: spatial dimension must be 2\n");
      return 0;
   }
 
   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;
   n=sizeof_data(ng);
 
   Nr=g->shape[0];
   Nz=g->shape[1];
   dz=(g->coords[3]-g->coords[2])/(Nz-1);
 
   for(i=0;i<Nr;i++)
      for(j=0;j<Nz;j++)
      {
         if (j==0) ng->data[i+j*Nr]=(g->data[i+(j+1)*Nr]-g->data[i+(Nz-2)*Nr])/2/dz;
         else if (j==(Nz-1)) ng->data[i+j*Nr]=(g->data[i+(1)*Nr]-g->data[i+(j-1)*Nr])/2/dz;
         else ng->data[i+j*Nr]=(g->data[i+(j+1)*Nr]-g->data[i+(j-1)*Nr])/2/dz;
      }
 
   return ng;
}  

//=============================================================================
// bs specific --- computes 1/r^2/a^2-1/r^2
//=============================================================================
grid *gf_bs_f1(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng;
   int n,i,j,Nr,Nz;
   double dr,r,a;
 
   if (g->coord_type != COORD_UNIFORM && g->coord_type != COORD_SEMI_UNIFORM)
   {
      printf("gf_bs_f1: non uniform coordinates not currently supported\n");
      return 0;
   }
 
   if (g->dim != 2)
   {
      printf("gf_bs_f1: spatial dimension must be 2\n");
      return 0;
   }
 
   if (!(ng=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,0))) return 0;
   n=sizeof_data(ng);
 
   Nr=g->shape[0];
   Nz=g->shape[1];
   dr=(g->coords[1]-g->coords[0])/(Nr-1);
 
   for(i=0;i<Nr;i++)
      for(j=0;j<Nz;j++)
      {
         r=i*dr;
         a=g->data[i+j*Nr];
         if (i>0) ng->data[i+j*Nr]=1/r/r/a/a-1/r/r; else ng->data[i+j*Nr]=0;
      }
 
   for(i=0,j=0;j<Nz;j++)
   {
      ng->data[j*Nr]=(4*ng->data[1+j*Nr]-ng->data[2+j*Nr])/3;
   }

   return ng;
}  

//=============================================================================
// bs specific --- attaches a semiuniform grid structure to A(x,z), 
//                 specified by the 1D register in the 2nd argument B=r(x)
//                 (default "r_of_x"). The first argument is a truncation
//                 length, beyond which the grid is clipped
//=============================================================================
grid *gf_bs_attach_nu(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *ga=it_a->g,*gb,*ng=0,*tg;
   int n,i,j,Nr,Nz,clip_pos,ibbox[6];
   reg *rb=0;
   s_iter it_b;
   double r,z,*rc,*zc,dz,max_r,x1,dx,x;
   char reg_name[MAX_NAME_LENGTH];
 
   if (ga->coord_type != COORD_UNIFORM)
   {
      printf("gf_bs_attach_nu: A be uniform\n");
      return 0;
   }
 
   if (ga->dim !=2)
   {
      printf("gf_bs_attach_nu: A must have dim=2\n");
      return 0;
   }

   reg_name[0]=0; max_r=0;
   if (args && strlen(args)>0) sscanf(args,"%lf,%s",&max_r,reg_name);
   if (reg_name[0]==0) strcpy(reg_name,"r_of_x");

   if (!(rb=find_reg(reg_name,1,1)))
   { 
      if (first_call) printf("gf_bs_attach_nu: cannot find register <%s>\n",reg_name); 
      if (first_call) printf("assuming coordinates of the form r=1/(1-x)\n");
   }
   else
      if (first_call) printf("coordinate array:%s   max_r=%f\n",reg_name,max_r);

   if (rb) gb=init_s_iter(&it_b,rb,0); else gb=0;

   Nr=ga->shape[0];
   if (rb && (Nr!=gb->shape[0]))
      { printf("gf_bs_attach_nu: shape[0] for registers B and A must be equal\n"); goto clean_up; }
   Nz=ga->shape[1];
   dz=(ga->coords[3]-ga->coords[2])/(Nz-1);
   dx=(ga->coords[1]-ga->coords[0])/(Nr-1);
   x1=ga->coords[1];

   while(gb && !(fuzz_eq(ga->time,gb->time,rb->fuzzy_t))
            && !(fuzz_eq(x1,gb->coords[1],rb->fuzzy_x[0]))
            && !(ga->shape[1]>=gb->shape[1])) gb=next_g(&it_b);
   if (!gb && rb) { printf("gf_bs_attach_nu: cannot find matching time in<%s>\n",reg_name); goto clean_up; }

   if (gb && gb->dim!=1)
      { printf("gf_bs_attach_nu: register B must have dim=1\n"); goto clean_up; }

   if (gb && gb->coord_type != COORD_UNIFORM)
      { printf("gf_bs_attach_nu: warning ... assuming register B is uniform\n"); }
                   
   if (!(ng=galloc(ga->dim,COORD_SEMI_UNIFORM,ga->shape,ga->time,ga->coords,ga->ccoords,ga->data))) return 0;
   n=sizeof_data(ng);
 
   rc=ng->ccoords;
   zc=rc+n;

   clip_pos=0;
   for(i=0;i<Nr;i++)
      for(j=0;j<Nz;j++)
      {
         z=j*dz+ga->coords[2];
         x=i*dx+ga->coords[0];
         if (x>(1.0-dx)) x=1-dx/2;
         if (gb) r=gb->data[i]; else r=1.0/(1.0-x);
         if (max_r>0 && r<max_r && clip_pos<i) clip_pos=i;
         rc[i+j*Nr]=r;
         zc[i+j*Nr]=z;
      }

   if (clip_pos>0)
   {
      tg=ng;
      ibbox[0]=1; ibbox[1]=clip_pos+1;
      ibbox[2]=1; ibbox[3]=ng->shape[1];
      ng=gclip(ng,ibbox);
      gfree(tg);
   }
 
clean_up:
   if (rb) FREE_REG(rb);

   return ng;
}  

//=============================================================================
// functions used by gf_bs_embed2 to find bubbles and strings
// r & z are areal r and embedding z.
// might not work if bubble is exactly at edge.
// bubbles sorted via radius of bubble
//=============================================================================
#define MAX_BUBBLES 100
#define BUBBLE_FRAC 1.5e0
#define BUBBLE_CLIP 1.2e0
int find_bubbles(double *r, double *z, int n, double *bub_r,  double *bub_z, int *bub_il, int *bub_ir, int *bub_i0, int *bub_name)
{
   int num_bub=0,in_order,order[MAX_BUBBLES];
   int i,j,ltrace=0;
   double p_rmin,t,p_bub_rmin,order_r[MAX_BUBBLES];

   IFL printf("find_bubbles: searching for bubbles\n");  
   p_rmin=r[0];
   i=0;
   while(i<(n-2))
   {
      i++;
      if (r[i]<p_rmin) p_rmin=r[i]; // smallest r since previous bubble in z
      if (r[i]>r[i-1] && r[i]>r[i+1] && r[i]>BUBBLE_FRAC*p_rmin)
      {
         if (num_bub>=(MAX_BUBBLES-1)) printf("find_bubbles ... error ... maximum exceeded\n");
         else
         {
            bub_r[num_bub]=r[i];
            bub_z[num_bub]=z[i];
            bub_i0[num_bub]=i;
            j=i;
            while(j && (bub_z[num_bub]-z[j])<BUBBLE_CLIP*bub_r[num_bub]) j--;
            bub_il[num_bub]=j; // left index of "start" of bubble
            if (num_bub>0)
            {
               j=bub_i0[num_bub-1];
               while(j<n && (z[j]-bub_z[num_bub-1])<BUBBLE_CLIP*bub_r[num_bub-1]) j++;
               bub_ir[num_bub-1]=j; // right index of end of previous bubble
            }
            order[num_bub]=num_bub+1;
            order_r[num_bub]=r[i];
            IFL printf("\t found bubble %i at w=%lf, r=%lf\n",num_bub+1,bub_z[num_bub],bub_r[num_bub]);
            num_bub++;
         }
         p_rmin=r[i];
      }
   }
   if (num_bub)
   {
      j=bub_i0[num_bub-1];
      while(j<n && (z[j]-bub_z[num_bub-1])<BUBBLE_CLIP*bub_r[num_bub-1]) j++;
      bub_ir[num_bub-1]=j; // right index of end of last bubble
   }

   // name bubbles from 1 to num_bub in order of size ... do so by sorting order
   IFL printf("Sorting bubbles\n");
   if (num_bub>1) 
   {
      in_order=0;
      while(!in_order)
      {
         in_order=1;
         for (i=0;i<(num_bub-1);i++) if (order_r[i]<order_r[i+1])
         {
            t=order_r[i]; order_r[i]=order_r[i+1]; order_r[i+1]=t; 
            j=order[i]; order[i]=order[i+1]; order[i+1]=j; 
            in_order=0;
         }
      }
      for (i=0;i<(num_bub);i++) { bub_name[order[i]-1]=i+1; IFL printf("\t bubble %i rank=%i\n",i+1,bub_name[i]);}
   }
   else if (num_bub) bub_name[0]=1;

   return num_bub;
}
//=============================================================================
// bs specific --- similar to gf_bs_embed, but used ah_areal and ah_pdz instead,
// and this produces "shifted" version automatically. Also finds segments
// and bubbles automatically. And don't know what this num-skip was all about
// args is dt,tag ... if dt>0, skips segments that are less than dt apart
//=============================================================================
grid *gf_bs_embed2(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g_area=it_a->g,*g_pdz=it_b->g,*ng,*ng2;
   double *z,*r,dl,dr,dz,r0,dw,*w;
   static double dt,prev_t;
   static char tag[256];
   int n,i,num_bub;
   double bub_r[MAX_BUBBLES],bub_z[MAX_BUBBLES],area,volume,length,z_tot,c_length;
   int bub_il[MAX_BUBBLES],bub_ir[MAX_BUBBLES],bub_i0[MAX_BUBBLES],bub_name[MAX_BUBBLES];
   int n_shape[1],ltrace=0;
   char name[256];
   int test_GL_first_seg=1; // if one, prints some experimental analysis of the first segment
   double z_tot_11,r_min_11,mu_max=0.44e0;
   static double T_0,T_1,T_2,T_3,T_4;
   static double A_0,A_1,A_2,A_3,A_4;
   static double O_0,O_1,O_2,O_3,O_4;
   double mu_0,mu_1,mu_2,mu_3,mu_4,dT;

 
   if (g_area->dim != 1)
   {
      printf("gf_bs_embed: spatial dimension must be 1\n");
      return 0;
   }

   if (first_call) 
   {
      tag[0]=0; dt=0;
      sscanf(args,"%lf,%s",&dt,tag);
      printf("gf_bs_embed2: dt=%lg, tag=%s\n",dt,tag);
      prev_t=it_a->ts->time-2*dt;
      T_0=T_1=T_2=T_3=T_4=0;
      A_0=A_1=A_2=A_3=A_4=1;
   }

   if (it_a->ts->time-prev_t < dt) return 0;
   dT=it_a->ts->time-prev_t;
   prev_t=it_a->ts->time;
 
   if (!(ng=galloc(g_area->dim,COORD_CURVILINEAR,g_area->shape,g_area->time,0,0,0))) return 0;
   n=sizeof_data(ng);

   z=ng->coords;
   w=it_a->g->coords;
   r=ng->data;
   z[0]=0;
   r[0]=g_area->data[0];
   area=length=z_tot=volume=c_length=0;
   for (i=1; i<n; i++)
   {
      r[i]=g_area->data[i];
      z[i]=z[i-1]+g_pdz->data[i-1];
      dr=g_area->data[i]-g_area->data[i-1];
      dz=g_pdz->data[i-1];
      dw=w[i]-w[i-1];
      dl=sqrt(dr*dr+dz*dz);
      r0=(r[i]+r[i-1])/2;
      area+=(4*M_PI*r0*r0*dl);
      length+=dl;
      c_length+=sqrt(dw*dw+dr*dr);
      z_tot+=dz;
      volume+=(4.0e0/3.0e0*M_PI*r0*r0*r0*dl);
   }

   num_bub=find_bubbles(r,z,n,bub_r,bub_z,bub_il,bub_ir,bub_i0,bub_name);

   // splice horizon up into bubbles and string segments. first bubbles

   if (num_bub)
   {
      for (i=0; i<num_bub; i++)
      {
         n_shape[0]=bub_ir[i]-bub_il[i]+1;
         if (n_shape[0]<0 || bub_il[i]<0 || bub_ir[i]>=n) printf("gf_bs_embed2: error in bubble size\n");
         else
         {
            IFL printf("about to add bubble %i, il=%i, ir=%i, n=%i, n_shape=%i\n",i+1, bub_il[i], bub_ir[i],n,n_shape[0]);
            if (!(ng2=galloc(g_area->dim,COORD_CURVILINEAR,n_shape,g_area->time,&z[bub_il[i]],0,&r[bub_il[i]]))) return 0;
            sprintf(name,"%s_bub_%i",args,bub_name[i]);
            add_grid_str(name,ng2);
         }
      }
      // first segment wraps around ... call it the 11 seg by default
      sprintf(name,"%s_seg_1_1",args);
      n_shape[0]=bub_il[0]+1;
      if (!(ng2=galloc(g_area->dim,COORD_CURVILINEAR,n_shape,g_area->time,z,0,r))) return 0;
      // length and minimum radius of first segment
      z_tot_11=z[n_shape[0]-1]-z[0];
      r_min_11=1e10; for (i=0;i<n_shape[0];i++) if (r_min_11>r[i]) r_min_11=r[i];
      add_grid_str(name,ng2);
      n_shape[0]=n-bub_ir[num_bub-1];
      if (!(ng2=galloc(g_area->dim,COORD_CURVILINEAR,n_shape,g_area->time,&z[bub_ir[num_bub-1]],0,&r[bub_ir[num_bub-1]]))) return 0;
      z_tot_11+=z[bub_ir[num_bub-1]+n_shape[0]-1]-z[bub_ir[num_bub-1]];
      for (i=0;i<n_shape[0];i++) if (r_min_11>r[bub_ir[num_bub-1]+i]) r_min_11=r[bub_ir[num_bub-1]+i];
      // find first few 1/2 integer modes present
      mu_0=2*M_PI*(0+0.5e0)*(r_min_11/z_tot_11/2); 
      if (mu_0<mu_max)
      {
         if (T_0==0) T_0=g_area->time; // mode just entered string
         O_0=((mu_max-mu_0)/mu_max)*mu_0/r_min_11;
         A_0=A_0+A_0*O_0*dT; //exponential growth at current omega
      }
      mu_1=2*M_PI*(1+0.5e0)*(r_min_11/z_tot_11/2); 
      if (mu_1<mu_max)
      {
         if (T_1==0) T_1=g_area->time; // mode just entered string
         O_1=((mu_max-mu_1)/mu_max)*mu_1/r_min_11;
         A_1=A_1+A_1*O_1*dT; //exponential growth at current omega
      }
      mu_2=2*M_PI*(2+0.5e0)*(r_min_11/z_tot_11/2); 
      if (mu_2<mu_max)
      {
         if (T_2==0) T_2=g_area->time; // mode just entered string
         O_2=((mu_max-mu_2)/mu_max)*mu_2/r_min_11;
         A_2=A_2+A_2*O_2*dT; //exponential growth at current omega
      }
      mu_3=2*M_PI*(3+0.5e0)*(r_min_11/z_tot_11/2); 
      if (mu_3<mu_max)
      {
         if (T_3==0) T_3=g_area->time; // mode just entered string
         O_3=((mu_max-mu_3)/mu_max)*mu_3/r_min_11;
         A_3=A_3+A_3*O_3*dT; //exponential growth at current omega
      }
      mu_4=2*M_PI*(4+0.5e0)*(r_min_11/z_tot_11/2); 
      if (mu_4<mu_max)
      {
         if (T_4==0) T_4=g_area->time; // mode just entered string
         O_4=((mu_max-mu_4)/mu_max)*mu_4/r_min_11;
         A_4=A_4+A_4*O_4*dT; //exponential growth at current omega
      }

      add_grid_str(name,ng2);

      // middle segments
      for (i=1; i<num_bub; i++)
      {
         n_shape[0]=bub_il[i]-bub_ir[i-1]+1;
         if (n_shape[0]>0) // because of BUBBLE_CLIP it may think some bubbles overlap ... ignore those segments
         {
            if (!(ng2=galloc(g_area->dim,COORD_CURVILINEAR,n_shape,g_area->time,&z[bub_ir[i-1]],0,&r[bub_ir[i-1]]))) return 0;
            sprintf(name,"%s_seg_%i_%i",args,bub_name[i-1],bub_name[i]);
            add_grid_str(name,ng2);
         }
      }
   }

   printf("t=%lf, area=%lf, volume=%lf, length=%lf, z_total=%lf, c_length=%lf, ",g_area->time,area,volume,length,z_tot,c_length);
   if (test_GL_first_seg)
   {
      printf(" Seg11: z=%6.4lf, r=%6.4lf, ",z_tot_11,r_min_11);
      printf(" T_0=%7.2lf, mu_0=%6.4lf, A_0=%7.2e, O_0=%6.4lf, ",T_0,mu_0,A_0,O_0);
      printf(" T_1=%7.2lf, mu_1=%6.4lf, A_1=%7.2e, O_1=%6.4lf, ",T_1,mu_1,A_1,O_1);
      printf(" T_2=%7.2lf, mu_2=%6.4lf, A_2=%7.2e, O_2=%6.4lf, ",T_2,mu_2,A_2,O_2);
      printf(" T_3=%7.2lf, mu_3=%6.4lf, A_3=%7.2e, O_3=%6.4lf, ",T_3,mu_3,A_3,O_3);
      printf(" T_4=%7.2lf, mu_4=%6.4lf, A_4=%7.2e, O_4=%6.4lf ",T_4,mu_4,A_4,O_4);
   }
   printf("\n");

   return ng;
}  

//=============================================================================
// bs specific --- produces an embedding diagram as gf_bs_embed2, but saves
// result as a 2D function, with second dimension now time.
//=============================================================================
grid *gf_bs_embed3(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g_area=it_a->g,*g_pdz=it_b->g,*ng,*nxt_g;
   double *z,*r;
   int n,i,numt;
   static int n_shape[2],t_i;
   static double pt,*ng_t,*ng_zp,*ng_r,dz,bbox[4];
   int ltrace=0;
 
   if (g_area->dim != 1)
   {
      printf("gf_bs_embed3: spatial dimension must be 1\n");
      return 0;
   }
   if (g_area->coord_type != COORD_UNIFORM)
   {
      printf("gf_bs_embed3:: non uniform coordinates not currently supported\n");
      return 0;
   }

   if (first_call) // find number of time steps
   {
      save_s_iter(it_a);
      nxt_g=g_area;
      pt=it_a->ts->time;
      bbox[0]=g_area->coords[0];
      bbox[1]=g_area->coords[1];
      bbox[2]=pt;
      numt=1;
      while(nxt_g)
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF)
         {
            numt++;
            pt=it_a->ts->time;
         }
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);
      bbox[3]=pt;

      n_shape[0]=g_area->shape[0];
      n_shape[1]=numt;
 
      if (!(ng=galloc(2,COORD_SEMI_UNIFORM,n_shape,0.0e0,bbox,0,0))) return 0;
      n=sizeof_data(ng);
      ng_zp=ng->ccoords;
      ng_t=ng_zp+n;
      ng_r=ng->data;
      pt=it_a->ts->time;
      t_i=0;
      add_grid_str(nf,ng);

      dz=(bbox[1]-bbox[0])/(n_shape[0]-1);
   }

   if (it_a->ts->time > pt) {pt=it_a->ts->time; t_i++;}

   for (i=0; i<n_shape[0]; i++) {ng_t[i+t_i*n_shape[0]]=pt;} // time is "y"

   z=ng_zp+t_i*n_shape[0]; // z is "x"
   r=ng_r+t_i*n_shape[0];  // r is "f(x,y)"
   z[0]=0;
   r[0]=g_area->data[0];
   for (i=0; i<n_shape[0]; i++)
   {
      r[i]=g_area->data[i];
      // z[i]=z[i-1]+g_pdz->data[i-1]; // HERE : CAN'T "UNIFORM" pdz!!!!
      z[i]=bbox[0]+i*dz;
   }

   return 0;
}  

//=============================================================================
// bs specific --- given ah_pdr and ah_pdz computes an embedding diagram of 
// the AH
//=============================================================================
grid *gf_bs_embed(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g_pdr=it_a->g,*g_pdz=it_b->g,*ng;
   double *z,*r,length,pdl,dl;
   int n,i,num_skip;
 
   if (g_pdr->dim != 1)
   {
      printf("gf_bs_embed: spatial dimension must be 1\n");
      return 0;
   }
 
   if (!(ng=galloc(g_pdr->dim,COORD_CURVILINEAR,g_pdr->shape,g_pdr->time,0,0,0))) return 0;
   n=sizeof_data(ng);

   z=ng->coords;
   r=ng->data;
   z[0]=0;
   r[0]=0;
   length=num_skip=0;
   for (i=0; i<(n-1); i++)
   {
      dl=sqrt(g_pdr->data[i]*g_pdr->data[i]+g_pdz->data[i]*g_pdz->data[i]);
      if (i>0 && pdl!=0 && fabs(dl/pdl)<0.1) 
      {
         r[i+1]=r[i]+g_pdr->data[i-1];
         z[i+1]=z[i]+g_pdz->data[i-1];
         length+=pdl;
         num_skip++;
      }
      else
      {
         r[i+1]=r[i]+g_pdr->data[i];
         z[i+1]=z[i]+g_pdz->data[i];
         length+=dl;
      }
      pdl=dl;
   }

   printf("gf_bs_embed: t=%lf   length=%lf    num_skip=%i\n",g_pdr->time,length,num_skip);
 
   return ng;
}  
//=============================================================================
// bs specific --- given a masked function A (zero < AH, non-zero > AH)
//                 returns a 1D slice of A on the masked (AH) boundary
//=============================================================================
grid *gf_bs_first_nz(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *A=it_a->g,*ng;
   int i,j;
 
   if (A->dim != 2)
   {
      printf("gf_bs_first_nz: input function must be 1D\n");
      return 0;
   }

   if (A->coord_type != COORD_UNIFORM)
   {
      printf("gf_bs_first_nz: non uniform coordinates not currently supported\n");
      return 0;
   }
 
   if (!(ng=galloc(1,COORD_UNIFORM,&A->shape[1],A->time,&A->coords[2],0,0))) return 0;

   for(j=0; j<A->shape[1]; j++)
   {
      i=0; while(i < A->shape[0] && fabs(A->data[i+j*A->shape[0]])<1.0e-10) i++;
      if (i < A->shape[0]) ng->data[j]=A->data[i+j*A->shape[0]]; else ng->data[j]=0;
   }
 
   return ng;
}  

//=============================================================================
// bs specific --- shifts the embedding diagram of a BS so that the minimum
// areal coordinate matches the minimum of the argument register
//=============================================================================
grid *gf_bs_shift_embed(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g_e=it_a->g,*ng,*g_s;
   double *r,*s,minr,mins,shift,dt,pt;
   s_iter it_s;
   int i,found=0,n;
   reg *sr;
 
   if (g_e->dim != 1)
   {
      printf("gf_bs_shift_embed: spatial dimension must be 1\n");
      return 0;
   }

   if (!(sr=find_reg(args,1,1)))
   {
      printf("gf_bs_shift_embed: cannot find register %s\n",args);
      return 0;
   }

   g_s=init_s_iter(&it_s,sr,0);
   pt=0;
   while(g_s && !found)
   {
      if (pt==0) dt=1.0e-5; else dt=g_s->time-pt;
      pt=g_s->time;
      if (fuzz_eq(g_e->time,g_s->time,dt)) found=1; else g_s=next_g(&it_s);
   }
   if (!found)
   {
      printf("warning --- cannot find time %lf in %s ... skipping\n",g_e->time,args);
      FREE_REG(sr);
      return 0;
   }
 
   if (!(ng=galloc(g_e->dim,COORD_CURVILINEAR,g_e->shape,g_e->time,g_e->coords,0,g_e->data))) return 0;
   n=sizeof_data(ng);

   r=ng->data;
   s=g_s->data;
   for (i=0,mins=s[0] ; i<sizeof_data(g_s); i++) mins=min(s[i],mins);
   for (i=0,minr=r[0] ; i<n; i++) minr=min(r[i],minr);
   shift=mins-minr;
   for (i=0; i<n; i++) r[i]+=shift;

   FREE_REG(sr);
   return ng;
}  

//------------------------------------------------------------------------------
// Produces a 3D grid ("inspiral_3D") in [x,y,t], with a potential surface schematically 
// representing the z=0 plane of an inspiraling BH binary.
//
// Also produces a 2D register ("inspiral_2D") in [x,y] x [t] of the same data.
// 
// arg is 'm2,r0,dt,tf,nx'
// mass m1=1, set m2<=1 
// initial seperation r0
// nt,nx^2 grid shape
//
// Newtonian inspiral with quadrupole energy loss:
// 
// xi=ri cos(wt)
// yi=ri sin(wt)
// 
// r=r1+r2
// m1*r1=m2*r2    (c.o.m frame)
// w=sqrt(m1+m2)/r^(3/2)  (circular orbit)
// 
// dr/dt = -64/5*(m1*m2)*(m1+m2)/r^3
// 
//------------------------------------------------------------------------------
void sf_inspiral(char *args)
{
   double m1=1,m2,r0,r1,r2,dt,tf,r;
   int nt,nx,i,j,k,ny;
   double t,dx,dy,x,y,d1,d2,w,x1,x2,y1,y2,dr;
   double bbox[6];
   int shape[3];
   grid *g3d,*g2d;

   if (args && (strlen(args)>0)) sscanf(args,"%lf,%lf,%lf,%lf,%i",&m2,&r0,&dt,&tf,&nx);

   printf("sf_inspiral: m2=%lf, r0=%lf, dt=%lf, tf=%lf, nx=%i\n",m2,r0,dt,tf,nx);

   delete_reg("inspiral_2D");
   delete_reg("inspiral_3D");

   ny=shape[0]=shape[1]=nx;
   shape[2]=nt=tf/dt+1;
   r1=m2*r0/(m1+m2);
   r2=m1*r0/(m1+m2);
   bbox[0]=bbox[2]=-2*max(r1,r2); bbox[1]=bbox[3]=-bbox[2];
   bbox[4]=0;
   bbox[5]=tf;
   dx=dy=(bbox[1]-bbox[0])/(nx-1);
   if (!(g3d=galloc(3,COORD_UNIFORM,shape,0,bbox,0,0))) return;

   t=0; r=r0;
   for (k=0; k<nt; k++)
   {
      r1=m2*r/(m1+m2);
      r2=m1*r/(m1+m2);
      if (r>0) w=pow((m1+m1)/r,0.5)/r; else w=0;
      x1=r1*cos(w*t);
      y1=r1*sin(w*t);
      x2=r2*cos(w*t+M_PI);
      y2=r2*sin(w*t+M_PI);

      printf("t=%lf, r1=%lf, r2=%lf, w=%lf\n",t,r1,r2,w);
      
      if (!(g2d=galloc(2,COORD_UNIFORM,shape,t,bbox,0,0))) { gfree(g3d); return; }

      for (i=0, x=bbox[0]; i<nx; i++, x+=dx)
         for (j=0, y=bbox[2]; j<ny; j++, y+=dy)
         {
            d1=pow((x-x1)*(x-x1)+(y-y1)*(y-y1),0.5)+1.0e-15;
            d2=pow((x-x2)*(x-x2)+(y-y2)*(y-y2),0.5)+1.0e-15;
            g3d->data[i+j*shape[0]+k*shape[0]*shape[1]]=
            g2d->data[i+j*shape[0]]=-m1/d1-m2/d2;
         }

      add_grid_str("inspiral_2D",g2d);
               
      t=t+dt;
      if (r>0) 
      {
         dr=dt*(-64.0/5.0*(m1*m2)*(m1+m2))/r/r/r;
         if ((-dr)>r) r=0; else r=r+dr;
      }
   }

   add_grid_str("inspiral_3D",g3d);

   return;
}

//------------------------------------------------------------------------------
// ds specific: computes the gamma factor, given (rho,z) components of
// velocity in (A,B) (flatspace)
//------------------------------------------------------------------------------
grid *gf_ds_gamma(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *ga=it_a->g,*gb=it_b->g,*ng;
   int i,j;
   double vrho,vz;

   if (ga->dim != 2 || gb->dim != 2)
   {
      printf("gf_ds_gamma: spatial dimension must be 2\n");
      return 0;
   }

   if (!(ng=galloc(ga->dim,ga->coord_type,ga->shape,ga->time,ga->coords,ga->ccoords,ga->data)))
   {
      printf("gf_ds_gamma: out of memory\n");
      return 0;
   }

   for (i=0; i<ga->shape[0]; i++)
   {
      for (j=0; j<ga->shape[1]; j++)
      {
         vrho=ga->data[i+j*ga->shape[0]];
         vz=gb->data[i+j*gb->shape[0]];
         ng->data[i+j*ng->shape[0]]=1/sqrt(1-vrho*vrho-vz*vz);
      }
   }

   return ng;
} 

//=============================================================================
// spins a 1-D sdf about 1/2-way around the axis. Argument is number of 
// sections
//=============================================================================
grid *gf_half_spin(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng=0;
   static int sections,repeat;
   int shape[2],i,j;
   double bbox[4];
   double *xc,*yc,*coords,*data,x,y,f,px,dx,xo;
 
   if (g->dim !=1)
   {
      printf("gf_half_spin: A must have dim=1\n");
      return 0;
   }

   if (first_call && args && strlen(args)>0) sscanf(args,"%i,%i",&sections,&repeat);
   else if (first_call) { sections=13; repeat=1; }
   if (first_call) printf("gf_half_spin: sections=%i, repetitions=%i\n",sections,repeat);

   shape[0]=g->shape[0]*repeat;
   shape[1]=sections;
                   
   if (!(ng=galloc(2,COORD_SEMI_UNIFORM,shape,g->time,0,0,0))) return 0;
   data=ng->data;
   coords=ng->coords;
   xc=ng->ccoords;
   yc=xc+sizeof_data(ng);
   bbox[0]=bbox[2]=1e10;
   bbox[1]=bbox[3]=-1e10;

   px=0;
   for(i=0;i<shape[0];i++)
   {
      xo=x;
      f=g->data[i%g->shape[0]];
      if (g->coord_type == COORD_UNIFORM)
      {
         x=g->coords[0]+i*(g->coords[1]-g->coords[0])/(g->shape[0]-1);
      }
      else if (g->coord_type == COORD_CURVILINEAR)
      {
         x=g->coords[i%g->shape[0]];
	 if (i>=g->shape[0]) x+=(px-g->coords[0]);
      }
      else
      {
         x=g->ccoords[i%g->shape[0]];
	 if (i>=g->shape[0]) x+=(px-g->ccoords[0]);
      }
      dx=x-xo;
      if ((i%g->shape[0])==(g->shape[0]-1)) px=x+dx;
      for(j=0;j<shape[1];j++)
      {
         y=-f*cos(M_PI*j/(double)(shape[1]-1));
         xc[i+j*shape[0]]=x;
         yc[i+j*shape[0]]=y;
         // data[i+j*shape[0]]=f*sin(M_PI*j/(double)(shape[1]-1));
         data[i+j*shape[0]]=f;
         coords[0]=min(coords[0],x);
         coords[1]=max(coords[1],x);
         coords[2]=min(coords[2],y);
         coords[3]=max(coords[3],y);
      }
   }

   return ng;
}  

//------------------------------------------------------------------------------
// for graxi_ad_w ... integrates J_dens (the argument) outwards
// from the origin.
//------------------------------------------------------------------------------
#define MAX_J 50000
void sf_j_int(char *args)
{
   char J[MAX_NAME_LENGTH];
   char J_int[MAX_NAME_LENGTH];

   reg *j_dens;
   level *lev;
   time_str *ts;
   grid *g;

   double rho0,rhon,drho,dz,j0;
   double jint[MAX_J];
   double rho[MAX_J];
   int i,j,i0,imax,jmin,jmax;

   strcpy(J,"all_J_dens3");
   if (args && (strlen(args)>0)) sscanf(args,"%s",J);

   strcpy(J_int,J);
   strcat(J_int,"_int");

   printf("sf_j_int: J=%s ... integrated(J)=%s\n",J,J_int);

   delete_reg(J_int);

   if (!(j_dens=find_reg(J,1,1)))
      { printf("sf_j_int: cannot find register <%s>\n",J_int); goto clean_up; }

   ts=j_dens->ts;
   while(ts)
   {
      rho0=0;
      i0=0;
      rho[0]=0; jint[0]=0;
      // start from finest level
      lev=ts->levels;
      while(lev->next) lev=lev->next;
      while(lev)
      {
         g=lev->grids;
	 if (g->next) printf("sf_j_int: WARNING ... cannot handle multiple grids per level yet\n"); 
	 if (g->dim != 2 || g->coord_type !=COORD_UNIFORM)
	    { printf("sf_j_int: error ... dim!=2 or g->coord_type !=COORD_UNIFORM\n"); goto clean_up; }
	 drho=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
	 dz=(g->coords[3]-g->coords[2])/(g->shape[1]-1);
	 if (abs(drho-dz)> drho/100)
	    { printf("sf_j_int: error ... drho must equal dz\n"); goto clean_up; }
	 rhon=rho0+drho;
	 while(i0<MAX_J && g->coords[0]<drho && rhon<g->coords[1] && rhon<g->coords[3] && rhon<(-g->coords[2]))
         {
	    rho0=rho0+drho;
	    i0++;
	    jmin=(-rho0-g->coords[2])/drho+0.5;
	    jmax=( rho0-g->coords[2])/drho+0.5;
	    imax=( rho0-g->coords[0])/drho+0.5;
	    if (jmin<0 || jmax>=g->shape[1] || imax>=g->shape[0])
	       { printf("sf_j_int: bug\n"); goto clean_up; }
	    j0=0;
	    for (i=0; i<=imax; i++)
	    {
	       j0+=drho*drho*(g->data[i+jmin*g->shape[0]]+g->data[i+jmax*g->shape[0]]);
	    }
	    for (j=jmin+1; j<=(jmax-1); j++)
	    {
	       j0+=drho*drho*(g->data[imax+j*g->shape[0]]);
	    }
            j0*=2*M_PI;
	    jint[i0]=j0+jint[i0-1];
	    rho[i0]=rho0;
	    rhon=rho0+drho;
	 }
         lev=lev->prev;
      }
      printf("t=%lf    J=%lf    rho_max=%lf\n",ts->time,jint[i0],rho[i0]);
      i0++;
      add_grid_str(J_int,galloc(1,COORD_CURVILINEAR,&i0,ts->time,rho,0,jint));
      ts=ts->next;
   }

clean_up:
   if (j_dens) FREE_REG(j_dens);
   return;
}

//------------------------------------------------------------------------------
// g2sc function
//
// plots the trajectory of A(B) along a uniform interval of values
// from x1 to x2. Produces a single sdf with "x" the new time
//------------------------------------------------------------------------------
grid *gf_A_vs_B_x_to_t(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g_a=it_a->g,*nxt_g;
   grid *g_b=it_b->g;
   static grid **ng;
   static int n,numt,n0,ks,shape;
   static double x1,x2,pt;
   double x0,dx,th;
   int i,ret,j;

   if (g_a->dim!=1) {printf("gf_A_vs_B_x_to_t: only dim=1 supported\n"); return 0;}

   if (first_call) 
   { 
      n0=0;
      n=10;
      x1=0;
      x2=2*M_PI;
      ng=0;
      ks=0;
      sscanf(args,"%lf,%lf,%i,%i",&x1,&x2,&n,&ks);
      printf("gf_A_vs_B_x_to_t: x1=%lf, x2=%lf, n=%i, cs=%i\n",x1,x2,n,ks);
      if (n<0) {printf("gf_A_vs_B_x_to_t: error ... n must be >0\n"); return 0; }
      if (!(ks==0 || ks>1)) {printf("gf_A_vs_B_x_to_t: error ... ks must be 0 or >1\n"); return 0; }
      if (n==1) dx=1; else dx=(x2-x1)/(n-1);
      save_s_iter(it_a);
      nxt_g=g_a;
      pt=it_a->ts->time;
      numt=1;
      while(nxt_g)
      {
         if (it_a->ts->time!=pt && it_a->selected!=GIV_OFF) 
         {
            numt++;
            pt=it_a->ts->time;
         } 
         nxt_g=next_g(it_a);
      }
      restore_s_iter(it_a);
      pt=it_a->ts->time;
      if (ks) shape=numt+ks+4; else shape=numt;
      // allocate all "times" now
      if (!(ng=(grid **)malloc(sizeof(grid *)*n))) {printf("gf_A_vs_B_x_to_t: out of memory\n"); return 0;}
      for (i=0,x0=x1; i<n; i++,x0+=dx)
      {
         if (!(ng[i]=galloc(1,COORD_CURVILINEAR,&shape,x0,0,0,0))) return 0;
	 add_grid_str(nf,ng[i]);
         if (ks)
         {
	    ng[i]->data[numt]=0;
	    ng[i]->coords[numt]=0;
            for (j=2; j<(ks+2); j++)
	    {
	       th=(j-2)*2*M_PI/(ks-1);
	       ng[i]->data[j+numt]=sin(th); if (ng[i]->data[j+numt]==0) ng[i]->data[j+numt]+=1e-10;
	       ng[i]->coords[j+numt]=cos(th);
	    }
	    ng[i]->data[numt+1]=ng[i]->data[numt+2];
	    ng[i]->coords[numt+1]=ng[i]->coords[numt+2];
	    ng[i]->data[numt+ks+2]=ng[i]->data[numt+2];
	    ng[i]->coords[numt+ks+2]=ng[i]->coords[numt+2];
	    ng[i]->data[numt+ks+3]=ng[i]->data[numt+3];
	    ng[i]->coords[numt+ks+3]=ng[i]->coords[numt+3];
	 }
      }
   }
   else if (!ng) return 0;

   if (it_a->ts->time!=pt) n0++;
   pt=it_a->ts->time;

   for (i=0,x0=x1; i<n; i++,x0+=dx)
   {
      ret=eval_1d(x0,g_a,mask,mask_val,&(ng[i]->data[n0]));
      ret=eval_1d(x0,g_b,mask,mask_val,&(ng[i]->coords[n0]));
   }

   save_s_iter(it_a);
   nxt_g=next_g(it_a); while(nxt_g && it_a->selected==GIV_OFF) nxt_g=next_g(it_a);
   if (!nxt_g) { free(ng); ng=0; }
   restore_s_iter(it_a);

   return 0;
}

//------------------------------------------------------------------------------
// attaches B as a semi-uniform coordinate system to A
//------------------------------------------------------------------------------
grid *gf_A_vs_B(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g_a=it_a->g;
   grid *g_b=it_b->g;
   grid *ng;

   if (!(ng=galloc(g_a->dim,COORD_SEMI_UNIFORM,g_a->shape,g_a->time,g_a->coords,g_b->data,g_a->data)))
   {
      printf("gf_A_vs_B: out of memory\n"); return 0;
   }

   return ng;
}

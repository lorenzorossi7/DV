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
#include <malloc.h>
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
   int n,ret;
   double *p,*m,pt;
   static time_str *ts;
   static double *ext,*t;
   static int ct,numt;

   if (first_call)
   {
      printf("maximum of %s:\nt\t\tmaximum\n",it_a->r->name);
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
   }

   if (!ext) return 0;
   if (it_a->ts!=ts)
   {
      printf("%f\t%e\n",t[ct],ext[ct]);
      ts=it_a->ts;
      ct++; ext[ct]=-1e50; t[ct]=ts->time;
   }

   p=g->data;
   n=sizeof_data(g);
   if (mask)
   {
      m=mask->data;
      while(n--)
      {
         if (*m++!=mask_val) { if ((*p)>(ext[ct])) ext[ct]=*p; }
         p++;
      }
   }
   else
   {
      while(n--)
      {
         if ((*p)>(ext[ct])) ext[ct]=*p;
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
// graxi-specific
//=============================================================================
extern void extend_(double *f, int *k, double *chr, int *first_n, int *axis, double *rho, double *z, int *Nrho, int *Nz);
extern void dmdiss3d_(double *f,double *work, double *eps, int *do_bdy, 
                      int *phys_bdy_type, int *even, int *odd, double *mask, double *mask_off, 
                      int *nx, int *ny, int *nz, double *chr, double *ex, int *do_ex);
extern void dmrepop3d1_(double *f, double *chr, double *ex, int *io, int *nx, int *ny, int *nz);
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
#endif

//=============================================================================
// from Smarr's article in sources of grav. rad. 1979
//
// de/dt = (1/4pi) * integral ( p  * r^2 d omega ), r=constant
//
// p=(integral [0..t] (psi4 dt))^2
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
   res/=(4*M_PI);
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
//=============================================================================
grid *gf_int_psi4_r(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng=0,*nxt_g;
   static double R,tn,tnm1,tnm2,*p,pt,*p4n,*p4nm1,*p4nm2,*de_dt,E,coords[2];
   static int n,numt,dsize,Ntheta,Nphi,test;
   int i,j,ij,ip1j,ijp1,ip1jp1;
   double st,dtheta,dphi;

   get_single_farg(&R,args,first_call,-1.0,"gf_int_psi4_r");

   if (g->dim>2) {printf("gf_int_psi4_r: dim too large\n"); return 0;}

   if (first_call) 
   { 
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
         if (Nphi>1)
         {
            for (j=0; j<(Nphi-1); j++)
            {
               ij=i+j*g->shape[0];
               ip1j=i+1+j*g->shape[0];
               ijp1=i+(j+1)*g->shape[0];
               ip1jp1=i+1+(j+1)*g->shape[0];

               // don't include theta=0 or PI points ... tetrad is singular there
               // and results crap

               if (i==0)
                  de_dt[n-1]+=((R*R*st/4/M_PI*dtheta*dphi)*
                               ((p[ip1j]*p[ip1j]+p[ip1jp1]*p[ip1jp1])/2));
               else if (i==(Ntheta-2))
                  de_dt[n-1]+=((R*R*st/4/M_PI*dtheta*dphi)*
                               ((p[ij]*p[ij]+p[ijp1]*p[ijp1])/2));
               else
                  de_dt[n-1]+=((R*R*st/4/M_PI*dtheta*dphi)*
                               ((p[ij]*p[ij]+p[ip1j]*p[ip1j]+p[ijp1]*p[ijp1]+p[ip1jp1]*p[ip1jp1])/4));
            
            }
         }
         else
         {
            //printf("i=%i, R=%lf, st=%lf, dtheta=%lf, p[i]=%lf, p[i+1]=%lf\n",i,R,st,dtheta,p[i],p[i+1]);
            de_dt[n-1]+=((R*R*st/2*dtheta)*((p[i]*p[i]+p[i+1]*p[i+1])/2));
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
      printf("Total energy radiated : %16.12lf\n",E);
      if (test) printf("Test ... expect %16.12lf\n",pow(coords[1]-coords[0],5)/30);
   }

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

   get_single_farg(&c,args,first_call,1,"gf_r_to_lnr");

   for(i=0,x=xi;i<g->shape[0];i++,x+=dx)
      for(j=0,y=yi;j<g->shape[1];j++,y+=dy)
      {
         r=sqrt(x*x+y*y);
         lnr=log(r+c)-log(c);
         if (r==0) { xn=yn=0; }
         else
         {
            xn=lnr*x/r;
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
//============================================================================= 
grid *gf_rtp_to_zxy(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ng,*nxt_g;
   int n,i,j,ind,ret;
   double dtheta,theta0,dphi,phi0,phi,theta,*xc,*yc,*z,*r,dx,dy,dz;
   static double csphere;
   static double *t,*x0,*y0,*z0;
   static int sizet;
   char pfile[64],buffer[256],c;
   int p_col,b_pos,t_pos,col;
   FILE *file;

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
      if (args && strlen(args)>0) sscanf(args,"%i,%i,%s",&csphere,&p_col,pfile);
      if (csphere!=0) printf("gf_rtp_to_zxy: transforming to a compactified coordinate sphere\n");
      if (pfile[0])
      {
         printf("reading position information from file %s;\n"
                "t is assumed to be in column 1, position info starts in column %i\n",pfile,p_col);
         if (!(file=fopen(pfile,"r")))
         {
            printf("error opening file %s\n",pfile);
         }
         else
         {
            while((c=fgetc(file))!=EOF) {if (c=='\n') sizet++;}
            sizet++;
            fclose(file);
            if (!(t=(double *)malloc(4*sizeof(double)*sizet)))
            {
               printf("error ... out of memory\n");
            }
            else
            {
               x0=&t[sizet];
               y0=&t[2*sizet];
               z0=&t[3*sizet];
               if (!(file=fopen(pfile,"r")))
               {
                  printf("error re-opening file %s\n",pfile);
               }
               else
               {
                  b_pos=0;
                  t_pos=0;
                  while((buffer[b_pos]=fgetc(file))!=EOF && b_pos<256 && t_pos<sizet) 
                  {
                     if (buffer[b_pos]=='\n')
                     {
                        sscanf(buffer,"%lf",&t[t_pos]);
                        col=1;
                        b_pos=0;
                        while(buffer[b_pos]!='\n' && col<p_col)
                        {
                           if (buffer[b_pos]==' ') { while(buffer[b_pos]==' ') b_pos++; col++; }
                           else b_pos++;
                        }
                        sscanf(&buffer[b_pos],"%lf %lf %lf",&x0[t_pos],&y0[t_pos],&z0[t_pos]);
                        printf("t=%lf, p=[%lf,%lf,%lf]\n",t[t_pos],x0[t_pos],y0[t_pos],z0[t_pos]);
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
      }
   }

   if (!(ng=galloc(g->dim,COORD_SEMI_UNIFORM,g->shape,g->time,g->coords,g->ccoords,g->data))) return 0;
   n=sizeof_data(ng);
   xc=ng->ccoords;
   yc=&ng->ccoords[n];
   z=ng->data;
   r=g->data;

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
// gh3d specific: evaluates a 3D or 2D axisymmetric function on an
// un-compactified sphere of radius R 
//
// NOTE: assumes levels sorted from lowest res to highest res
//-----------------------------------------------------------------------------
grid *gf_eval_r(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g=it_a->g,*ret_g,*nxt_g;
   grid *ng;
   int i,j,k,ret,ind;
   static time_str *ts;
   static double R,*evalr,coords[4];
   static int error,dim,numt,Ntheta,Nphi,Ntot,shape[2];
   double f,x0[3],theta,phi;
   
   ret_g=0;
   if (first_call)
   {
      error=0;
      ts=0;
      dim=g->dim;
      if (args && strlen(args)>0) sscanf(args,"%lf,%i,%i", &R,&Ntheta,&Nphi);
      else
      {
         printf("gf_eval_r: arguments required\n"); error=1; return 0;
      }
      shape[0]=Ntheta;
      shape[1]=Nphi;
      coords[0]=0;
      coords[1]=M_PI;
      coords[2]=0;
      coords[3]=2*M_PI;
      switch(dim)
      {
         case 2: printf("gf_eval_r: evaluating gf at R=%lf, Ntheta=%i\n",R,Ntheta); 
                 Ntot=Ntheta;
                 break;
         case 3: printf("gf_eval_r: evaluating gf at R=%lf, Ntheta=%i,Nphi=%i\n",R,Ntheta,Nphi); 
                 Ntot=Ntheta*Nphi;
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
      if (!(ng=galloc(dim-1,COORD_UNIFORM,shape,ts->time,coords,0,evalr))) return 0;
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
      switch(dim)
      {
         case 2: 
            for (i=0; i<Ntheta; i++)
            {
               theta=i*M_PI/(Ntheta-1);
               x0[0]=2*atan(R*cos(theta))/M_PI;
               x0[1]=2*atan(R*sin(theta))/M_PI;
               ret=eval_2d(x0[0],x0[1],g,mask,mask_val,&evalr[i]); 
            }
            break;
         case 3: 
            for (i=0; i<Ntheta; i++)
            {
               for (j=0; j<Nphi; j++)
               {
                  theta=i*M_PI/(Ntheta-1);
                  phi=j*2*M_PI/(Nphi-1);
                  ind=i+j*Ntheta;
                  x0[0]=2*atan(R*cos(phi)*sin(theta))/M_PI;
                  x0[1]=2*atan(R*sin(phi)*sin(theta))/M_PI;
                  x0[2]=2*atan(R*cos(theta))/M_PI;
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
      if (!ng) {if (!(ng=galloc(dim-1,COORD_UNIFORM,shape,ts->time,coords,0,evalr))) return 0;}
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
   double dx,frac,fuzz,fuzzn,dxn;

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
            i=(x-g->coords[0])/dx+0.1;
            if (i>=(N-1)) i=N-2; if (i<0) i=0;
            frac=(x-(i*dx+g->coords[0]))/dx;
            if (!mask || (mask->data[i]!=mask_val && mask->data[i+1]!=mask_val))
            {
               *f=frac*g->data[i+1]+(1-frac)*g->data[i];
               ret=1;
            }
         }
         break;
      case COORD_CURVILINEAR: 
         dx=(g->coords[1]-g->coords[0]);
         if (dx<0) di=N-1; else di=0;
         dxn=(g->coords[N-1]-g->coords[N-2]);
         fuzz=dx/R_INT_FUZZ;
         fuzzn=dxn/R_INT_FUZZ;
         if (!(fuzz_lt(x,g->coords[0+di],fuzz) || fuzz_gt(x,g->coords[N-1-di],fuzzn)))
         {
            i=1;
            while(x>g->coords[i]) i++;
            if (i>(N-1)) i=N-1;
            if (!mask || (mask->data[i]!=mask_val && mask->data[i-1]!=mask_val))
            {
               frac=(x-g->coords[i-1])/(g->coords[i]-g->coords[i-1]);
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
         fuzzy=dx/R_INT_FUZZ;
         if (!(fuzz_lt(x,g->coords[0+di],fuzzx) || fuzz_gt(x,g->coords[1-di],fuzzx) || 
               fuzz_lt(y,g->coords[2+dj],fuzzy) || fuzz_gt(y,g->coords[3-dj],fuzzy)))
         {
            i=(x-g->coords[0])/dx+0.1;
            if (i>=(Nx-1)) i=Nx-2; if (i<0) i=0;
            j=(y-g->coords[2])/dy+0.1;
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
         fuzzy=dx/R_INT_FUZZ;
         fuzzz=dz/R_INT_FUZZ;
         if (!(fuzz_lt(x,g->coords[0+di],fuzzx) || fuzz_gt(x,g->coords[1-di],fuzzx) || 
               fuzz_lt(y,g->coords[2+dj],fuzzy) || fuzz_gt(y,g->coords[3-dj],fuzzy) || 
               fuzz_lt(z,g->coords[4+dk],fuzzy) || fuzz_gt(z,g->coords[5-dk],fuzzy)))
         {
            i=(x-g->coords[0])/dx+0.1;
            if (i>=(Nx-1)) i=Nx-2; if (i<0) i=0;
            j=(y-g->coords[2])/dy+0.1;
            if (j>=(Ny-1)) j=Ny-2; if (j<0) j=0;
            k=(z-g->coords[4])/dz+0.1;
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
   grid *g=it_a->g,*ret_g,*nxt_g;
   grid *ng;
   int i,j,k,n,ret;
   static time_str *ts;
   static double x0[3],*evalr,*t;
   static int error,dim,numt,ct,l_eval;
   double f,pt;
   
   ret_g=0;
   if (first_call)
   {
      error=0;
      ts=0;
      dim=g->dim;
      if (args && strlen(args)>0) sscanf(args,"%lf,%lf,%lf", &x0[0],&x0[1],&x0[0]);
      else
      {
         printf("gf_eval: arguments required\n"); error=1; return 0;
      }
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
         return 0;
      }
      ts=it_a->ts;
      l_eval=0;
      ct=0;
      evalr[ct]=0;
      t[ct]=ts->time;
   }

   if (!evalr) return 0;

   if (it_a->ts!=ts)
   {
      printf("%f\t%e (l=%i)\n",t[ct],evalr[ct],l_eval);
      ts=it_a->ts;
      ct++; evalr[ct]=0; t[ct]=ts->time;
   }

   if (g->dim!=dim)
   {
      printf("gf_eval: error ... multiple grid dimensions in register\n");
      printf("   -> grid of dimension %i will be dropped\n",g->dim);
   }
   else
   {
      ret=0;
      switch(dim)
      {
         case 1: 
            ret=eval_1d(x0[0],g,mask,mask_val,&f); break;
         case 2: 
            ret=eval_2d(x0[0],x0[1],g,mask,mask_val,&f); break;
         case 3: 
            ret=eval_3d(x0[0],x0[1],x0[2],g,mask,mask_val,&f); break;
      }
      if (ret) { evalr[ct]=f; t[ct]=ts->time; l_eval=it_a->ln; }
   }

   ng=0;
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
// bs specific --- given ah_pdr and ah_pdz computes an embedding diagram of 
// the AH
//=============================================================================
grid *gf_bs_embed(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call)
{
   grid *g_pdr=it_a->g,*g_pdz=it_b->g,*ng;
   double *z,*r,length,pdl,dl;
   int n,i,num_skip;
 
   if (g_pdr->coord_type != COORD_UNIFORM)
   {
      printf("gf_bs_embed: non uniform coordinates not currently supported\n");
      return 0;
   }
 
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

//-----------------------------------------------------------------------------
// s_iter.c --- sequential iterator for a (locked) register
//
// The next_<str>() functions implement a 'depth-first' traversal of the register
// up to the depth of <str>.
//
// The iterator was designed for read-only operations.
//
// The optional argument givec is a generalized index vector, for selection
// control. The format for givec is
//
// "t=ivec;l=ivec;cb=coordinate bbox;ib=index bbox"
// 
// each piece is optional, and the ordering is arbitrary (though only
// 1 of cbb or ibb will be interpreted). All indecies start at 1.
// 
// The 'selected' flag of the iterator then has the following values:
// 
// GIV_ON --- grid matches all criteria, or no givec specified
// GIV_OFF --- grid fails to match all criteria
// GIV_CLIP --- grid matches t and l ivecs, but only part of it matches
//              the bounding box, hence it needs to be clipped
//
// The save/restore_s_iter() functions allow one to mark a position
// within a traversal, and later return to. (We could eventually
// extend this mechanism using a stack, but perhaps it would
// be better to add more options in a new iterator class, say
// a random-access iterator)
//-----------------------------------------------------------------------------

#include "s_iter.h"
#include "misc.h"
#include <bbhutil.h>

void s_iter_set_selected(s_iter *it, int redo_t, int redo_l, 
                         int use_tivec, int *tivec, int use_livec, int *livec)
{
   grid *g;

   it->selected=GIV_ON;
   if (use_tivec)
      if (redo_t)
      {
         if (!do_ivec(it->tn,it->num_t,tivec))
         {
            it->selected=GIV_OFF;
            it->t_selected=0;
            return;
         }
         it->t_selected=1;
      }
      else if (!it->t_selected)
      {
         it->selected=GIV_OFF;
         return;
      }

   if (use_livec)
      if (redo_l)
      {
         if (!do_ivec(it->ln,it->num_l,livec))
         {
            it->selected=GIV_OFF;
            it->l_selected=0;
            return;
         }
         it->l_selected=1;
      }
      else if (!it->l_selected)
      {
         it->selected=GIV_OFF;
         return;
      }
      
   g=it->g;
   if (it->use_bbox==IBBOX)
   {
      switch(g->dim)
      {
         case 3: if (it->ibbox[4]>g->shape[2]) { it->selected=GIV_OFF; return; }
                 if (it->ibbox[4]>1 || it->ibbox[5]<g->shape[2]) it->selected=GIV_CLIP;
         case 2: if (it->ibbox[2]>g->shape[1]) { it->selected=GIV_OFF; return; }
                 if (it->ibbox[2]>1 || it->ibbox[3]<g->shape[1]) it->selected=GIV_CLIP;
         case 1: if (it->ibbox[0]>g->shape[0]) { it->selected=GIV_OFF; return; }
                 if (it->ibbox[0]>1 || it->ibbox[1]<g->shape[0]) it->selected=GIV_CLIP;
      }
   }
   else if (it->use_bbox==CBBOX)
   {
      if (g->coord_type!=COORD_UNIFORM && g->coord_type!=COORD_SEMI_UNIFORM)
      {
         printf("s_iter_set_selected: only uniform/semi-uniform grids can be clipped at this time\n");
         return;
      }
      switch(g->dim)
      {
         case 3: if (fuzz_gt(it->cbbox[4],g->coords[5],it->r->fuzzy_x[2]) ||
                     fuzz_lt(it->cbbox[5],g->coords[4],it->r->fuzzy_x[2]) )
                    { it->selected=GIV_OFF; return; }
                 if (fuzz_gt(it->cbbox[4],g->coords[4],it->r->fuzzy_x[2]) ||
                     fuzz_lt(it->cbbox[5],g->coords[5],it->r->fuzzy_x[2]) )
                    it->selected=GIV_CLIP; 
         case 2: if (fuzz_gt(it->cbbox[2],g->coords[3],it->r->fuzzy_x[1]) ||
                     fuzz_lt(it->cbbox[3],g->coords[2],it->r->fuzzy_x[1]) )
                    { it->selected=GIV_OFF; return; }
                 if (fuzz_gt(it->cbbox[2],g->coords[2],it->r->fuzzy_x[1]) ||
                     fuzz_lt(it->cbbox[3],g->coords[3],it->r->fuzzy_x[1]) )
                    it->selected=GIV_CLIP; 
         case 1: if (fuzz_gt(it->cbbox[0],g->coords[1],it->r->fuzzy_x[0]) ||
                     fuzz_lt(it->cbbox[1],g->coords[0],it->r->fuzzy_x[0]) ) 
                    { it->selected=GIV_OFF; return; }
                 if (fuzz_gt(it->cbbox[0],g->coords[0],it->r->fuzzy_x[0]) ||
                     fuzz_lt(it->cbbox[1],g->coords[1],it->r->fuzzy_x[0]) ) 
                    it->selected=GIV_CLIP;
      }
   }
}

#define MAX_LN 32
#define MAX_IVECL 256
grid *init_s_iter(s_iter *it, reg *r, const char *givec)
{
   const char *s,*se;
   char *p;
   char ivec[MAX_IVECL*2]="x:=";
   int tivec[MAX_IVECL];
   int livec[MAX_IVECL];
   int i_livec[MAX_IVECL];
   int use_tivec,use_livec;
   int redo_l,redo_t,ferr=0;

   double dx[MAX_LN];
   float fbbox[MAX_DIM*2];
   int i,j,num;

   time_str *ts;
   level *l;
   grid *g;

   int ltrace=1;

   it->r=it->r_s=r;
   it->ts=it->ts_s=ts=r->ts;
   it->l=it->l_s=r->ts->levels;
   it->g=it->g_s=r->ts->levels->grids;

   if (givec)
   {
      it->use_givec=1;
      it->tn=it->tn_s=1;
      it->num_t=1; while(ts=ts->next) it->num_t++;

      it->num_l=1;
      dx[0]=it->l->dx;
      ts=r->ts;
      while(ts && it->num_l<MAX_LN)
      {
         l=ts->levels;
         while(l && it->num_l<MAX_LN)
         {
            i=0;
            while(i<it->num_l && fuzz_lt(l->dx,dx[i],r->fuzzy_x[0])) i++;
            if (i==it->num_l)
            {
               it->num_l++;
               dx[i]=l->dx;
            }
            else if (!(fuzz_eq(dx[i],l->dx,r->fuzzy_x[0])))
            {
               for (j=it->num_l; j>i; j--) dx[j]=dx[j-1];
               it->num_l++;
               dx[i]=l->dx;
            }
            l=l->next;
         }
         ts=ts->next;
      }
               
      if (it->num_l==MAX_LN)
         printf("init_s_iter: warning ... with a givec,"
                " s_iter can only handle the first %i levels\n",MAX_LN);

      ts=r->ts;
      while(ts)
      {
         l=ts->levels;
         while(l)
         {
            i=0;
            while(i<it->num_l && !(fuzz_eq(dx[i],l->dx,r->fuzzy_x[0]))) i++;
            l->ln=i+1;
            l=l->next;
         }
         ts=ts->next;
      }
      it->ln=it->ln_s=it->l->ln;
      
      s=givec;
      use_tivec=0;
      if (s=first_c(s,'t'))
         if (s=next_c(++s,'='))
         {
            if (!(se=first_c(++s,';'))) se=s+strlen(s);
            if ((se-s)>(MAX_IVECL*2-4))
               printf("init_s_iter: givec <%s> string too large\n",givec);
            else 
            {
               strncpy(&ivec[3],s,se-s);
               ivec[3+se-s]=0;
               if (!(sget_ivec_param(ivec,"x",tivec,MAX_IVECL)))
                  printf("init_s_iter: sget_ivec_param failed for <%s>\n",ivec);
               else
               {
                  use_tivec=1;
                  fixup_ivec(1,it->num_t,0,tivec);
                  if (ltrace) printf("register %s:: using tivec=<%s>"
                                     ", num_t=%i\n",r->name,ivec,it->num_t);
               }
            }
         }
         else ferr=1;

      s=givec;
      use_livec=0;
      if (s=first_c(s,'l'))
         if (s=next_c(++s,'='))
         {
            if (!(se=first_c(++s,';'))) se=s+strlen(s);
            if ((se-s)>(MAX_IVECL*2-4))
               printf("init_s_iter: givec <%s> string too large\n",givec);
            else 
            {
               strncpy(&ivec[3],s,se-s);
               ivec[3+se-s]=0;
               if (!(sget_ivec_param(ivec,"x",i_livec,MAX_IVECL)))
                  printf("init_s_iter: sget_ivec_param failed for <%s>\n",ivec);
               else
               {
                  use_livec=1;
                  fixup_ivec(1,it->num_l,0,i_livec);
                  if (ltrace) printf("register %s:: using livec=<%s>"
                                     ", num_l=%i\n",r->name,ivec,it->num_l);
               }
            }
         }
         else ferr=1;
                  
      s=givec;
      it->use_bbox=0;
      if ((s=first_c(s,'b')) && s>givec)
      {
         if (*(s-1)=='c') 
            it->use_bbox=CBBOX;
         else if (*(s-1)=='i')
            it->use_bbox=IBBOX;
         else ferr=1;
         if (!ferr)
         {
            s++;
            if (s=next_c(s,'='))
            {
               if (!(se=first_c(++s,';'))) se=s+strlen(s);
               if ((se-s)>(MAX_IVECL*2-4))
                  printf("init_s_iter: givec <%s> string too large\n",givec);
               else
               {
                  if (it->use_bbox==CBBOX)
                  {
                     for (i=0;i<2*MAX_DIM;i++) fbbox[i]=0;
                     num=sscanf(s,"%f,%f,%f,%f,%f,%f",&fbbox[0],&fbbox[1],&fbbox[2],
                                                  &fbbox[3],&fbbox[4],&fbbox[5]);
                     for (i=0;i<2*MAX_DIM;i++) it->cbbox[i]=fbbox[i];
                     if (ltrace) printf("register %s:: using cbbox=[%f,%f,%f,%f,%f,%f]\n",
                        r->name,it->cbbox[0],it->cbbox[1],it->cbbox[2],
                        it->cbbox[3],it->cbbox[4],it->cbbox[5]);
                  }
                  else
                  {
                     for (i=0;i<2*MAX_DIM;i++) it->ibbox[i]=0;
                     num=sscanf(s,"%i,%i,%i,%i,%i,%i",&it->ibbox[0],&it->ibbox[1],&it->ibbox[2],
                                                  &it->ibbox[3],&it->ibbox[4],&it->ibbox[5]);
                     if (ltrace) printf("register %s:: using ibbox=[%i,%i,%i,%i,%i,%i]\n",
                        r->name,it->ibbox[0],it->ibbox[1],it->ibbox[2],
                        it->ibbox[3],it->ibbox[4],it->ibbox[5]);
                  }
               }
            }
            else ferr=1;
         }
      }

      if (ferr)
         printf("init_s_iter: givec syntax error. format is: t=<t ivec>; l=<level ivec>; cb=<coord bbox> | ib=<index bbox>\n");

      // calculate the giv_selected flag for all grids now. 
      // this is because do_ivec is a sequential operator, i.e.
      // we can't call it with a smaller value once a larger one
      // has been used. So to avoid saving copies of the ivec arrays
      // for each iterator, we just compute the flag here.
      ts=it->ts;
      while(ts)
      {
         redo_t=1;
         it->l=l=ts->levels;
         while(l)
         {
            redo_l=1;
            if (use_livec) for (i=0; i<MAX_IVECL; i++) livec[i]=i_livec[i]; 
            it->g=g=l->grids;
            it->ln=l->ln;
            while(g)
            {
               s_iter_set_selected(it,redo_t,redo_l,use_tivec,tivec,use_livec,livec);
               redo_t=0;
               redo_l=0;
               g->giv_selected=it->selected;
               it->g=g=g->next;
            }
            it->l=l=l->next;
         }
         it->tn++;
         it->ts=ts=ts->next;
      }
      it->g=restore_s_iter(it);
   }
   else
   {
      it->use_givec=0;
      it->selected=GIV_ON;
   }

   return it->g;
}

grid *next_g(s_iter *it)
{
   if (!IS_IT_VALID(it)) return 0;
   if (it->g=it->g->next)
   {
      if (it->use_givec) it->selected=it->g->giv_selected;
      return it->g;
   }
   if (it->l=it->l->next)
   {
      it->g=it->l->grids;
      if (it->use_givec)
      {
         it->ln=it->l->ln;
         it->selected=it->g->giv_selected;
      }
      return it->g;
   }
   if (it->ts=it->ts->next)
   {
      it->l=it->ts->levels;
      it->g=it->l->grids;
      if (it->use_givec)
      {
         it->ln=it->l->ln;
         it->tn++;
         it->selected=it->g->giv_selected;
      }
      return it->g;
   }
   SET_IT_INVALID(it);
   return 0;
}

level *next_l(s_iter *it)
{
   if (!IS_IT_VALID(it)) return 0;
   if (it->l=it->l->next)
   {
      it->g=it->l->grids;
      if (it->use_givec)
      {
         it->ln=it->l->ln;
         it->selected=it->g->giv_selected;
      }
      return it->l;
   }
   if (it->ts=it->ts->next)
   {
      it->l=it->ts->levels;
      it->g=it->l->grids;
      if (it->use_givec)
      {
         it->ln=it->l->ln;
         it->tn++;
         it->selected=it->g->giv_selected;
      }
      return it->l;
   }
   SET_IT_INVALID(it);
   return 0;
}

time_str *next_ts(s_iter *it)
{
   if (!IS_IT_VALID(it)) return 0;
   if (it->ts=it->ts->next)
   {
      it->l=it->ts->levels;
      it->g=it->l->grids;
      if (it->use_givec)
      {
         it->ln=it->l->ln;
         it->tn++;
         it->selected=it->g->giv_selected;
      }
      return it->ts;
   }
   SET_IT_INVALID(it);
   return 0;
}

void save_s_iter(s_iter *it)
{
   if (!IS_IT_VALID(it)) return;  // can't save an invalid state
   it->r_s=it->r;
   it->ts_s=it->ts;
   it->g_s=it->g;
   it->l_s=it->l;
   it->tn_s=it->tn;
   it->ln_s=it->ln;
}

grid *restore_s_iter(s_iter *it)
{
   it->r=it->r_s;
   it->ts=it->ts_s;
   it->g=it->g_s;
   it->l=it->l_s;
   it->tn=it->tn_s;
   it->ln=it->ln_s;
   if (it->use_givec) it->selected=it->g->giv_selected;
   return it->g;
}

//-----------------------------------------------------------------------------
// the clipping function uses an index bbox; if the user specified a 
// coordinate bbox, this routine calculates it for the *current* grid
//-----------------------------------------------------------------------------
void set_ibbox(s_iter *it)
{
   double dx;
   int i;
   grid *g=it->g;

   if ((it->use_bbox!=CBBOX) || !(IS_IT_VALID(it))) return;

   if (g->coord_type != COORD_UNIFORM && g->coord_type != COORD_SEMI_UNIFORM)
   {
       printf("set_ibbox: only uniform grids currently supported\n");
       return;
   }

   for (i=0; i<g->dim; i++)
   {
      if (g->shape[i]>1) dx=(g->coords[2*i+1]-g->coords[2*i])/(g->shape[i]-1);
      else dx=1;
      it->ibbox[2*i]=(it->cbbox[2*i]-g->coords[2*i])/dx+2.0-it->r->fuzzy_x[i]; 
      it->ibbox[2*i+1]=(it->cbbox[2*i+1]-g->coords[2*i])/dx+1.0+it->r->fuzzy_x[i];
   }
}

/*
   functions to build and evaluate the parse trees
   that will be produced by DV_calc.y
*/

#define IFL if(ltrace)

#include "parse.h"
#include <stdlib.h>
#ifndef DARWIN
#include <malloc.h>
#endif 
#include <math.h>
#include "reg.h"
#include "fncs.h"
#include "instr.h"
#include "string.h"

pte *stmnt_list=0;

char *DV_calc_input=0;
char *DV_calc_input_ptr=0;

char *DV_calc_mask=0;
char *DV_calc_filter=0;
char *DV_calc_arg=0;
double DV_calc_mask_val=0;

int DV_calc_debug_mode=0;

/*=============================================================================
   allocates a new parse tree element. 
   NOTE: reg is used as the actual storage for the string, assumed to have been
   malloc'd prior to this call
  =============================================================================*/
pte *alloc_pte(pte *left, pte *right, int op, double number, char *reg)
{
   pte *p;

   if (!(p=(pte *)malloc(sizeof(struct pte))))
   {
      printf("alloc_pte: out of memory\n");
      return 0;
   }

   p->left=left;
   p->right=right;
   p->op=op;
   p->number=number;
   p->reg=reg;
   p->next_stmnt=0;

   return p;
}

void free_pte(pte *p)
{
   if (p->reg) free(p->reg);
   free(p);
}

/*=============================================================================
   adds a pte p to the end of the list of statements
  =============================================================================*/
void add_stmnt(pte *p)
{
   pte *q;

   if (q=stmnt_list)
   {
      while(q->next_stmnt) q=q->next_stmnt;
      q->next_stmnt=p;
   }
   else
      stmnt_list=p;
}

void free_stmnt_list()
{
   pte *q,*p;

   q=stmnt_list;
   while(q)
   {
      p=q;
      q=q->next_stmnt;
      free_pte(p);
   }
   stmnt_list=0;
}

/*=============================================================================
   utility functions for managing temporary variables
  =============================================================================*/
#define TMP_LENGTH 50
const char tmp_prefix[]="__DV_calc_tmp_var_";

char *get_tmp_var()
{
   static int tmp_no=0;
   char *name;

   if (!(name=(char *)malloc(TMP_LENGTH)))
   {
      printf("get_tmp_var: out of memory\n");
      return 0;
   }

   sprintf(name,"%s%i",tmp_prefix,tmp_no++);
   return name;
}

int is_tmp_var(char *p)
{
   if (p && !(strncmp(p,tmp_prefix,strlen(tmp_prefix)))) return 1; else return 0;
}

void free_tmp_var(char *p)
{
   if (is_tmp_var(p))
   {
      if (!DV_calc_debug_mode) delete_reg(p);
      free(p);
   }
}

/*=============================================================================
   for testing, simple symbol-table
  =============================================================================*/
#define MAX_SYM_VARS 20
char *sym_var_name[MAX_SYM_VARS];
double sym_var_val[MAX_SYM_VARS];

void clear_sym_tab(void)
{
   int i;

   for (i=0;i<MAX_SYM_VARS;i++) sym_var_name[i]=0;
}

int set_sym(char *name, double val)
{
   int i,k;

   for (k=-1,i=0;i<MAX_SYM_VARS && k==-1;i++) if (sym_var_name[i] && !(strcmp(name,sym_var_name[i]))) k=i;
   
   if (k>=0)
      sym_var_val[k]=val;
   else
   {
      for (k=-1,i=0;i<MAX_SYM_VARS && k==-1;i++) if (!(sym_var_name[i])) k=i;
      if (k>=0) 
      {
         sym_var_name[k]=name;
         sym_var_val[k]=val;
      }
      else
      {
         printf("set_sym: out of variable space!\n");
         return 0;
      }
   }

   return 1;
}

int read_sym(char *name, double *val)
{
   int i,k;

   for (k=-1,i=0;i<MAX_SYM_VARS && k==-1;i++) if (sym_var_name[i] && !(strcmp(name,sym_var_name[i]))) k=i;
   
   if (k>=0)
      *val=sym_var_val[k];
   else
   {
      printf("read_sym: undefined variable <%s>\n",name);
      return 0;
   }

   return 1;
}

int remove_sym(char *name)
{
   int i,k;

   for (k=-1,i=0;i<MAX_SYM_VARS && k==-1;i++) if (sym_var_name[i] && !(strcmp(name,sym_var_name[i]))) k=i;
   
   if (k>=0) { free(name); sym_var_name[k]=0; } else return 0;

   return 1;
}

/*=============================================================================
   evaluates a parse tree node via depth-first traversal
  =============================================================================*/
int eval_error_state=0;
void eval_stmnt(pte *p)
{
   int ltrace=0;
   double num;
   char *reg;
   pte *preg,*pnum;

   if (p->left) eval_stmnt(p->left); if (eval_error_state) return;
   if (p->right) eval_stmnt(p->right); if (eval_error_state) return;

   switch(p->op)
   {
      case OP_DECL_CONST: IFL printf("DECL_CONST: %lf\n",p->number); break;
      case OP_DECL_REGISTER: 
         IFL printf("OP_DECL_REGISTER: %s\n",p->reg);
         if (DV_calc_debug_mode)
         {
            if (!(read_sym(p->reg,&p->number))) eval_error_state=1;
         }
         break;
      case OP_ASSIGN: 
         IFL printf("OP_ASSIGN: %s=%lf\n",p->reg,p->right->number); 
         if (DV_calc_debug_mode)
         {
            if (!(set_sym(p->reg,p->right->number))) eval_error_state=1;
         }
         else
         {
            if (!p->right->reg)
            {
               printf("execution error: cannot assign a number to register %s\n",p->reg);
            }
            else 
            {
               /* mask has no effect in clone, and don't want DV complaining about mismatches */
               apply_unary_gf(gf_clone,DV_calc_arg,p->right->reg,p->reg,0,0,DV_calc_filter);
            }
         }
         break;
      case OP_RENAME: 
         IFL printf("OP_RENAME: %s -> %s\n",p->left->reg,p->reg); 
         if (DV_calc_debug_mode)
         {
            if (!(set_sym(p->reg,p->left->number))) eval_error_state=1;
            if (strcmp(p->reg,p->left->reg)) remove_sym(p->left->reg);
         }
         else
         {
            rename_reg(p->left->reg,p->reg);
         }
         break;
      case OP_SET_MASK:
         IFL printf("OP_SET_MASK: %s\n",p->right->reg);
         if (DV_calc_mask) free(DV_calc_mask); DV_calc_mask=0;
         if (p->right->reg && strlen(p->right->reg))
            if (!(DV_calc_mask=strdup(p->right->reg)))
            {
               printf("execution error: out of memory\n"); eval_error_state=1;
            }
         break;
      case OP_SET_MASK_VAL:
         IFL printf("OP_SET_MASK_VAL: %lf\n",p->number);
         DV_calc_mask_val=p->number;
         break;
      case OP_SET_FILTER:
         IFL printf("OP_SET_FILTER: %s\n",p->right->reg);
         if (DV_calc_filter) free(DV_calc_filter); DV_calc_filter=0;
         if (p->right->reg && strlen(p->right->reg))
            if (!(DV_calc_filter=strdup(p->right->reg)))
            {
               printf("execution error: out of memory\n"); eval_error_state=1;
            }
         break;
      /*******************************************************************
       * NOTE: OP_SET_ARG also serves as a "null" operator, in that      *
       *       it passes the left register through without modification. *
       *       Therefore, after doing this, the left register is cleared *
       *       to avoid erroneous deletion if it is a temporary.         * 
       *******************************************************************/
      case OP_SET_ARG: 
         IFL printf("OP_SET_ARG: %s\n",p->reg);
         if (DV_calc_arg) free(DV_calc_arg); DV_calc_arg=0;
         if (p->reg && strlen(p->reg))
            if (!(DV_calc_arg=strdup(p->reg)))
            {
               printf("execution error: out of memory\n"); eval_error_state=1;
            }
         if (p->left)
         {
            p->reg=p->left->reg;
            p->left->reg=0;
         }
         break;
      case OP_DELETE: 
         IFL printf("OP_DELETE: %s\n",p->reg);
         if (DV_calc_debug_mode) remove_sym(p->reg);
         else delete_reg(p->reg);
         break;
      case OP_SAVE: 
         IFL printf("SAVE: <%s> to <%s>\n",p->reg,p->right->reg);
         if (!DV_calc_debug_mode) instr_write_sdf(p->reg,p->right->reg,1);
         break;
      case OP_LOAD: 
         IFL printf("OP_LOAD: <%s> from <%s>\n",p->reg,p->right->reg);
         if (!DV_calc_debug_mode) instr_read_sdf(p->right->reg,0,p->reg);
         break;
      case OP_ROUTE: 
         IFL printf("OP_ROUTE: <%s> \n",p->reg);
         if (!DV_calc_debug_mode) instr_route(p->reg);
         break;
      case OP_COARSEN: 
         IFL printf("OP_COARSEN: <%s> \n",p->left->reg);
         if (!DV_calc_debug_mode)
         {
            if (!p->left->reg)
            {
               printf("execution error: nothing to coarsen\n");
            }
            else 
            {
               p->reg=get_tmp_var();
               apply_unary_gf(gf_coarsen,DV_calc_arg,p->left->reg,p->reg,DV_calc_mask,DV_calc_mask_val,DV_calc_filter);
            }
         }
         break;
      case OP_EVAL_R: 
         IFL printf("OP_EVAL_R: <%s> \n",p->left->reg);
         if (!DV_calc_debug_mode)
         {
            if (!p->left->reg)
            {
               printf("execution error: nothing to eval_r\n");
            }
            else 
            {
               p->reg=get_tmp_var();
               apply_unary_gf(gf_eval_r,DV_calc_arg,p->left->reg,p->reg,DV_calc_mask,DV_calc_mask_val,DV_calc_filter);
            }
         }
         break;
      case OP_EXIT: 
         IFL printf("OP_EXIT:\n");
         eval_error_state=-1;
         break;
      case OP_NULL_STATEMENT: 
         IFL printf("OP_NULL_STATEMENT:\n");
         break;
      default:
         if (!DV_calc_debug_mode) 
         {
            printf("execution error : no register arithmetic implemented yet (op=%i)\n",p->op); 
            eval_error_state=1;
            return;
         }
         if ((p->left->reg) || (p->right->reg)) p->reg=get_tmp_var();
         /* register-register operations */
         if ((p->left->reg) && (p->right->reg))
         {
            switch(p->op)
            {
               case '+': p->number=p->left->number+p->right->number; break;
               case '-': p->number=p->left->number-p->right->number; break;
               case '*': p->number=p->left->number*p->right->number; break;
               case '/': if (p->right->number!=0) 
                            p->number=p->left->number/p->right->number;
                         else
                         {
                            printf("WARNING: divide by zero\n");
                            printf("setting scalar to zero\n");
                            p->number=0;
                         }
                         break;
               case '^': p->number=pow(p->left->number,p->right->number); break;
               case OP_ABS: p->number=fabs(p->left->number); break;
               case OP_EXP: p->number=exp(p->left->number); break;
               case OP_LOG: if (p->number>0) 
                            p->number=log10(p->left->number); 
                         else
                         {
                            printf("WARNING: log(%lf) attempted\n",p->left);
                            printf("setting register to zero\n");
                            p->number=0;
                         }
                         break;
               case OP_LN: if (p->number>0) 
                            p->number=log(p->left->number); 
                         else
                         {
                            printf("WARNING: ln(%lf) attempted\n",p->left);
                            printf("setting register to zero\n");
                            p->number=0;
                         }
                         break;
               default: printf("operator %i not yet implemented\n",p->op);
                        eval_error_state=1;
            }
            IFL { printf("RR op: %s(%lf) = ; op=",p->reg,p->number);
                  if (p->op<256) printf("%c\n",p->op); else printf("%i\n",p->op); }
         }
         /* number-number operations, or unary number operations */
         else if (!(p->left->reg) && !(p->right->reg))
         {
            switch(p->op)
            {
               case '+': p->number=p->left->number+p->right->number; break;
               case '-': p->number=p->left->number-p->right->number; break;
               case '*': p->number=p->left->number*p->right->number; break;
               case '/': if (p->right->number!=0) 
                            p->number=p->left->number/p->right->number;
                         else
                         {
                            printf("WARNING: divide by zero\n");
                            printf("setting scalar to zero\n");
                            p->number=0;
                         }
                         break;
               case '^': p->number=pow(p->left->number,p->right->number); break;
               case OP_ABS: p->number=fabs(p->left->number); break;
               case OP_EXP: p->number=exp(p->left->number); break;
               case OP_LOG: if (p->number>0) 
                            p->number=log10(p->left->number); 
                         else
                         {
                            printf("WARNING: log(%lf) attempted\n",p->left);
                            printf("setting scalar to zero\n");
                            p->number=0;
                         }
                         break;
               case OP_LN: if (p->number>0) 
                            p->number=log(p->left->number); 
                         else
                         {
                            printf("WARNING: ln(%lf) attempted\n",p->left);
                            printf("setting scalar to zero\n");
                            p->number=0;
                         }
                         break;
               case OP_DIFF: case OP_DIFF_CN: 
                         printf("Error: cannot differentiate a scalar\n");
                         eval_error_state=1;
                         break;
               default: printf("operator %i not yet implemented\n",p->op);
                        eval_error_state=1;
            }
            IFL { if (p->op<256) printf("NN op: %lf = %lf  %c %lf\n",p->number,p->left->number,p->op,p->right->number);
                  else printf("NN op: %lf = %lf  %i %lf\n",p->number,p->left->number,p->op, p->right->number); }

         }
         /* number-register operations, or unary register operations */
         else
         {
            if (p->left->reg) 
            {
               preg=p->left; pnum=p->right;
            }
            else 
            {
               preg=p->right; pnum=p->left;
            }
            switch(p->op)
            {
               case '+': p->number=pnum->number+preg->number; break;
               case '-': p->number=pnum->number-preg->number; break;
               case '*': p->number=pnum->number*preg->number; break;
               case '/': if (p->left->reg && pnum->number!=0) 
                            p->number=preg->number/pnum->number;
                         else if (p->right->reg && preg->number!=0)
                            p->number=pnum->number/preg->number;
                         else
                         {
                            printf("WARNING: divide by zero\n");
                            printf("setting scalar to zero\n");
                            p->number=0;
                         }
                         break;
               case '^': p->number=pow(p->left->number,p->right->number); break;
               case OP_ABS: p->number=fabs(p->left->number); break;
               case OP_EXP: p->number=exp(p->left->number); break;
               case OP_LOG: if (p->number>0) 
                            p->number=log10(p->left->number); 
                         else
                         {
                            printf("WARNING: log(%lf) attempted\n",p->left);
                            printf("setting scalar to zero\n");
                            p->number=0;
                         }
                         break;
               case OP_LN: if (p->number>0) 
                            p->number=log(p->left->number); 
                         else
                         {
                            printf("WARNING: ln(%lf) attempted\n",p->left);
                            printf("setting scalar to zero\n");
                            p->number=0;
                         }
                         break;
               default: printf("operator %i not yet implemented\n",p->op);
                        eval_error_state=1;
            }
            IFL { printf("RN op: %s(%lf) = ; op=",p->reg,p->number);
                  if (p->op<256) printf("%c\n",p->op); else printf("%i\n",p->op); }
         }
   }
   if (p->left) free_tmp_var(p->left->reg);
   if (p->right) free_tmp_var(p->right->reg);
}

/*=============================================================================
   returns 0 if OK
           1 if an error occurred
          -1 upon 'exit'
  =============================================================================*/
int eval_stmnt_list()
{
   int ltrace=0;
   int num=0;
   pte *p;

   p=stmnt_list;
   eval_error_state=0;
   clear_sym_tab();
   while(p && !(eval_error_state))
   {
      num++;
      IFL printf("======== statement %i ========\n",num);
      eval_stmnt(p);
      if (eval_error_state>0) 
      {
         free_stmnt_list(); 
         printf("eval_stmnt_list: error encountered ... terminating program\n"); 
         return 1;
      }
      else if (eval_error_state<0) 
      {
         free_stmnt_list();
         IFL printf("eval_stmnt_list: stopping\n"); 
         return -1; 
      }
      p=p->next_stmnt;
   }

   return 0;
}

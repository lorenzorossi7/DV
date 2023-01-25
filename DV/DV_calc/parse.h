/* 
   functions to build and evaluate the parse trees
   that will be produced by DV_calc.y
*/
#ifndef _PARSE_H
#define _PARSE_H

/* operators
  
   +,-,^, etc ... are just the corresponding ASCII character values*/

#define OP_DECL_REGISTER 256
#define OP_DECL_CONST 257
#define OP_ASSIGN 258
#define OP_ABS 259
#define OP_EXP 260
#define OP_LOG 261
#define OP_LN 262
#define OP_DIFF 263
#define OP_DIFF_CN 264
#define OP_SET_MASK 265
#define OP_SET_MASK_VAL 266
#define OP_SET_FILTER 267
#define OP_RENAME 269
#define OP_LOAD 270
#define OP_ROUTE 271
#define OP_EXIT 272
#define OP_DELETE 273
#define OP_SAVE 274
#define OP_NULL_STATEMENT 275
#define OP_COARSEN 276
#define OP_SET_ARG 277
#define OP_EVAL_R 278

typedef struct pte
{
   struct pte *left;
   struct pte *right;
   int op;            /* operator */
   double number;     /* either register or number, but not bothering with */
   char *reg;         /* a union ... number if reg=0, though prior to evaluation */
                      /* reg may be zero even for a register. */

   struct pte *next_stmnt;   /* book keeping */
} pte;

pte *alloc_pte(pte *left, pte *right, int op, double number, char *reg);
void free_pte(pte *p);
void add_stmnt(pte *p);
void free_stmnt_list();
void eval_stmnt(pte *p);
int eval_stmnt_list();

extern char *DV_calc_input;
extern char *DV_calc_input_ptr;
extern char *DV_calc_mask;
extern char *DV_calc_filter;
extern char *DV_calc_mask;
extern char *DV_calc_arg;
extern double DV_calc_mask_val;

extern int DV_calc_debug_mode; /* if (true) then only does real arithmetic */

#endif /* _PARSE_H */

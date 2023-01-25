/*
   command line interface to DV_calc in debug mode
*/

#include "parse.h"
#include "DV_calc.tab.h"
#include <stdio.h>

/*
   the following are stub routines for the DV calls
*/

void *find_reg(char *name, int lock_rl, int free_rl)
{
   return 0;
}

int delete_reg(char *reg_name)
{
   return 0;
}

int rename_reg(char *old_name, char *new_name)
{
   return 0;
}

void *gf_clone,*gf_coarsen,*gf_eval_r;
int apply_unary_gf(void *gf, char *args, char *f, char *nf,
                   char *mask, double mask_val, char *givec)
{
   return 0;
}

void instr_read_sdf(char *file_name, int as_perim, char *new_name)
{
}

int instr_write_sdf(char *name, char *file_name, int lock_rl)
{
   return 0;
}

void instr_route(char *name)
{
}

#define MAX_DVC_ISTRING 10000
int main(int argc,char **argv)
{
  FILE *f;
  char *istring;
  int num;

  DV_calc_debug_mode=1;

  if (argc!=2) { printf("Usage:\n%s input_file\n",argv[0]); exit(1); }

  if (!(istring=(char *)malloc(sizeof(char)*MAX_DVC_ISTRING))) 
  {
     printf("error allocating %i bytes\n",MAX_DVC_ISTRING); 
     exit(1); 
  }

  if (!(f=fopen(argv[1],"r"))) { free(istring); printf("error opening file %s\n", argv[1]); exit(1); }

  num=fread(istring,sizeof(char),MAX_DVC_ISTRING-1,f);
  if (num==MAX_DVC_ISTRING-1) printf("WARNING: file too large ... truncated at %i bytes\n",num);

  istring[num]=0;

  DV_calc_input=istring;
  DV_calc_input_ptr=istring;

  DV_calcparse();
  eval_stmnt_list();
  free_stmnt_list();

  free(istring);
  fclose(f);
}

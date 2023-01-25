#ifndef _DV_USERF_TABLE_H
#define _DV_USERF_TABLE_H

//=============================================================================
// userf_table.h
//=============================================================================

#define MAX_USERF 100
#define MAX_MACRO_LINES 100
#define MAX_USERF_LINE_LENGTH 256
#define USERF_NEW 0;
#define USERF_EDIT 1;
#define MAX_USERF_INPUT_PARAMS 100
#define MAX_USERF_INFO_LENGTH 256

typedef struct
{
   char *func;
   int  ifunc;
   char *reg_a;
   char *reg_b;
   char *mask;
   char *mvalue;
   char *args;
   char *new_name;
} userf_macro_line;

typedef struct
{
   char *name;
   char *info;
   int ninput_regs;
   int last;
//   userf_macro_line macro_line[MAX_MACRO_LINES];
   userf_macro_line *macro_line[MAX_MACRO_LINES];
} userf_table_entry;


struct {
  int last;
  userf_table_entry *entry[MAX_USERF];
} userf_table;

userf_macro_line def_AB_etc;
userf_table_entry backup_entry;
int userf_mode;
int last_backed_up = -1;


#endif 

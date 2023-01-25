#ifndef _INSTR_H 
#define _INSTR_H 

#include <stdio.h>

void instr_dump_all(void);
void instr_read_sdf(char *file_name, int as_perim, char *new_name);
void instr_read_sdf_stream(FILE *stream);
int instr_write_sdf(char *name, char *file_name, int lock_rl);
void instr_route(char *name);
void instr_write_sdf_stream(char *name);
void instr_mem_usage(void);
void instr_ex_dv_calc_prog(char *prog);

#endif /* _INSTR_H */

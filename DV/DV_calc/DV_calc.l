/* 
lexical analyser for DV_calc.y

*/

%pointer

%{

#include "fncs.h"
#include "parse.h"
#include "DV_calc.tab.h"
#include <math.h>

int line_num=1;

int my_DV_calcinput(char *buf, int max_size);

#undef YY_INPUT
#define YY_INPUT(b,r,ms) (r=my_DV_calcinput(b,ms))

#undef DV_calcwrap

%}

WORD [a-zA-Z_][a-zA-Z_0-9]*
/* improved number syntax thanks to Martin:*/
NUM (([0-9]+(\.[0-9]*)?)|(\.[0-9]+))([eE][+-]?[0-9]+)?

%%

[ \t]*  /* strip whitespace */

\n { line_num++; }

mask  { return MASK; }

mask_val  { return MASK_VAL; }

load  { return LOAD; }

save  { return SAVE; }

route  { return ROUTE; }

exit  { return EXIT; }

delete  { return DELETE; }

filter  { return FILTER; }

\"[^"]*\" { DV_calclval.string=(char *)malloc(DV_calcleng-1);
            strncpy(DV_calclval.string,&DV_calctext[1],DV_calcleng-2);
            DV_calclval.string[DV_calcleng-2]=0;
            return REGISTER; }

\'[^']*\' { DV_calclval.string=(char *)malloc(DV_calcleng-1);
            strncpy(DV_calclval.string,&DV_calctext[1],DV_calcleng-2);
            DV_calclval.string[DV_calcleng-2]=0;
            return REGISTER; }

abs { DV_calclval.fnc=OP_ABS;
      return UFNC; }

coarsen { DV_calclval.fnc=OP_COARSEN;
      return UFNC; }

eval_r { DV_calclval.fnc=OP_EVAL_R;
         return UFNC; }

diff { DV_calclval.fnc=OP_DIFF;
       return BFNC; }

diff_CN { DV_calclval.fnc=OP_DIFF_CN;
          return BFNC; }

{WORD} { DV_calclval.string=(char *)malloc(DV_calcleng+1);
         strncpy(DV_calclval.string,&DV_calctext[0],DV_calcleng);
         DV_calclval.string[DV_calcleng]=0;
         return REGISTER; }

{NUM} { DV_calclval.number=atof(DV_calctext); return CONST; }

"+" { DV_calclval.op='+'; return '+'; }
"-" { DV_calclval.op='-'; return '-'; }
"*" { DV_calclval.op='*'; return '*'; }
"/" { DV_calclval.op='/'; return '/'; }
"(" { DV_calclval.op='('; return '('; }
")" { DV_calclval.op=')'; return ')'; }
";" { DV_calclval.op=';'; return ';'; }
"^" { DV_calclval.op='^'; return '^'; }
">" { DV_calclval.op='>'; return '>'; }
"=" { DV_calclval.op='='; return '='; }
"," { DV_calclval.op=','; return ','; }

. { printf("line %i:syntax error ... unrecognized character(s) %s\n",
    line_num,DV_calctext); 
    return 0; }

%%

int DV_calcwrap()
{
   int ltrace=0;

   if (ltrace) printf("DV_calcwrap: line_num=%i\n",line_num);
   if (YY_CURRENT_BUFFER) yy_flush_buffer(YY_CURRENT_BUFFER);
   return 1;
}

int my_DV_calcinput(char *buf, int max_size)
{
   int n=0,ltrace=0;

   if (ltrace)
      printf("in my_DV_calcinput: max_size=%i, \nremaining string(%i)=\n%s\n",
              max_size,strlen(DV_calc_input_ptr),DV_calc_input_ptr);

   while(n<=max_size && *DV_calc_input_ptr) { n++; *buf++=*DV_calc_input_ptr++; }

   if (ltrace) printf("n=%i\n",n);

   return n;
}

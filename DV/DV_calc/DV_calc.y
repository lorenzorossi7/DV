%{
/* Grammer for a simple calculator language; 'compiled' programs of
   which are to be executed on the DV. */

#include "fncs.h"
#include "parse.h"
#include <stdio.h>

pte *pte_1,*pte_2;

%}

%union
{
   double number;
   char *string;
   int decl;
   char op;
   int fnc;
   pte *node;
}

%token <number> CONST
%token <string> REGISTER 
%token <decl> MASK FILTER MASK_VAL LOAD SAVE EXIT ROUTE DELETE NULL_STATEMENT
%token <fnc> BFNC UFNC 

%left <op> '-' '+'
%left <op> '*' '/'
%left <op> UPM
%right <op> '^'

%type <node> expr stmnt

%%
stmnt_list:  /*nothing*/
          | stmnt_list stmnt { add_stmnt($2); }
          ;

stmnt:   REGISTER '=' expr ';' { $$=alloc_pte(0,$3,OP_ASSIGN,0,$1); }
       | REGISTER '>' REGISTER ';' { pte_1=alloc_pte(0,0,OP_DECL_REGISTER,0,$1);
                                     $$=alloc_pte(pte_1,0,OP_RENAME,0,$3); }
       | MASK '=' REGISTER ';' { pte_2=alloc_pte(0,0,OP_DECL_REGISTER,0,$3);
                                 $$=alloc_pte(0,pte_2,OP_SET_MASK,0,0); }
       | MASK_VAL '=' CONST ';' { pte_2=alloc_pte(0,0,OP_DECL_CONST,$3,0);
                                  $$=alloc_pte(0,pte_2,OP_SET_MASK_VAL,0,0); }
       | FILTER '=' REGISTER ';' { pte_2=alloc_pte(0,0,OP_DECL_REGISTER,0,$3);
                                   $$=alloc_pte(0,pte_2,OP_SET_FILTER,0,0); }
       | LOAD REGISTER '>' REGISTER ';' { pte_2=alloc_pte(0,0,OP_DECL_REGISTER,0,$2);
                                         $$=alloc_pte(0,pte_2,OP_LOAD,0,$4); }
       | LOAD REGISTER ';' { pte_2=alloc_pte(0,0,OP_DECL_REGISTER,0,$2);
                             $$=alloc_pte(0,pte_2,OP_LOAD,0,0); }
       | ROUTE REGISTER ';' { $$=alloc_pte(0,0,OP_ROUTE,0,$2); }
       | DELETE REGISTER ';' { $$=alloc_pte(0,0,OP_DELETE,0,$2); }
       | SAVE REGISTER '>' REGISTER ';' { pte_2=alloc_pte(0,0,OP_DECL_REGISTER,0,$4);
                                          $$=alloc_pte(0,pte_2,OP_SAVE,0,$2); }
       | EXIT ';' { $$=alloc_pte(0,0,OP_EXIT,0,0); }
       | ';' { $$=alloc_pte(0,0,OP_NULL_STATEMENT,0,0); }
       ;

expr: CONST { $$=alloc_pte(0,0,OP_DECL_CONST,$1,0); }
    | REGISTER { $$=alloc_pte(0,0,OP_DECL_REGISTER,0,$1); }
    | expr '+' expr { $$=alloc_pte($1,$3,$2,0,0); }
    | expr '-' expr { $$=alloc_pte($1,$3,$2,0,0); }
    | expr '/' expr { $$=alloc_pte($1,$3,$2,0,0); }
    | expr '*' expr { $$=alloc_pte($1,$3,$2,0,0); }
    | expr '^' expr { $$=alloc_pte($1,$3,$2,0,0); }
    | '-' expr %prec UPM { pte_1=alloc_pte(0,0,OP_DECL_CONST,0,0);
                           $$=alloc_pte(pte_1,$2,$1,0,0); }
    | '+' expr %prec UPM { pte_1=alloc_pte(0,0,OP_DECL_CONST,0,0);
                           $$=alloc_pte(pte_1,$2,$1,0,0); }
    | '(' expr ')' { $$=$2; }
    | UFNC '(' expr ')' { $$=alloc_pte($3,0,$1,0,0); }
    | UFNC '(' expr ',' REGISTER ')' { pte_1=alloc_pte($3,0,OP_SET_ARG,0,$5);
                                       pte_2=alloc_pte(pte_1,0,$1,0,0); 
                                       $$=alloc_pte(pte_2,0,OP_SET_ARG,0,0); }
    | BFNC '(' expr ',' expr ')' { $$=alloc_pte($3,$5,$1,0,0); }
    | BFNC '(' expr ',' expr ',' REGISTER ')' { pte_1=alloc_pte($3,0,OP_SET_ARG,0,$7);
                                                pte_2=alloc_pte(pte_1,$5,$1,0,0); 
                                                $$=alloc_pte(pte_2,0,OP_SET_ARG,0,0);
                                              }
    ;
%%

DV_calcerror(char *s)
{
   printf("%s\n", s);
}

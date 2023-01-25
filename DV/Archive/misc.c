//=============================================================================
// misc.c --- miscellaneous routines
//=============================================================================

#include "misc.h"
#include <math.h>

// fuzzy logic
// 'fuzz' is assumed positive

int fuzz_eq(double x1, double x2, double fuzz)
{
   if (fabs(x2-x1)<fuzz) return 1; else return 0;
}
 
int fuzz_lt(double x1, double x2, double fuzz)
{
   if (x1<-fuzz+x2) return 1; else return 0;
}
 
int fuzz_lte(double x1, double x2, double fuzz)
{
   if (x1<=fuzz+x2) return 1; else return 0;
}

int fuzz_gt(double x1, double x2, double fuzz)
{
   if (x1>fuzz+x2) return 1; else return 0;
} 

int fuzz_gte(double x1, double x2, double fuzz)
{
   if (x1>=-fuzz+x2) return 1; else return 0;
} 

const char *first_c(const char *s, const char c)
{
   while(*s && *s!=c) *s++;
   if (*s==c) return s; else return 0;
}

const char *next_c(const char *s, const char c)
{
   while(*s==' ') *s++;
   if (*s==c) return s; else return 0;
}

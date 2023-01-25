#ifndef _MISC_H
#define _MISC_H 1
//========================================================================================
// misc.h 
//
// miscellaneous stuff
//========================================================================================

#define IFL if (ltrace)

int fuzz_eq(double x1, double x2, double fuzz);
int fuzz_lt(double x1, double x2, double fuzz);
int fuzz_lte(double x1, double x2, double fuzz);
int fuzz_gt(double x1, double x2, double fuzz);
int fuzz_gte(double x1, double x2, double fuzz);

#define max(a, b)  (((a) > (b)) ? (a) : (b)) 
#define min(a, b)  (((a) < (b)) ? (a) : (b)) 

const char *first_c(const char *s, const char c);
const char *next_c(const char *s, const char c);

#endif // _MISC_H

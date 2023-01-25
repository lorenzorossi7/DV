#ifndef _S_ITER_H
#define _S_ITER_H
//-----------------------------------------------------------------------------
// s_iter.h --- sequential iterator for a (locked) register
//-----------------------------------------------------------------------------

#include "reg.h"

typedef struct
{
   reg *r,*r_s;
   time_str *ts,*ts_s;
   level *l,*l_s;
   grid *g,*g_s;
   int use_givec;
   int selected;     // if a givec is used --- see 'siter.c'
                     // NOTE: The remaining members of the structure are ONLY valid
                     // if a givec is used (to use without a givec, pass 
                     // some empty string to init_s_iter, like " "):
   int t_selected;
   int l_selected;
   int tn,tn_s;      // tn=time index, starting at 1
   int ln,ln_s;      // ln=global (for register) level index, starting at 1
   int num_t;        // total number of times
   int num_l;        // total number of levels 
   int use_bbox;     
   double cbbox[2*MAX_DIM];
   int ibbox[2*MAX_DIM];
} s_iter;

#define IS_IT_VALID(it) ((it)->r)
#define SET_IT_INVALID(it) (it)->r=0

#define GIV_ON 0
#define GIV_OFF 1
#define GIV_CLIP 2

#define CBBOX 1
#define IBBOX 2

grid *init_s_iter(s_iter *it, reg *r, const char *givec);
grid *next_g(s_iter *it);
time_str *next_ts(s_iter *it);
level *next_l(s_iter *it);
void save_s_iter(s_iter *it);
grid *restore_s_iter(s_iter *it);
void set_ibbox(s_iter *it);

#endif // _S_ITER_H

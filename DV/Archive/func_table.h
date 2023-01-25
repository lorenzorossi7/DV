#ifndef _DV_FUNC_TABLE_H
#define _DV_FUNC_TABLE_H

#include "fncs.h" 
//=============================================================================
// func_table.h
//=============================================================================

#define TYPE_UNARY_FUNC 1
#define TYPE_BINARY_FUNC 2
#define TYPE_NULL_FUNC 3
#define TYPE_RENAME 4
#define TYPE_MERGE 5
#define TYPE_SPECIAL 6

typedef struct
{
   const char *name;
   const char *info;
   int func_type;
   void *func;       // later cast to appropriate type
   char *new_func_ps;
} func_table_entry;

#define MAX_FUNCS 100
func_table_entry func_table[MAX_FUNCS]=
{
   { "A+B","",TYPE_BINARY_FUNC,gf_add,"(%s+%s)" },
   { "A-B","",TYPE_BINARY_FUNC,gf_sub,"(%s-%s)" },
   { "A*B","",TYPE_BINARY_FUNC,gf_mult,"(%s*%s)" },
   { "A/B","",TYPE_BINARY_FUNC,gf_div,"(%s/%s)" },
   { "A*c","argument is constant c",TYPE_UNARY_FUNC,gf_smult,"(%s*c)" },
   { "A+c","argument is constant c",TYPE_UNARY_FUNC,gf_sadd,"(%s+c)" },
   { "A^p","argument is power p",TYPE_UNARY_FUNC,gf_pow,"(%s^p)" },
   { "abs(A)","",TYPE_UNARY_FUNC,gf_abs,"abs(%s)" },
   { "exp(A)","",TYPE_UNARY_FUNC,gf_exp,"exp(%s)" },
   { "ln(A)","if A(x)<=0, then A(x) is set to 0",TYPE_UNARY_FUNC,gf_ln,"ln(%s)" },
   { "log(A)","if A(x)<=0, then A(x) is set to 0",TYPE_UNARY_FUNC,gf_log,"log(%s)" },
   { "x*c","scale coordinates by a vector constant c\n"
           "argument is ct,cx1,cx2,cx3",TYPE_UNARY_FUNC,gf_smult_x,"(x(%s)*c)" },
   { "x+c","translate coordinates by a vector constant c\n"
           "argument is ct,cx1,cx2,cx3",TYPE_UNARY_FUNC,gf_sadd_x,"(x(%s)+c)" },
   { "unit(x(A))","replace the bounding box of A with a unit cube",TYPE_UNARY_FUNC,gf_unit_bbox,"unit(%s)" },
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "clone","",TYPE_UNARY_FUNC,gf_clone,"copy of %s" },
   { "rename","",TYPE_RENAME,0,"%s" },
   { "merge(A,B)","",TYPE_MERGE,0,"%s_%s" },
   { "coordinate(A)",
     "Argument tells which coordinate to convert\nspecify coordinate via its name, or `x1',... "
     ,TYPE_UNARY_FUNC,gf_coords,"x" },
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "2:1 coarsen (A)","note: assumes FORTRAN style array indexing",TYPE_UNARY_FUNC,gf_coarsen,"(2:1[%s]))" },
   { "bound(A)","arg is 'min,max' --- sets values of A > max to max,\nand values < min to min",
     TYPE_UNARY_FUNC,gf_bound,"bound(%s)" },
   { "l2norm(A)","",TYPE_UNARY_FUNC,gf_l2norm,"---" },
   { "infnorm(A)","",TYPE_UNARY_FUNC,gf_infnorm,"---" },
   { "min(A)","",TYPE_UNARY_FUNC,gf_min,"min(%s)"},
   { "max(A)","",TYPE_UNARY_FUNC,gf_max,"max(%s)"},
   { "extremum(A)","",TYPE_UNARY_FUNC,gf_extremum,"extremum(%s)"},
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "2Dcluster(A)","arguments:threshold(float), minwidth(int>=0),\n"
     "buf_wid(int>=0), min_eff(0<float<=1), blk_fac(int>=1)",
     TYPE_UNARY_FUNC,gf_dc_2dcls,"clusters" },
   { "2Dinterpolate(A)","argument: fine-to-coarse ratio (default=2)",
     TYPE_UNARY_FUNC,gf_2dinterpolate,"interp(%s)" },
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "r(A)","graxi specific. calculates r=sqrt(rho^2+z^2)",TYPE_UNARY_FUNC,gf_r,"r" },
   { "int[(A^p)*PL(l,x)] dS","graxi specific. integrates (A^p)*PL(l,x) along an r=constant\nsurface (flat metric)\n"
      "args=r,p,l; PL(l,x) are the Legendre polynomials",TYPE_UNARY_FUNC,gf_int_fds,"---" },
   { "de_dt(psi4)","graxi specific. argument is radius r\nreturns psi4 integrated in time"
      ,TYPE_UNARY_FUNC,gf_de_dt,"int(%s,t)" },
   { "(A*r)^p","graxi specific. Argument is power p [def=1]",TYPE_UNARY_FUNC,gf_multr_p,"([%s*r]^p)" },
   { "mass_dens(psi)","graxi specific. calculates Brill mass density",TYPE_UNARY_FUNC,gf_mass_dens,"md(%s)" },
   { "fix_sf(A)","graxi specific. <overwrites excisting function>",TYPE_UNARY_FUNC,gf_fix_sf,"---"},
   { "fix_mc_mr(A)","graxi specific. (A*r)^p. arg1=time rescale, arg2=p (set p=0 to ignore)\n<overwrites excisting function>",TYPE_UNARY_FUNC,gf_fix_mc_mr,"---"},
   { "apply_dis_ex_bdy","graxi specific. apply_dis_ex_bdy(f,f_surf) \narg is the number of times to apply",
     TYPE_BINARY_FUNC,gf_apply_dis_ex_bdy,"(dis(%s,%s))" },
   { "extend (A,mask)","graxi specific. arg is `tn,n,a,clip[0|1],k,kcell,eps,first_n'"
     ": n is # of sweeps per tn'th iteration, stencil 'size' k\na=0 --> f(z=0)=0\n"
     "a=1 --> f,rho(z=0)=0\na=2 --> just smooth f(z=0)",TYPE_UNARY_FUNC,gf_extend,"extend(%s)" },
   { "distance_2BH (psi,mask)", "graxi specific.", TYPE_UNARY_FUNC,gf_distance_2BH,"(null(%s))" },
   { "diss_ko2(A,mask)", "graxi specific. arg is `n-times,eps,k,axis'\n"
     "axis: =0 odd, =1 even, =2 whatever",TYPE_UNARY_FUNC,gf_diss_ko2,"diss_ko2(%s)" },
   { "uni_avg(A,mask)", "graxi specific. arg is `n,axis'\n"
     "axis: =0 odd, =1 even, =2 whatever",TYPE_UNARY_FUNC,gf_uni_avg,"uni_avg(%s)" },
   { "d_overrho (A,mask)", "graxi specific.", TYPE_UNARY_FUNC,gf_d_overrho,"d_overrho(%s)" },
   { "d_overrho_d (A,mask)", "graxi specific.", TYPE_UNARY_FUNC,gf_d_overrho_d,"d_overrho_d(%s)" },
   { "d_byrhosq (A,mask)", "graxi specific.", TYPE_UNARY_FUNC,gf_d_byrhosq,"d_byrhosq(%s)" },
   { "(r,theta)->(ln(r+c),theta)", "graxi specific. Argument is c", TYPE_UNARY_FUNC,gf_r_to_lnr,"%s(ln(r))" },
   { "separate_rb (A)", "2D specific.", TYPE_UNARY_FUNC,gf_separate_rb,"rb_sep(%s)" },
   { "null_int", "graxi_ad specific.\narg is t0,t1,steps,log_offset[,eps_dis].\neps_dis::\n 0/3/default --- 5 pnt uniform\n"
                 " 0-1: KO   2: 3pnt uniform\n 4:5 pnt weighted   5: none", TYPE_SPECIAL,sf_null_int,"---" },
   { "ln_vt", "graxi_ad specific.\narg is reg,n,t*,min(ln(t*-t)),nl,dt\n",TYPE_SPECIAL,sf_ln_vt,"---"},
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "A(B,z)","bs specific. given a uniform grid A(x,z), attaches a non-uniform\nradial coordinate specified via"
              "\nthe 2nd argument B=r(x) (default r_of_x)\nFirst argument is an r-truncation length",
              TYPE_UNARY_FUNC,gf_bs_attach_nu,"%s(r(x),z)" },
   { "d(A)/dz","bs specific. calculates d(A)/dz assuming periodicity in z",TYPE_UNARY_FUNC,gf_bs_ddz,"d(%s)/dz" },
   { "1/r^2/a^2-1/r^2","bs specific.",TYPE_UNARY_FUNC,gf_bs_f1,"hal" },
   { "","",0,0,"" } // denotes end of list
};
       

#endif // _FUNC_TABLE_H 

#ifndef _DV_FUNC_TABLE_H
#define _DV_FUNC_TABLE_H

#include "fncs.h" 
/*=============================================================================*/
/* func_table.h                                                                */
/*=============================================================================*/

#define TYPE_UNARY_FUNC 1
#define TYPE_BINARY_FUNC 2
#define TYPE_NULL_FUNC 3
#define TYPE_RENAME 4
#define TYPE_MERGE 5
#define TYPE_SPECIAL 6
#define TYPE_DELETE 7

typedef struct
{
   const char *name;
   const char *info;
   int func_type;
   void *func;       /* later cast to appropriate type */
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
   { "sin(A)","",TYPE_UNARY_FUNC,gf_sin,"sin(%s)" },
   { "cos(A)","",TYPE_UNARY_FUNC,gf_cos,"cos(%s)" },
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
   { "delete","",TYPE_DELETE,0,"" },
   { "merge(A,B)","",TYPE_MERGE,0,"%s_%s" },
   { "A(B)","Attaches B as a semi-uniform coordinate system to A",TYPE_BINARY_FUNC,gf_A_vs_B,"%s(%s)" },
   { "coordinate(A)",
     "Argument tells which coordinate to convert\nspecify coordinate via its name, or `x1',... "
     ,TYPE_UNARY_FUNC,gf_coords,"x" },
   { "(r,theta)->(x,y)", "Attach 2D curvilinear coordinates via:\n"
                         "x=r*sin(theta), y=r*cos(theta).", TYPE_UNARY_FUNC,gf_rt_to_xy,"%s(x,y)" },
   { "A->uniform/unigrid",
     "Argument is 'x1,x2,Nx,y1,y2,Ny,z1,z2,Nz'\n(uses linear interpolation)",
     TYPE_UNARY_FUNC,gf_to_uniform_unigrid,"uniform/unigrid(%s)" },
   { "trim(A)", "Shrinks the grid by n (default 1) points around the boundary\n",
     TYPE_UNARY_FUNC,gf_trim,"trim(%s)" },
   { "zero_bnd(A)", "Zeros grid in a zone n (default 1) points around the boundary\n",
     TYPE_UNARY_FUNC,gf_zero_bnd,"zero_bnd(%s)" },
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "2:1 coarsen (A)","note: assumes FORTRAN style array indexing",TYPE_UNARY_FUNC,gf_coarsen,"(2:1[%s]))" },
   { "bound(A)","arg is 'min,max' --- sets values of A > max to max,\nand values < min to min",
     TYPE_UNARY_FUNC,gf_bound,"bound(%s)" },
   { "l2norm(A)","",TYPE_UNARY_FUNC,gf_l2norm,"l2norm(%s)"},
   { "infnorm(A)","",TYPE_UNARY_FUNC,gf_infnorm,"infnorm(%s)"},
   { "min(A)","",TYPE_UNARY_FUNC,gf_min,"min(%s)"},
   { "max(A)","",TYPE_UNARY_FUNC,gf_max,"max(%s)"},
   { "extremes(A)","",TYPE_UNARY_FUNC,gf_extremum,"extremes(%s)"},
   { "eval(A)","Option 1: with Arguments x0,y0,z0, evaluate A at a point (x0,y0,z0);\n"
     "returns 1D register where x1=time.\n" 
     "Option 2: with x0,y0,z0,x1,y1,z1,N, evaluate A along a line at N\n"
     "equally spaced points between (x0,y0,z0) and (x1,y1,z1);\n"
     "returns a sequence (in t) of 1D registers.",
     TYPE_UNARY_FUNC,gf_eval,"eval(%s)"},
   { "t->-ln(t*-t)", "arg is t*",TYPE_UNARY_FUNC,gf_ln_ts_m_t,"%s(-ln(t*-t))"},
   { "t(A)->lambda(t(A))", "arg is lambda(t), a non-uniform 1D register\ndefining the time transformation",
     TYPE_UNARY_FUNC,gf_t_to_lt,"%s(lambda(t))"},
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
/*   { "fix_mc_mr(A)","graxi specific. (A*r)^p. arg1=time rescale, arg2=p (set p=0 to ignore)\n<overwrites excisting function>", */
/*     TYPE_UNARY_FUNC,gf_fix_mc_mr,"---"}, */
   { "apply_dis_ex_bdy","graxi specific. apply_dis_ex_bdy(f,f_surf) \narg is the number of times to apply",
     TYPE_BINARY_FUNC,gf_apply_dis_ex_bdy,"(dis(%s,%s))" },
   { "extend (A,mask)","graxi specific. arg is `tn,n,a,clip[0|1],k,k_axis,kcell,eps,first_n'"
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
   { "(r,theta)->(ln(r+c),theta)", "graxi specific. Argument is c,[flip]", TYPE_UNARY_FUNC,gf_r_to_lnr,"%s(ln(r))" },
   { "separate_rb (A)", "2D specific.", TYPE_UNARY_FUNC,gf_separate_rb,"rb_sep(%s)" },
/*   { "null_int", "graxi_ad specific.\narg is t0,t1,steps,log_offset[,eps_dis].\neps_dis::\n 0/3/default --- 5 pnt uniform\n" */
/*                 " 0-1: KO   2: 3pnt uniform\n 4:5 pnt weighted   5: none", TYPE_SPECIAL,sf_null_int,"---" }, */
/*   { "ln_vt", "graxi_ad specific.\narg is reg,n,t*,min(ln(t*-t)),nl,dt\n",TYPE_SPECIAL,sf_ln_vt,"---"}, */
   { "gd_concat[rho,z]", "graxi_ad specific.\narg is L,phase,t0,z_reg",TYPE_SPECIAL,sf_gd_concat,"---"},
   { "gd_concat[null]", "graxi_ad specific.\narg is t0,eps,scale_type,gd_n,tau*,lambda_max,z_reg\n"
                        "t0 an integer <0 specifies output every -t0 'steps'"
                        "\nscale_type=1 use tau* and gd_n; =2 use min/max of z_ref",
           TYPE_SPECIAL,sf_gd_concat_b,"---"},
   { "gd_concat[n]", "graxi_ad specific.\narg is n0,use_lambda(1/0),z_reg",TYPE_SPECIAL,sf_gd_concat_c,"---"},
   { "x(tp)->x(-ln(tp*-tp))", "graxi_ad specific.\narg is tp*,search_n,skip_n,trace(0:extrema,1:all,-1:none)",
              TYPE_UNARY_FUNC,gf_ln_tps_m_tp,"%s(-ln(tp*-tp))"},
   { "local_max(A)", "~graxi_ad specific. A is 1D",TYPE_UNARY_FUNC,gf_local_max,"local_max(%s)"},
   { "spec_coeff(A)", "graxi specific. Argument is l", TYPE_UNARY_FUNC,gf_spec_coeff,"spec(%s)" },
   { "J_int(A)", "graxi specific. Argument is J_dens", TYPE_SPECIAL,sf_j_int,"---" },
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "R(theta,phi)->Z(x,y)", "gh3d specific. arg is csphere,particles,p_col,file_name\n"
     "csphere!=0 transform to a coord. sphere\nfile_name: file containing time(col=1) and\n"
     "position (col=p_col) information", TYPE_UNARY_FUNC,gf_rtp_to_zxy,"Z(%s)" },
   { "dmdiss3d(A,mask)", "gh3d specific. arg is n-times,eps,do_bdy,phys_bdy[6]\n"
                         "phys_bdy=1 even, 2 odd",TYPE_UNARY_FUNC,gf_dmdiss3d,"dmdiss3d(%s)" },
   { "dmrepop3d(A,mask)", "gh3d specific. arg is interpolation order(2,3 or 4)\n",TYPE_UNARY_FUNC,gf_dmrepop3d,"dmrepop3d(%s)" },
   { "multbyrp(A)", "gh3d specific. multiply by r^p ... arg is p",TYPE_UNARY_FUNC,gf_multbyrp,"(%s)*r^p" },
   { "uncompact(A)", "gh3d specific. uncompactifies A",TYPE_UNARY_FUNC,gf_uncompact,"%s_bar" },
   { "eval_r(A,mask)", "gh3d specific. evaluate A along an r=const sphere\narg is r,Ntheta,Nphi",TYPE_UNARY_FUNC,gf_eval_r,"%s_at_r0" },
   { "int_psi4_r(A)", "gh3d specific. A is as produced by eval_r\narg 'r,[0..3]',r is *same* value of r used in eval_r,\n "
                      "0 ... energy, 1,2,3 for x,y,z component of momentum\n (3D only as of now)",TYPE_UNARY_FUNC,gf_int_psi4_r,"de_dt(%s)" },
   { "smooth_ah_r(A)", "gh3d specific. if arg is 1 saves ah, else var-array\n",TYPE_UNARY_FUNC,gf_smooth_ah_r,"AH_s(%s)"},
   { "cylm(A,B)","gh3d specific. arg=l,m,range,tag\ncomputes l,m spectral coef. of A+i*B",TYPE_BINARY_FUNC,gf_cylm,"cylm(%s,%s)" },
   { "cm2yl6m(A,B)","gh3d specific. arg=filter_range,scale_m,scale_r,\n  test_l,test_m,tag\ncomputes spin weight -2, l=2..6, m(=+-2) spectral coef. of A+i*B",TYPE_BINARY_FUNC,gf_cm2yl6m,"cm2yl6m(%s,%s)" },
   { "int_psi4_spec_r(A)", "gh3d specific. takes input from cylm\narg is *same* value of r used in eval_r",
     TYPE_UNARY_FUNC,gf_int_psi4_spec_r,"de_dt(%s)" },
   { "bbh_t_to_v(A)", "gh3d specific. takes 1D input function, and converts t (x) to v\n"
                      "arguments MT,r0,scale,pcol,pfile\npfile is data file with t in pos 1,\nBH coord position in col pcol",
     TYPE_UNARY_FUNC,gf_bbh_t_to_v,"%s(t->v)" },
   { "bbh_quad(A)", "gh3d specific. calculates quadrupole waves from input trajectory\n"
                      "arguments r0,m1,m2,scale,p_col,m_col,Ntheta,Nphi,pfile1 pfile2\n"
                      "scale: 0 none, 1: all const, 2: var scale, const t, 3: all var",
     TYPE_SPECIAL,gf_bbh_quad,""},
   { "gf_psi4_to_h(A)", "gh3d specific. arg = i,j,extrap,m0,r0,do_eps,flip,shift,tag\n",TYPE_UNARY_FUNC,gf_psi4_to_h,"extrap(%s)"},
   { "diss_ko_1d(A)", "gh3d specific. arg is eps,k_max,num\n",TYPE_UNARY_FUNC,gf_diss_ko_1d,"smooth(%s)"},
   { "overlap", "gh3d specific. \narg is n\nassumes Y1r, Y1i, Y2r, Y2i exist",TYPE_SPECIAL,sf_overlap,"---"},
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "A_vs_B(x->t)", "g2sc specific. \narg is x1,x2,n,n_ref_circle",TYPE_BINARY_FUNC,gf_A_vs_B_x_to_t,"%s(%s)"},
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "A(B,z)","bs specific. given a uniform grid A(x,z), attaches a non-uniform\nradial coordinate specified via"
              "\nthe 2nd argument B=r(x) (default r_of_x)\nFirst argument is an r-truncation length",
              TYPE_UNARY_FUNC,gf_bs_attach_nu,"%s(r(x),z)" },
   { "d(A)/dz","bs specific. calculates d(A)/dz assuming periodicity in z",TYPE_UNARY_FUNC,gf_bs_ddz,"d(%s)/dz" },
/*   { "1/r^2/a^2-1/r^2","bs specific.",TYPE_UNARY_FUNC,gf_bs_f1,"hal" }, */
   { "AH_embed(pdr,pdz)","bs specific.",TYPE_BINARY_FUNC,gf_bs_embed,"embed(AH)" },
   { "AH_embed2(area,pdw)","bs specific. arg is dt,tag. dt>0 outputs only every dt. tag is for bubble/string segments",TYPE_BINARY_FUNC,gf_bs_embed2,"embed2(AH)" },
   { "AH_embed3(area,pdw)","bs specific. produces 2D map of embedding diagram, i.e. AH_R(z,t)",TYPE_BINARY_FUNC,gf_bs_embed3,"AH_areal_r(zp,t)" },
   { "AH_shift(embed)","bs specific. argument is areal r-shift register",TYPE_UNARY_FUNC,gf_bs_shift_embed,"shifted(%s)" },
   { "First NZ(A)","bs specific. A is 2D masked function ... returns 1D function on mask bdy",TYPE_UNARY_FUNC,gf_bs_first_nz,"first_nz(%s)" },
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "half_spin(A)","Spins 1D register about the axis (1/2 way)\nargument is number of segments,repetitions",TYPE_UNARY_FUNC,gf_half_spin,"spun(%s)" },
   { "gamma(A,B)","dark side specific.",TYPE_BINARY_FUNC,gf_ds_gamma,"gamma(%s,%s)" },
   { "@-----------------------","",TYPE_NULL_FUNC,0,"" },
   { "inspiral","arg is 'm2,r0,dt,tf,nx'\n'masses' m1=1, m2. r0=initial seperation\n"
                "dt=time step. tf=final time\nnx is size of spatial (x and y) grid"
     ,TYPE_SPECIAL,sf_inspiral,"" },
   { "gw_vis(A)", "visualization of a monochromatic GW on a lattice of geodesics.\n"
                      "arguments [A,omega,Nxy,Nz,Dxy,Dz,name]",
     TYPE_SPECIAL,gf_gw_vis,""},
   { "","",0,0,"" } /* denotes end of list */
};
       

#endif /* _FUNC_TABLE_H */

#ifndef _FNCS_H 
#define _FNCS_H 
//-----------------------------------------------------------------------------
// fncs.h
//-----------------------------------------------------------------------------

#include "s_iter.h"

//-----------------------------------------------------------------------------
// function prototypes for standard unary/binary grid functions
// s_iter is passed instead of a grid structure for those functions that might 
// need the current time, parse a coordinate argument, etc...
//
// SPECIAL_FNC does everything itself (find the registers, sync them ... whatever)
//----------------------------------------------------------------------------
typedef grid *(*UNARY_FNC) (s_iter *it_a, grid *mask, double mask_val, char *nf, 
                            char *args, int first_call); 
typedef grid *(*BINARY_FNC) (s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, 
                             char *nf, char *args, int first_call); 
typedef void (*SPECIAL_FNC) (char *args);

int apply_unary_gf(UNARY_FNC gf, char *args, char *f, char *nf, 
                   char *mask, double mask_val, char *givec);
int apply_binary_gf(BINARY_FNC gf, char *args, char *f1, char *f2, char *nf, 
                    char *mask, double mask_val, char *givec);

//-----------------------------------------------------------------------------
// unary functions
//-----------------------------------------------------------------------------
grid *s_diff(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *s_diff_CN(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_smult(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_sadd(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_smult_x(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_sadd_x(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_pow(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_abs(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_exp(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_ln(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_log(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_coarsen(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_bound(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_clone(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_coords(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_l2norm(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_infnorm(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_extremum(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_min(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_max(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_int_fds(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_unit_bbox(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);

grid *gf_dc_2dcls(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_2dinterpolate(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);

grid *gf_r(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_multr_p(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_fix_sf(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_fix_mc_mr(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_extend(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_diss_ko2(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_uni_avg(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_distance_2BH(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_d_overrho(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_d_overrho_d(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_d_byrhosq(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_de_dt(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_r_to_lnr(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_separate_rb(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_mass_dens(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);

grid *gf_bs_ddz(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_bs_f1(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_bs_attach_nu(s_iter *it_a, grid *mask, double mask_val, char *nf, char *args, int first_call);

//-----------------------------------------------------------------------------
// binary functions
//-----------------------------------------------------------------------------
grid *gf_add(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_sub(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_div(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call);
grid *gf_mult(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call);

int merge_gf(char *f1, char *f2, char *nf, char *givec);

grid *gf_apply_dis_ex_bdy(s_iter *it_a, s_iter *it_b, grid *mask, double mask_val, char *nf, char *args, int first_call);

void sf_null_int(char *args);
void sf_ln_vt(char *args);

//-----------------------------------------------------------------------------
// t derivative
//-----------------------------------------------------------------------------
int t_diff(char *f, char *mask, double mask_val);

//-----------------------------------------------------------------------------
// the following functions are in rnpl_diff/[1|2|3]d/updates?.f ==>
// they differentiate uniform grid functions
//-----------------------------------------------------------------------------

void upd_f10__(double *f1,double *mask1,double *f1_x1,int *g1_Nx1,double *dx1,double *mask_val);
void upd_f11__(double *f1,double *mask1,double *f1_x1,int *g1_Nx1,double *dx1,double *mask_val);

void upd_f20__(double *f2,double *mask2,double *f2_x1,int *g1_Nx1,int *g1_Nx2,double *dx1,double *mask_val);
void upd_f21__(double *f2,double *mask2,double *f2_x2,int *g1_Nx1,int *g1_Nx2,double *dx2,double *mask_val);
void upd_f22__(double *f2,double *mask2,double *f2_x1,int *g1_Nx1,int *g1_Nx2,double *dx1,double *mask_val);
void upd_f23__(double *f2,double *mask2,double *f2_x2,int *g1_Nx1,int *g1_Nx2,double *dx2,double *mask_val);

void upd_f30__(double *f3,double *mask3,double *f3_x1,int *g1_Nx1,int *g1_Nx2,int *g1_Nx3,double *dx1,double *mask_val);
void upd_f31__(double *f3,double *mask3,double *f3_x2,int *g1_Nx1,int *g1_Nx2,int *g1_Nx3,double *dx2,double *mask_val);
void upd_f32__(double *f3,double *mask3,double *f3_x3,int *g1_Nx1,int *g1_Nx2,int *g1_Nx3,double *dx3,double *mask_val);
void upd_f33__(double *f3,double *mask3,double *f3_x1,int *g1_Nx1,int *g1_Nx2,int *g1_Nx3,double *dx1,double *mask_val);
void upd_f34__(double *f3,double *mask3,double *f3_x2,int *g1_Nx1,int *g1_Nx2,int *g1_Nx3,double *dx2,double *mask_val);
void upd_f35__(double *f3,double *mask3,double *f3_x3,int *g1_Nx1,int *g1_Nx2,int *g1_Nx3,double *dx3,double *mask_val);

void get_single_farg(double *val, char *args, int first_call, double def, char *fname);

#endif // _FNCS_H

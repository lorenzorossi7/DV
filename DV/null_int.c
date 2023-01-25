//=============================================================================
// null_int.c
// 
// graxi_ad specific null integration (AND OTHER routines that do not 
// easily operate as 'grid' functions). 
//
// see /d/godel/usr2/people/fransp/tensor_211/geod
// for the maple routines giving the e.o.m. (OLDER)
// 
//=============================================================================

#include "reg.h"
#include "fncs.h"
#include "misc.h"
#include "s_iter.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef DARWIN
#include <malloc.h>
#endif 
#include <math.h>

//=============================================================================
// evaluates a 1D function f at (x,t(ts)), via linear interpolation on
// the finest level containing x. if (df), also computes the derivative
// there.
//
// returns 0 if no grid contains x
// if (dx), *dx is set to the value of dx of the grid used 
//
// ASSUMES UNIFORM COORDINATE TYPE
//=============================================================================
int N_eval_f_1d(time_str *ts, double x, double *dx, double *f, double *df)
{
   level *l;
   grid *g,*cg=0;
   int i,nx;
   double x0,x1,cdx,w1,w2,df1,df2;
   int ltrace=0;

   IFL printf("N_eval_f_1d: t=%f, x=%f\n",ts->time,x);

   l=ts->levels;
   while(l)
   {
      g=l->grids;
      while(g)
      {
         if (g->coords[0]<=x && g->coords[1]>=x) cg=g;
         g=g->next;
      }
      l=l->next;
   }

   if (!cg) return 0;

   nx=cg->shape[0];
   x0=cg->coords[0]; x1=cg->coords[1];
   cdx=(x1-x0)/(nx-1);
   if (dx) *dx=cdx;
   i=(x-x0)/cdx;

   if (i>=(nx-1)) 
   {
      *f=cg->data[nx-1];
      if (df && nx>3) *df=(3*cg->data[nx-1]-4*cg->data[nx-2]+cg->data[nx-3])/2/cdx;
      return 1; 
   }

   w1=((i+1)*cdx+x0)-x;
   w2=x-(i*cdx+x0);
   *f=(w2*cg->data[i+1]+w1*cg->data[i])/cdx;

   if (df && nx>3)
   {
      if (i==0) df1=(-3*cg->data[0]+4*cg->data[1]-cg->data[2])/2/cdx;
      else df1=(cg->data[i+1]-cg->data[i-1])/2/cdx;

      if (i==(nx-2)) df2=(3*cg->data[nx-1]-4*cg->data[nx-2]+cg->data[nx-3])/2/cdx;
      else df2=(cg->data[i+2]-cg->data[i])/2/cdx;

      *df=(w1*df1+w2*df2)/cdx;
   }

   IFL printf("N_eval_f_1d: f=%f, df=%f\n",*f,*df);
      
   return 1;
}

//=============================================================================
// the following operation is currently `illegal`, though specifying a 
// gridcmp sort order could be introduced as part of the virtual machine
// specification.
//=============================================================================
void reverse_times(reg *r)
{
   time_str *prev,*next,*ts;

   ts=r->ts;

   while(ts)
   {
      next=ts->next;
      ts->next=ts->prev;
      ts->prev=next;
      prev=ts;
      ts=next;
   }

   r->ts=prev;
}

//=============================================================================
#define ALPHA_RHO0 0
#define BZ_RHO0 1
#define BRHO_RHO_RHO0 2
#define PSI_RHO0 3
#define PHI_RHO0 4
#define ALPHA_Z0 5
#define BRHO_Z0 6
#define BZ_Z_Z0 7
#define PSI_Z0 8
#define PHI_Z0 9
#define OMEGABAR_Z0 10
#define NUM_VARS 11

void sf_null_int(char *args)
{
   double t0=0,t,rho_i,z_i,cpt,fn[NUM_VARS],fnp1[NUM_VARS],dt,rho_dot,z_dot,rho_dotdot,z_dotdot;
   double f_t[NUM_VARS],f[NUM_VARS],f_x[NUM_VARS],fn_x[NUM_VARS],fnp1_x[NUM_VARS],x;
   double fnp2[NUM_VARS],fnp3[NUM_VARS],fnp4[NUM_VARS],fnm1[NUM_VARS];
   double fnm2[NUM_VARS];
   double *rho,*z,*phi_z0,*phi_rho0,*lambda_rho0,*lambda_z0,*ev_test,*diff,*mem=0;
   double dlambda_dt_z0,dlambda_dt_rho0,drho_dt,dz_dt,lambda_z0_i,lambda_rho0_i,lambda;
   double phi_rho0_i,f1,f2,prev_t,nprev_t,dx;
   double eps_dis=0,dtp1,dtp2,dtm1,dtm2,y;
   double f_tp1,f_tm1,f_tp2,f_tm2;
   int eps,reversed=0,can_dis,ltrace=0;
   reg *r[NUM_VARS];
   time_str *ts[NUM_VARS],*ts_np1[NUM_VARS],*qts;
   char *names[NUM_VARS]=
      { "rho0_slice_alphaN", "rho0_slice_bzN", "rho0_slice_brho_rhoN", "rho0_slice_psiN", "rho0_slice_phiN", 
        "z0_slice_alphaN", "z0_slice_brhoN", "z0_slice_bz_zN", "z0_slice_psiN", "z0_slice_phiN", "z0_slice_omegabarN" };
   grid *g;
   int i,num,n,n2,steps;
   double t1,gdt,log_offset;

   for (i=0;i<NUM_VARS;i++) r[i]=0;

   if (args) sscanf(args,"%lf,%lf,%i,%lf",&t0,&t1,&steps,&log_offset,&eps_dis);
   if (eps_dis<=0) eps_dis=3; // default ... seems to work best in regions of interest
   if (steps<1) steps=1;
   if (steps>1 && (t0*t1)<0) steps=1;
   if (log_offset<=0) log_offset=1.0e-5;
   gdt=(t1-t0)/steps;

   if (eps_dis>1 && eps_dis <3) { printf("using 3-point uniform average for smoothing\n"); eps_dis=2; }
   if (eps_dis>2 && eps_dis <4) { printf("using 5-point uniform average for smoothing\n"); eps_dis=3; }
   if (eps_dis>3 && eps_dis <5) { printf("using 5-point weighted average for smoothing\n"); eps_dis=4; }
   if (eps_dis>4 && eps_dis <6) { printf("no smoothing\n"); eps_dis=0; }

   printf("sf_null_int: t0=%f, t1=%f, steps=%i, log_offset=%f, eps_dis=%f\n",t0,t1,steps,log_offset,eps_dis);

   for (i=0;i<NUM_VARS;i++)
   {
      if (!(r[i]=find_reg(names[i],1,1)))
         { printf("sf_null_int: cannot find register <%s>\n",names[i]); goto clean_up; }
   }

   if (t0>=0)
   {
      eps=1;
      printf("sf_null_int: t0>=0 ... integrating forwards in time, eps_dis=%f\n",eps_dis);
   }
   else
   {
      eps=-1;
      printf("sf_null_int: t0<0 ... integrating backwards in time, eps_dis=%f\n",eps_dis);
   }
     
   printf("Null integration (first order accurate)");
   printf("test function is d[alpha]/dt along z=0\n");

   while (steps--)
   {
      for (i=0;i<NUM_VARS;i++)
         ts[i]=r[i]->ts;
      t=ts[0]->time;
   
      cpt=0;
      while(t<fabs(t0))
      {
         if (!N_eval_f_1d(ts[ALPHA_Z0],0,0,&fn[ALPHA_Z0],0)) 
            { printf("sf_null_int: error ... time %f does not contain the origin\n",t); goto clean_up; }
         for (i=0;i<NUM_VARS;i++) 
         {
            ts[i]=ts[i]->next;
            if (!ts[i]) { printf("sf_null_int: data ends before t0=%f\n",fabs(t0)); goto clean_up; }
         }
         dt=ts[0]->time-t;
         t=ts[0]->time;
         cpt=cpt+fn[ALPHA_Z0]*dt;
      }
      cpt=eps*(cpt+fn[ALPHA_Z0]*eps*dt/2);

      if (eps==-1){ for (i=0;i<NUM_VARS;i++) { reverse_times(r[i]); reversed=1; } }

      qts=ts[0]; num=0; while(qts=qts->next) num++; 
      if (!(mem=(double *)malloc(num*8*sizeof(double))))
         { printf("sf_null_int: could not allocate %i doubles\n",num*8); }
      rho=mem; z=&mem[num]; phi_z0=&mem[2*num];
      phi_rho0=&mem[3*num]; lambda_rho0=&mem[4*num];
      lambda_z0=&mem[5*num]; ev_test=&mem[6*num];
      diff=&mem[7*num];

      n=0; rho_i=0; z_i=0; 
      lambda_z0_i=0; lambda_rho0_i=0;
      while(ts[0]->next)
      {
         if (n==0)
         {
            printf("t=%f, cpt=%f\n\n",t,cpt);
            IFL printf("\n  t\t\t lambda(z=0) \t rho \t\t phi(z=0) \t lambda(rho=0) \t z \t\t phi(rho=0)\n");
            IFL printf("============================================================================================================\n");
         }
         for (i=0;i<NUM_VARS;i++) 
         {
            if (eps_dis==0) can_dis=0; else can_dis=1;
            if (i<ALPHA_Z0) x=z_i; else x=rho_i;
            prev_t=ts[i]->time;
            // search for closest adjacent times where function value is defined.
            while (ts[i] && !N_eval_f_1d(ts[i],x,&dx,&fn[i],&fn_x[i])) ts[i]=ts[i]->next;
            if (!ts[i]) goto done;
            if (ts[i]->time!=prev_t) printf("warning ... skipped %f in t, t=%f, i=%i\n",ts[i]->time-prev_t,ts[i]->time,i);

            // some form of smoothing is needed for time derivatives of
            // constrained functions, becomes of the current adaptive solution method
            if (eps_dis>0)
            {
               qts=ts[i]->prev;
               while(qts && !N_eval_f_1d(qts,x,0,&fnm1[i],0)) qts=qts->prev;
               if (!qts) can_dis=0; else { y=qts->time; dtm1=ts[i]->time-y; qts=qts->prev; }
               while(qts && !N_eval_f_1d(qts,x,0,&fnm2[i],0)) qts=qts->prev;
               if (!qts) can_dis=0; else dtm2=y-qts->time;
            }
   
            prev_t=ts[i]->time;
            ts[i]=ts[i]->next;
            while (ts[i] && !N_eval_f_1d(ts[i],x,0,&fnp1[i],&fnp1_x[i])) ts[i]=ts[i]->next;
            if (!ts[i]) goto done;
   
            if (eps_dis>0 && can_dis)
            {
               qts=ts[i]->next;
               while(qts && !N_eval_f_1d(qts,x,0,&fnp2[i],0)) qts=qts->next;
               if (!qts) can_dis=0; else { y=qts->time; dtp1=y-ts[i]->time; qts=qts->next; }
               while(qts && !N_eval_f_1d(qts,x,0,&fnp3[i],0)) qts=qts->next;
               if (!qts) can_dis=0; else dtp2=qts->time-y;
            }

            dt=ts[i]->time-prev_t;
            t=(ts[i]->time+prev_t)/2;
            
            f_t[i]=(fnp1[i]-fn[i])/dt;
            if (can_dis)
            {
               f_tp1=(fnp2[i]-fnp1[i])/dtp1;
               f_tp2=(fnp3[i]-fnp2[i])/dtp2;
               f_tm1=(fn[i]-fnm1[i])/dtm1;
               f_tm2=(fnm1[i]-fnm2[i])/dtm2;
            }
            if (can_dis && eps_dis<=1)
               f_t[i]=f_t[i]-(eps_dis/16)*(f_tp2 - 4*f_tp1 + 6*f_t[i] - 4*f_tm1 + f_tm2);
            else if (can_dis && eps_dis==2)
               f_t[i]=(f_t[i]+f_tp1+f_tm1)/3;
            else if (can_dis && eps_dis==3)
               f_t[i]=(f_t[i]+f_tp1+f_tm1+f_tp2+f_tm2)/5;
            else if (can_dis && eps_dis==4)
               f_t[i]=(3*f_t[i]+2*(f_tp1+f_tm1)+f_tp2+f_tm2)/9;

            f[i]=(fnp1[i]+fn[i])/2;
            f_x[i]=(fnp1_x[i]+fn_x[i])/2;
            if (i==ALPHA_Z0) ev_test[n]=f_t[ALPHA_Z0];
         }
         phi_z0[n]=f[PHI_Z0];
         phi_rho0[n]=f[PHI_RHO0];
         rho[n]=rho_i;
         z[n]=z_i;
         lambda_z0[n]=lambda_z0_i;
         lambda_rho0[n]=lambda_rho0_i;
   
         IFL printf("%10.6f\t%10.6f\t%8.4f\t%10.6f\t%10.6f\t%8.4f\t%10.6f\n",
                     t,lambda_z0[n],rho_i,phi_z0[n],lambda_rho0[n],z_i,phi_rho0[n]);
   
         // _dot refers to d/dlambda
         // initial condition for rho_dot, z_dot : affine parameter lambda is in
         // sync with central proper time flow (i.e. dlambda=dcpt @ rho=z=0)
         if (n==0)
         {
            rho_dot=z_dot=eps/f[PSI_Z0]/f[PSI_Z0];
         }
   
         z_dotdot=-z_dot*z_dot/3*(6*pow(f[ALPHA_RHO0],3)*f_x[PSI_RHO0]+
                                  f[ALPHA_RHO0]*f[ALPHA_RHO0]*(3*f[PSI_RHO0]*f_x[ALPHA_RHO0]+
                                                               2*eps*pow(f[PSI_RHO0],3)*f_x[BZ_RHO0])+
                                  f[ALPHA_RHO0]*pow(f[PSI_RHO0],5)*(-f[BZ_RHO0]*f_x[BZ_RHO0]+
                                                                    3*f_t[BZ_RHO0])-
                                  6*eps*f[ALPHA_RHO0]*pow(f[PSI_RHO0],3)*f[BZ_RHO0]*f_x[ALPHA_RHO0]+
                                  3*pow(f[PSI_RHO0],5)*f[BZ_RHO0]*(f_x[ALPHA_RHO0]*f[BZ_RHO0]-
                                                                   f_t[ALPHA_RHO0])+
                                  2*pow(f[PSI_RHO0],3)*f[ALPHA_RHO0]*f[BRHO_RHO_RHO0]*
                                                                  (2*f[ALPHA_RHO0]*eps-
                                                                   f[PSI_RHO0]*f[PSI_RHO0]*f[BZ_RHO0]))
                   /f[ALPHA_RHO0]/f[PSI_RHO0]/pow(f[ALPHA_RHO0]-eps*f[PSI_RHO0]*f[PSI_RHO0]*f[BZ_RHO0],2);
   
         rho_dotdot=-rho_dot*rho_dot/3*(2*eps*pow(f[ALPHA_Z0]*f[PSI_Z0],3)*rho_i*f[OMEGABAR_Z0]+
                                        6*pow(f[ALPHA_Z0],3)*f_x[PSI_Z0]+
                                        4*eps*f[ALPHA_Z0]*f[ALPHA_Z0]*pow(f[PSI_Z0],3)*f_x[BRHO_Z0]+
                                        3*f[ALPHA_Z0]*f[ALPHA_Z0]*f_x[ALPHA_Z0]*f[PSI_Z0]+
                                        f[ALPHA_Z0]*pow(f[PSI_Z0],5)*(-f[ALPHA_Z0]*rho_i*f[OMEGABAR_Z0]*f[BRHO_Z0]
                                                                      -2*f[BRHO_Z0]*f_x[BRHO_Z0]
                                                                      +3*f_t[BRHO_Z0])-
                                        6*eps*f[ALPHA_Z0]*pow(f[PSI_Z0],3)*f[BRHO_Z0]*f_x[ALPHA_Z0]+
                                        3*pow(f[PSI_Z0],5)*f[BRHO_Z0]*(f[BRHO_Z0]*f_x[ALPHA_Z0]-
                                                                       f_t[ALPHA_Z0])+
                                        pow(f[PSI_Z0],3)*f[ALPHA_Z0]*f[BZ_Z_Z0]*
                                                                    (2*f[ALPHA_Z0]*eps-
                                                                     f[PSI_Z0]*f[PSI_Z0]*f[BRHO_Z0]))   
                   /f[ALPHA_Z0]/f[PSI_Z0]/pow(f[ALPHA_Z0]-eps*f[PSI_Z0]*f[PSI_Z0]*f[BRHO_Z0],2);
   
         dz_dt=eps*f[ALPHA_RHO0]/f[PSI_RHO0]/f[PSI_RHO0]-f[BZ_RHO0];
         drho_dt=eps*f[ALPHA_Z0]/f[PSI_Z0]/f[PSI_Z0]-f[BRHO_Z0];
         z_i=z_i+dz_dt*dt;
         rho_i=rho_i+drho_dt*dt;
   
         dlambda_dt_z0=drho_dt/rho_dot;
         dlambda_dt_rho0=dz_dt/z_dot;
         lambda_z0_i=lambda_z0_i+dlambda_dt_z0*dt;
         lambda_rho0_i=lambda_rho0_i+dlambda_dt_rho0*dt;
   
         rho_dot=rho_dot+rho_dotdot*dlambda_dt_z0*dt;
         z_dot=z_dot+z_dotdot*dlambda_dt_rho0*dt;
         n++;
      }
done:
      IFL printf("\n fractional difference [phi(z=0)-phi(rho=0)][|phi(z=0)|+|phi(rho=0)|]/2 as a function of lambda:\n\n");
      IFL printf("lambda\t\t  difference\n============================\n");
      for (n2=0,i=1; i<(n-1) && n2<(n-1); n2++)
      {
         lambda=lambda_z0[n2];
         while(fabs(lambda_rho0[i])<=fabs(lambda) && i<n) i++;
         f1=fabs(lambda_rho0[i])-fabs(lambda);
         f2=fabs(lambda)-fabs(lambda_rho0[i-1]);
         phi_rho0_i=(f2*phi_rho0[i]+f1*phi_rho0[i-1])/(f1+f2);
         diff[n2]=(phi_z0[n2]-phi_rho0_i)/(fabs(phi_z0[n2])+fabs(phi_rho0_i))/2;
         IFL printf("%12.8f\t%12.8f\n",lambda,diff[n2]);
      }

      if (n>1)
      {
         add_grid("phi(z=0,lambda)",cpt,1,COORD_CURVILINEAR,&n,lambda_z0,0,phi_z0);
         add_grid("phi(rho=0,lambda)",cpt,1,COORD_CURVILINEAR,&n,lambda_rho0,0,phi_rho0);
         add_grid("rho(lambda)",cpt,1,COORD_CURVILINEAR,&n,lambda_z0,0,rho);
         add_grid("z(lambda)",cpt,1,COORD_CURVILINEAR,&n,lambda_rho0,0,z);
         add_grid("ev_test(lambda)",cpt,1,COORD_CURVILINEAR,&n,lambda_z0,0,ev_test);
      }
      if (n2>1)
      {
         add_grid("diff(lambda)",cpt,1,COORD_CURVILINEAR,&n2,lambda_z0,0,diff);
      }
      for (i=0; i<n; i++)
      {
         lambda_z0[i]=log(fabs(lambda_z0[i])+log_offset)-log(log_offset);
         lambda_rho0[i]=log(fabs(lambda_rho0[i])+log_offset)-log(log_offset);
      }

      if (n>1)
      {
         add_grid("phi(z=0,log(lambda))",cpt,1,COORD_CURVILINEAR,&n,lambda_z0,0,phi_z0);
         add_grid("phi(rho=0,log(lambda))",cpt,1,COORD_CURVILINEAR,&n,lambda_rho0,0,phi_rho0);
         add_grid("rho(log(lambda))",cpt,1,COORD_CURVILINEAR,&n,lambda_z0,0,rho);
         add_grid("z(log(lambda))",cpt,1,COORD_CURVILINEAR,&n,lambda_rho0,0,z);
         add_grid("ev_test(log(lambda))",cpt,1,COORD_CURVILINEAR,&n,lambda_z0,0,ev_test);
      }
      if (n2>1)
      {
         add_grid("diff(log(lambda))",cpt,1,COORD_CURVILINEAR,&n2,lambda_z0,0,diff);
      }

      if (reversed) for (i=0;i<NUM_VARS;i++) reverse_times(r[i]);
      reversed=0;
      free(mem); mem=0;
      t0=t0+gdt;
   }

clean_up:
   for (i=0;i<NUM_VARS;i++) if (r[i]) {if (reversed) reverse_times(r[i]); FREE_REG(r[i]); }
   if (mem) free(mem);
}  

//-----------------------------------------------------------------------------
// reads in 'func', and produces, via linear interpolation
// in time, a set of output times in ln space
//-----------------------------------------------------------------------------
void sf_ln_vt(char *args)
{
   int n,nl,l,k,i;
   double ts,ln_te,ln_ts,dt,t,ln_t,step,*p,*q,tn,tnp1,frac;
   reg *r;
   char func[MAX_NAME_LENGTH];
   char new_reg[MAX_NAME_LENGTH]="vt_",*cp;
   grid *g,*gn,*gnp1;
   level *l_tn,*l_tnp1,*ls;
   time_str *cts;
   s_iter it;

   if (!args) { printf("sf_ln_vt: arguments required\n"); return; }

   sscanf(args,"%s",func);
   cp=func; while(*cp && *cp!=',') cp++;
   *cp=0;
   if (!(r=find_reg(func,1,1)))
   {
      printf("sf_ln_vt: cannot find register %s\n",func);
      return;
   }

   sscanf(&cp[1],"%i,%lf,%lf,%i,%lf",&n,&ts,&ln_te,&nl,&dt);

   strncat(new_reg,func,MAX_NAME_LENGTH-10);

   printf("sf_ln_vt: function=%s, new register=%s (dropping level 1)\nt\n================\n",func,new_reg);

   *cp=',';

   ln_ts=log(ts);
   ln_t=ln_ts;
   step=(ln_te-ln_ts)/n;
   for (i=0;i<(n+nl);i++)
   {
      if (i<n) { t=ts-exp(ln_t); ln_t+=step; } else t+=dt;
      printf("%lf\n",t);
      for (l=2; l<=it.num_l; l++)
      {
         init_s_iter(&it,r," ");
         l_tn=l_tnp1=0;
         cts=it.ts;
         while(cts && (cts->time <= t))
         {
            ls=it.l;
            while (ls && ls->ln!=l) ls=ls->next; 
            if (ls) { l_tn=ls; tn=cts->time; }
            cts=next_ts(&it);
         }
         while(cts && (!l_tnp1))
         {
            ls=it.l;
            while (ls && ls->ln!=l) ls=ls->next; 
            if (ls) { l_tnp1=ls; tnp1=cts->time; }
            cts=next_ts(&it);
         }
         if (l_tn && l_tnp1)
         {
            gn=l_tn->grids;
            gnp1=l_tnp1->grids;
            frac=(t-tn)/(tnp1-tn);
            while(gn && gnp1)
            {
               if (!gridcmp(gn,gnp1))
               {
                  if (g=galloc(gn->dim,gn->coord_type,gn->shape,t,gn->coords,gn->ccoords,gn->data))
                  {
                      p=g->data;
                      q=gnp1->data;
                      for (k=0; k<sizeof_data(g); k++) *p++=frac*(*q++)+(1-frac)*(*p);
                      add_grid_str(new_reg,g);
                   }
               }
               gn=gn->next;
               gnp1=gnp1->next;
            }
         }
      }
   }

   FREE_REG(r);
}

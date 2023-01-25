//=============================================================================
// geod_int.c
// 
// graxi_ad specific to manipulate files produces by geodesic integrator
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

#define GD_RHO 0
#define GD_Z 1
#define GD_C 2
#define GD_R 3
#define GD_R2 4
#define GD_LAMBDA 5
#define GD_PHI 6
#define GD_PI_RHO 7
#define GD_PI_Z 8
#define GD_T0 9
#define GD_THETA0 10
#define GD_L 11
#define GD_N 12
#define GD_ires 13

#define GD_NUM_VARS 14

char *gd_names[GD_NUM_VARS]=
{ "gd_rho", "gd_z", "gd_C", "gd_R", "gd_R2", 
  "gd_lambda", "gd_phi", "gd_pi_rho", "gd_pi_z", "gd_t0", "gd_theta0", "gd_L",
  "gd_n", "gd_ires" };
 
//------------------------------------------------------------------------------
// to show geodesics as a function of rho,z
//------------------------------------------------------------------------------
void sf_gd_concat(char *args)
{
   int which_L,phase,i;
   reg *rho=0,*z=0,*L=0,*z_val=0,*t0=0;
   char z_reg[MAX_NAME_LENGTH],out_name[MAX_NAME_LENGTH];
   int z_valid=1,n,m;
   s_iter rho_it,z_it,L_it,z_val_it,t0_it;
   grid *rho_g,*z_g,*L_g,*z_val_g,*ng,*t0_g;
   double *p,bbox[4],t0_c=-1,*q;
   int shape[2];

   which_L=1; // for both L=0 and L=-1
   phase=0; // if one plot (pi_rho,pi_z)
   z_reg[0]=0; // the register that will become the value of the new reg.

   if (args && (strlen(args)>0)) sscanf(args,"%i,%i,%lf,%s",&which_L,&phase,&t0_c,z_reg);

   printf("sf_gd_concat: L=%i, phase=%i, t0=%lf, z-register=%s\n",which_L,phase,t0_c,z_reg);

   sprintf(out_name,"gd_concat(%s,%lf)",z_reg,t0_c);

   if (strlen(z_reg)==0) z_valid=0;

   if (phase) 
   {
      if (!(rho=find_reg(gd_names[GD_PI_RHO],1,1)))
         { printf("sf_gd_concat: cannot find register <%s>\n",gd_names[GD_PI_RHO]); goto clean_up; }
      if (!(z=find_reg(gd_names[GD_PI_Z],1,1)))
         { printf("sf_gd_concat: cannot find register <%s>\n",gd_names[GD_PI_Z]); goto clean_up; }

      if (z_valid && !(strcmp(z_reg,gd_names[GD_PI_Z]))) z_valid=0;
      if (z_valid && !(strcmp(z_reg,gd_names[GD_PI_RHO]))) z_valid=0;
   }
   else
   {
      if (!(rho=find_reg(gd_names[GD_RHO],1,1)))
         { printf("sf_gd_concat: cannot find register <%s>\n",gd_names[GD_RHO]); goto clean_up; }
      if (!(z=find_reg(gd_names[GD_Z],1,1)))
         { printf("sf_gd_concat: cannot find register <%s>\n",gd_names[GD_Z]); goto clean_up; }

      if (z_valid && !(strcmp(z_reg,gd_names[GD_Z]))) z_valid=0;
      if (z_valid && !(strcmp(z_reg,gd_names[GD_RHO]))) z_valid=0;
   }
   if (which_L==0 || which_L==-1)
   {
      if (!(L=find_reg(gd_names[GD_L],1,1)))
         { printf("sf_gd_concat: cannot find register <%s>\n",gd_names[GD_L]); goto clean_up; }

      if (z_valid && !(strcmp(z_reg,gd_names[GD_L]))) z_valid=0;
   }

   if (t0_c>-1)
   {
      if (!(t0=find_reg(gd_names[GD_T0],1,1)))
         { printf("sf_gd_concat: cannot find register <%s>\n",gd_names[GD_T0]); goto clean_up; }

      if (z_valid && !(strcmp(z_reg,gd_names[GD_T0]))) z_valid=0;
   }

   if (z_valid)
   {
      if (!(z_val=find_reg(z_reg,1,1)))
         { printf("sf_gd_concat: cannot find register <%s>\n",z_reg); }
      z_valid=0;
   }

   rho_g=init_s_iter(&rho_it,rho,0);
   z_g=init_s_iter(&z_it,z,0);
   if (L) L_g=init_s_iter(&L_it,L,0); else L_g=0;
   if (z_val) z_val_g=init_s_iter(&z_val_it,z_val,0); else z_val_g=0;
   if (t0) t0_g=init_s_iter(&t0_it,t0,0); else t0_g=0;

   // the DV cannot handle particles at this stage, so
   // adding as a semi-uniform N/2 X 2 2d grid
   while(rho_g && z_g)
   {
      n=sizeof_data(rho_g);
      if (L_g || t0_g)
      {
         m=n; n=0;
         if (L_g) p=L_g->data; else p=0;
         if (t0_g) q=t0_g->data; else q=0;
         while (m--) 
         {
            // update the similar line below if the conditions change
            if ( (!p || (fabs(*p-which_L) < 0.1)) && 
                 (!q || (fabs(*q-t0_c) < 1.0e-6)) ) n++; 
            if (p) p++;
            if (q) q++;
         }
      }

      shape[0]=n/2; if ((n%2)==1) shape[0]++;
      shape[1]=2;
      bbox[0]=bbox[2]=0;
      bbox[1]=bbox[3]=1;

      if (!(ng=galloc(2,COORD_SEMI_UNIFORM,shape,rho_g->time,bbox,0,0)))
         { printf("sf_gd_concat: galloc failed\n"); goto clean_up; }

      m=0; 
      i=0;
      while(i<sizeof_data(rho_g))
      {
         if ( (!L_g || (fabs(L_g->data[i]-which_L) < 0.1)) &&
              (!t0_g || (fabs(t0_g->data[i]-t0_c) < 1.0e-6)) )
         {
            if (z_val_g) ng->data[m]=z_val_g->data[i]; else ng->data[m]=0;
            ng->ccoords[m]=rho_g->data[i];
            ng->ccoords[m+shape[0]*shape[1]]=z_g->data[i];
            m++;
         }
         i++;
      }

      if ((n%2)==1)
      {
         if (z_val_g) ng->data[m]=ng->data[m-1]; else ng->data[m]=0;
         ng->ccoords[m]=ng->ccoords[m-1];
         ng->ccoords[m+shape[0]*shape[1]]=ng->ccoords[m+shape[0]*shape[1]-1];
      }

      add_grid_str(out_name,ng);
      rho_g=next_g(&rho_it);
      z_g=next_g(&z_it);
      if (L_g) L_g=next_g(&L_it);
      if (z_val_g) z_val_g=next_g(&z_val_it);
      if (t0_g) t0_g=next_g(&t0_it);
   }

clean_up:
   if (rho) FREE_REG(rho);
   if (L) FREE_REG(L);
   if (z) FREE_REG(z);
   if (z_val) FREE_REG(z_val);
   if (t0) FREE_REG(t0);
}  

int get_3index_gd(grid *g1, grid *g2, grid *g3, double gv_1, double gv_2, double gv_3)
{
   int i;

   for (i=0; i<sizeof_data(g1); i++)
   {
      if (g1->data[i]==gv_1 && g2->data[i]==gv_2 && g3->data[i]==gv_3) return i;
   }

   return -1;
}

//------------------------------------------------------------------------------
// produces a 2D map of each null cone, in (lambda,theta)  vs. t0
// second argument is KO smoothing eps to apply.
//------------------------------------------------------------------------------
#define GD_LMAX 7
void sf_gd_concat_b(char *args)
{
   int i,j,nt,nth,k,found,ind,sl,out_of_range;
   reg *L=0,*z_val=0,*t0=0,*theta0=0,*lambda=0,*gd_n=0;
   char z_reg[MAX_NAME_LENGTH];
   char out_name_nu[MAX_NAME_LENGTH];
   char out_name_diff_nu[MAX_NAME_LENGTH];
   char out_name_u[MAX_NAME_LENGTH];
   char out_name_diff_u[MAX_NAME_LENGTH];
   char out_name_u_l[GD_LMAX][MAX_NAME_LENGTH];
   int z_valid=1,n,m,gd_n0;
   s_iter L_it,z_val_it,t0_it,theta0_it,lambda_it,n_it;
   grid *L_g,*z_val_g,*ng=0,*t0_g,*theta0_g,*lambda_g=0,*ng2=0,*ng_nu=0,*ng_nu_m_th0=0,*n_g;
   grid *ng_u=0,*ng_u_m_th0=0,*ng_u_l[GD_LMAX];
   double *p,*q,*l,bbox[4],t,lmax,*y,lambda0,*thetac,*tc,dt,dt2,t0_c=-1,prev_t,*xc,*yc;
   double lambda_n,lambda_np1,z_n,z_np1,frac,prev_t0,d_lambda,eps=0.0,dtheta,avg,tau_s,r;
   double gd_n_pt,prev_gd_n_pt,spec,Dx,lambda_max=0;
   int scale_type;
   double scale,lambda_of_min,lambda_of_max,c_min,c_max,lambda_s,theta,bound;
   int shape[2],ltrace=1,skip,num,error_state;
   int shape_u[2],i0,j0; 
   double coords_u[4],x0,y0,dx,dy,frac_r,frac_th,x;
   double *plx;

   z_reg[0]=0; // the register that will become the value of the new reg.
   for (sl=0;sl<GD_LMAX;sl++) ng_u_l[sl]=0;

   if (args && (strlen(args)>0)) sscanf(args,"%lf,%lf,%i,%i,%lf,%lf,%s",&t0_c,&eps,&scale_type,&gd_n0,&tau_s,&lambda_max,z_reg);

   printf("sf_gd_concat_b: t0=%lf, eps=%lf, gd_n0=%i, tau*=%lf, lambda_max=%lf, z-register=%s\n",t0_c,eps,gd_n0,tau_s,lambda_max,z_reg);
   skip=t0_c;
   if (skip<0) printf("t0_c<0 ... producing a map every %i steps\n",skip);
   if (scale_type==1 && gd_n0>0)
      printf("scaling using tau*\n");
   else if (scale_type==2) 
      printf("scaling using min/max feature\n");
   else
      printf("no scaling\n");

   sprintf(out_name_nu,"gd_concat_b(%s,tau*=%lf,n=%i,st=%i)",z_reg,tau_s,gd_n0,scale_type);
   sprintf(out_name_diff_nu,"gd_concat_b(%s-<theta>,tau*=%lf,n=%i,st=%i)",z_reg,tau_s,gd_n0,scale_type);
   sprintf(out_name_u,"gd_concat_b_uni(%s,tau*=%lf,n=%i,st=%i)",z_reg,tau_s,gd_n0,scale_type);
   sprintf(out_name_diff_u,"gd_concat_b_uni(%s-<theta>,tau*=%lf,n=%i,st=%i)",z_reg,tau_s,gd_n0,scale_type);
   for (n=0;n<GD_LMAX;n++) sprintf(&out_name_u_l[n][0],"gd_concat_b_uni_L%i(%s,tau*=%lf,n=%i,st=%i)",n,z_reg,tau_s,gd_n0,scale_type);

   if (strlen(z_reg)==0) z_valid=0;

   if (!(L=find_reg(gd_names[GD_L],1,1)))
         { printf("sf_gd_concat_b: cannot find register <%s>\n",gd_names[GD_L]); goto clean_up; }
   if (z_valid && !(strcmp(z_reg,gd_names[GD_L]))) z_valid=0;

   if (!(t0=find_reg(gd_names[GD_T0],1,1)))
         { printf("sf_gd_concat_b: cannot find register <%s>\n",gd_names[GD_T0]); goto clean_up; }
   if (z_valid && !(strcmp(z_reg,gd_names[GD_T0]))) z_valid=0;

   if (!(theta0=find_reg(gd_names[GD_THETA0],1,1)))
         { printf("sf_gd_concat_b: cannot find register <%s>\n",gd_names[GD_THETA0]); goto clean_up; }
   if (z_valid && !(strcmp(z_reg,gd_names[GD_THETA0]))) z_valid=0;

   if (!(lambda=find_reg(gd_names[GD_LAMBDA],1,1)))
         { printf("sf_gd_concat_b: cannot find register <%s>\n",gd_names[GD_LAMBDA]); goto clean_up; }
   if (z_valid && !(strcmp(z_reg,gd_names[GD_LAMBDA]))) z_valid=0;

   if (gd_n0>0)
   {
      if (!(gd_n=find_reg(gd_names[GD_N],1,1)))
         { printf("sf_gd_concat_b: cannot find register <%s>\n",gd_names[GD_N]); goto clean_up; }
      printf("transforming to the proper time of geodesic gd_n0\n");
      if (z_valid && !(strcmp(z_reg,gd_names[GD_N]))) z_valid=0;
   }

   if (z_valid)
   {
      if (!(z_val=find_reg(z_reg,1,1)))
         { printf("sf_gd_concat_b: cannot find register <%s>\n",z_reg); }
      z_valid=0;
   }

   t0_g=init_s_iter(&t0_it,t0,0); 
   L_g=init_s_iter(&L_it,L,0); 
   theta0_g=init_s_iter(&theta0_it,theta0,0); 
   lambda_g=init_s_iter(&lambda_it,lambda,0); 
   if (gd_n0>0) n_g=init_s_iter(&n_it,gd_n,0); else n_g=0;
   if (z_val) z_val_g=init_s_iter(&z_val_it,z_val,0); else z_val_g=0;

   prev_t=t=-0.001;
   dt=dt2=0;
   
   num=0;
   error_state=0;
   gd_n_pt=0;
   while(t0_g)
   {
      out_of_range=0;
      dt=t0_g->time-t;
      t=t0_g->time;
      if (gd_n0>0)
      {
         p=n_g->data;
         l=lambda_g->data;
         prev_gd_n_pt=gd_n_pt;
         gd_n_pt=t;
         for (i=0; i<sizeof_data(n_g); l++, p++, i++)
            if (*p==gd_n0) gd_n_pt=*l;
         if (gd_n_pt==t && scale_type==1) out_of_range=1;
         else if (tau_s>0) 
         {
            if (gd_n_pt>tau_s) out_of_range=1;
            else gd_n_pt=-log(tau_s-gd_n_pt);
         }
      }
      else gd_n_pt=t;

      // calculate size of surface, based on number of theta=0 points, and t=t0
      // points
      save_s_iter(&t0_it);
      save_s_iter(&theta0_it);
      save_s_iter(&L_it);
      save_s_iter(&lambda_it);
      if (z_val) save_s_iter(&z_val_it);

      nt=0; nth=0;
      p=t0_g->data; l=L_g->data;
      for (i=0; i<sizeof_data(t0_g); i++, l++, p++)
         if (fabs((*p-t)/dt)<0.1 && fabs(*l)<0.1) nth++;
      if (nth>1) {dt2=t-prev_t; prev_t=t;}
      if (ltrace && nth>0) printf("t=%lf, pt=%lf, nth=%i, dt2=%lf\n",t,gd_n_pt,nth,dt2);

      if (nth>1 && ((t0_c<0 && (!(num%(-skip)))) || fabs(t0_c - t)<dt2) && !(out_of_range))
      {
         found=nth;
         while(t0_g && (found==nth))
         {
            p=t0_g->data; l=L_g->data; y=lambda_g->data;
            found=0;
            for (i=0; i<sizeof_data(t0_g); i++, y++, l++, p++)
            {
               if (fabs((*p-t)/dt)<0.1 && fabs(*l)<0.1) { found++; lmax=*y; }
            }
            if (found==nth) nt++;
            t0_g=next_g(&t0_it);
            L_g=next_g(&L_it);
            lambda_g=next_g(&lambda_it);
         }
         L_g=restore_s_iter(&L_it);
         t0_g=restore_s_iter(&t0_it);
         lambda_g=restore_s_iter(&lambda_it);

         if (ltrace) printf("nt=%i\n",nt);

         shape[0]=nth; 
         shape[1]=nt;
         bbox[0]=bbox[2]=0;
         bbox[1]=M_PI;
         bbox[3]=lmax;

         if (!(ng=galloc(2,COORD_SEMI_UNIFORM,shape,gd_n_pt,bbox,0,0)))
            { printf("sf_gd_concat_b: galloc failed\n"); goto clean_up; }

         // first, fill in coordinates
         thetac=ng->ccoords;
         tc=thetac+sizeof_data(ng);
         // we assume that theta0 increases monotonically in the gd_... registers
         p=theta0_g->data;
         q=t0_g->data;
         l=L_g->data;
         i=0;
         for (k=0;k<sizeof_data(t0_g);k++,q++,p++,l++)
         {
            if (fabs((*q-t)/dt)<0.1 && fabs(*l)<0.1 && i<nth)
            {
               for (j=0; j<nt; j++) thetac[i+j*nth]=*p;
               i++;
            }
         }

         // now lambda

         j=0;
         while(t0_g && j<nt)
         {
            ind=get_3index_gd(L_g,theta0_g,t0_g,0,thetac[j*nth],t);
            if (ind<0)
            { printf("sf_gd_concat_b: index error\n"); }
            else
            {
               for (i=0; i<nth; i++) tc[i+j*nth]=lambda_g->data[ind];
            }
            j=j+1;
            t0_g=next_g(&t0_it);
            L_g=next_g(&L_it);
            lambda_g=next_g(&lambda_it);
            theta0_g=next_g(&theta0_it);
         }
         theta0_g=restore_s_iter(&theta0_it);
         L_g=restore_s_iter(&L_it);
         t0_g=restore_s_iter(&t0_it);
         lambda_g=restore_s_iter(&lambda_it);

         // now, point by point, fill in the grid, via linear interpolation in 
         // lambda
         for (i=0; i<nth && !error_state; i++)
         {
            lambda0=lambda_np1=lambda_n=tc[i];
            ind=get_3index_gd(L_g,theta0_g,t0_g,0,thetac[i],t);
            if (ind<0) { if (!error_state) printf("error ... ind<0 . i=%i   t=%lf\n",i,t); error_state=1; ind=0;}
            if (z_val) z_n=z_np1=z_val_g->data[ind]; 
            else z_n=z_np1=lambda_g->data[ind];
            for (j=0; j<nt && !error_state ; j++)
            {

               lambda0=tc[i+j*nth];
               // IFL printf("\t i,j=%i,%i,  lambda0=%lf\n",i,j,lambda0);
               while(t0_g && ind>=0 && lambda_g->data[ind]<lambda0)
               {
                  z_n=z_np1;
                  lambda_n=lambda_np1;
                  t0_g=next_g(&t0_it);
                  L_g=next_g(&L_it);
                  lambda_g=next_g(&lambda_it);
                  theta0_g=next_g(&theta0_it);
                  if (z_val) z_val_g=next_g(&z_val_it);
                  if (t0_g)
                  {
                     ind=get_3index_gd(L_g,theta0_g,t0_g,0,thetac[i+j*nth],t);
                     if (ind>=0)
                     {
                        if (z_val) z_np1=z_val_g->data[ind]; 
                        else z_np1=lambda_g->data[ind];
                        lambda_np1=lambda_g->data[ind];
                     } 
                  }
               }

               if (t0_g && ind>=0)
               {
                  d_lambda=lambda_np1-lambda_n;
                  if (d_lambda>0) 
                  {
                     frac=(lambda0-lambda_n)/d_lambda;
                     ng->data[i+j*nth]=(1-frac)*z_n+frac*(z_np1);
                  }
                  else ng->data[i+j*nth]=z_n;
               }
               else
               {
                  ng->data[i+j*nth]=z_n;
               }
            }
            theta0_g=restore_s_iter(&theta0_it);
            L_g=restore_s_iter(&L_it);
            t0_g=restore_s_iter(&t0_it);
            lambda_g=restore_s_iter(&lambda_it);
            if (z_val) z_val_g=restore_s_iter(&z_val_it);
         }

         if (!(ng2=galloc(2,COORD_SEMI_UNIFORM,shape,gd_n_pt,bbox,ng->ccoords,ng->data)))
            { printf("sf_gd_concat_b: galloc failed\n"); goto clean_up; }

         // smooth --- 3 passes

         if (eps>0)
         {
            for (k=0; k<3; k++)
            {
               for (i=0; i<sizeof_data(ng); i++) ng2->data[i]=ng->data[i];
               for (i=0; i<nth; i++)
               {
                  for (j=0; j<nt; j++)
                  {
                     if (i>1 && i<(nth-2)) ng->data[i+j*nth]-=eps/16*(ng2->data[i-2+j*nth]+ng2->data[i+2+j*nth]
                                           -4*(ng2->data[i-1+j*nth]+ng2->data[i+1+j*nth])+6*ng2->data[i+j*nth]);
                     if (j>1 && j<(nt-2)) ng->data[i+j*nth]-=eps/16*(ng2->data[i+(j-2)*nth]+ng2->data[i+(j+2)*nth]
                                           -4*(ng2->data[i+(j-1)*nth]+ng2->data[i+(j+1)*nth])+6*ng2->data[i+j*nth]);
                  }
               }
            }
         }

         // ng_nu has (x=lambda*sin(theta),y=lambda*cos(theta)) coordinates
         if (!(ng_nu=galloc(2,COORD_SEMI_UNIFORM,shape,gd_n_pt,bbox,ng->ccoords,ng->data)))
            { printf("sf_gd_concat_b: galloc failed\n"); goto clean_up; }
         xc=ng_nu->ccoords;
         yc=xc+sizeof_data(ng_nu);
         // seek first local extremes in gd_z, along theta=0
         c_min=1e10; c_max=-1e10;
         i=0;
         for (j=0; j<nt; j++) 
         {
            if (ng->data[i+j*nth]<c_min) 
               { c_min=ng->data[i+j*nth]; lambda_of_min=tc[i+j*nth]; } 
            if (ng->data[i+j*nth]>c_max) 
               { c_max=ng->data[i+j*nth]; lambda_of_max=tc[i+j*nth]; } 
         }
         lambda_s=min(lambda_of_min,lambda_of_max);
         if (lambda_s==0) lambda_s=max(lambda_of_min,lambda_of_max);
         if (scale_type==2)
         {
            scale=1/lambda_s;
            bound=6;
         }
         else if (scale_type==1 && gd_n0>0)
         {
            scale=1/exp(-gd_n_pt); 
            // bound=6*lambda_s*scale;
            bound=30;
         }
         else
         {
            scale=1;
            bound=lmax/2;
         }
         if (lambda_max>1e-10) bound=lambda_max;
         printf("scale=%lf, lambda('z') of min/max=%lf(%lf) %lf(%lf), lambda_max=%lf\n",scale,
                lambda_of_min,c_min,lambda_of_max,c_max,bound);
         for (i=0; i<nth; i++)
         {
            for (j=0; j<nt; j++)
            {
               r=scale*tc[i+j*nth];
               xc[i+j*nth]=r*sin(thetac[i+j*nth]);
               yc[i+j*nth]=r*cos(thetac[i+j*nth]);
            }
         }

         // ng_nu_m_th0 is the same as ng_nu, except the l=0 part of the solution has
         // been subtracted off, to give some measure of any asymmetry
         if (!(ng_nu_m_th0=galloc(2,COORD_SEMI_UNIFORM,shape,gd_n_pt,bbox,ng_nu->ccoords,ng->data)))
            { printf("sf_gd_concat_b: galloc failed\n"); goto clean_up; }
         for (j=0; j<nt; j++)
         {
            for (i=0, avg=0; i<nth; i++) avg+=ng->data[i+j*nth]; 
            avg/=nth;
            for (i=0; i<nth; i++) ng_nu_m_th0->data[i+j*nth]-=avg;
         }

         ng_nu->coords[2]=ng_nu_m_th0->coords[2]=bbox[2]*scale;
         ng_nu->coords[3]=ng_nu_m_th0->coords[3]=bbox[3]*scale;

         // ng_u and ng_u_m_th0 are uniform versions of ng_u and ng_nu_m_th0
         // ng_l_u[l] is spectral decomposition of ng_u
         shape_u[0]=nth*3;
         shape_u[1]=(shape_u[0]-1)*2+1;
         coords_u[0]=0; coords_u[1]=bound; dx=bound/(shape_u[0]-1);
         coords_u[2]=-bound; coords_u[3]=bound; dy=dx;
         if (!(ng_u=galloc(2,COORD_UNIFORM,shape_u,gd_n_pt,coords_u,0,0)))
         { printf("sf_gd_concat_b: galloc failed\n"); goto clean_up; }
         if (!(ng_u_m_th0=galloc(2,COORD_UNIFORM,shape_u,gd_n_pt,coords_u,0,0)))
         { printf("sf_gd_concat_b: galloc failed\n"); goto clean_up; }
         for (sl=0; sl<GD_LMAX; sl++)
            if (!(ng_u_l[sl]=galloc(1,COORD_UNIFORM,shape_u,gd_n_pt,coords_u,0,0)))
            { printf("sf_gd_concat_b: galloc failed\n"); goto clean_up; }

         dtheta=M_PI/(nth-1);
         if (!(plx=malloc(sizeof(double)*nth*GD_LMAX))) { printf("sf_gd_concat_b: out of memory\n"); goto clean_up; }
         for(i=0,theta=0; i<(nth-1); i++, theta+=dtheta)
         {
            x=cos(theta+dtheta/2);
            for(sl=0; sl<GD_LMAX; sl++) plx[i+sl*nth]=Pl_x(sl,x);
         }
         
         for(i=0; i<shape_u[0]; i++)
         {
            for(j=0; j<shape_u[1]; j++)
            {
               x0=i*dx+coords_u[0]; y0=j*dy+coords_u[2];
               if (y0==0) theta=M_PI/2;
               else if (y0<0) theta=M_PI-atan(-x0/y0); else theta=atan(x0/y0);
               r=sqrt(x0*x0+y0*y0);
               i0=theta/dtheta; if (i0>=(nth-1)) i0=nth-2;
               frac_th=(theta-(i0*dtheta))/dtheta;
               j0=1; while(j0<ng_nu->shape[1] && r>scale*((1-frac_th)*tc[i0+j0*nth]+frac_th*tc[i0+1+j0*nth])) j0++;
               if (j0==ng_nu->shape[1]) j0--;
               j0--;
               frac_r=(r-scale*tc[i0+j0*nth])/scale/((1-frac_th)*(tc[i0+(j0+1)*nth]-tc[i0+j0*nth])+
                                                     (  frac_th)*(tc[i0+1+(j0+1)*nth]-tc[i0+1+j0*nth]));
               frac_r=max(frac_r,0); frac_r=min(frac_r,1);
               ng_u->data[i+j*shape_u[0]]=(1-frac_th)*(1-frac_r)*ng_nu->data[i0+j0*nth]+
                                          (  frac_th)*(1-frac_r)*ng_nu->data[i0+1+j0*nth]+
                                          (1-frac_th)*(  frac_r)*ng_nu->data[i0+(j0+1)*nth]+
                                          (  frac_th)*(  frac_r)*ng_nu->data[i0+1+(j0+1)*nth];
               ng_u_m_th0->data[i+j*shape_u[0]]=(1-frac_th)*(1-frac_r)*ng_nu_m_th0->data[i0+j0*nth]+
                                          (  frac_th)*(1-frac_r)*ng_nu_m_th0->data[i0+1+j0*nth]+
                                          (1-frac_th)*(  frac_r)*ng_nu_m_th0->data[i0+(j0+1)*nth]+
                                          (  frac_th)*(  frac_r)*ng_nu_m_th0->data[i0+1+(j0+1)*nth];
            }
            // now calculate the spectral decomposition
            r=x0=i*dx+coords_u[0];
            i0=0;
            j0=1; while(j0<ng_nu->shape[1] && r>scale*(tc[i0+j0*nth])) j0++;
            if (j0==ng_nu->shape[1]) j0--;
            j0--;
            frac_r=(r-scale*tc[i0+j0*nth])/scale/(tc[i0+(j0+1)*nth]-tc[i0+j0*nth]);
            frac_r=max(frac_r,0); frac_r=min(frac_r,1);
            for (sl=0; sl<GD_LMAX; sl++)
            {
               spec=0;
               for (i0=0, theta=0; i0<(nth-1); i0++, theta+=dtheta)
               {
                  x=cos(theta+dtheta/2);
                  Dx=cos(theta+dtheta)-cos(theta);
                  // see gf_spec_coeff()
                  spec+=(-(sl+0.5)*plx[i0+sl*nth]*Dx)*((1-frac_r)*0.5*(ng_nu->data[i0+j0*nth]+ng_nu->data[i0+1+j0*nth])+
                                                      (  frac_r)*0.5*(ng_nu->data[i0+(j0+1)*nth]+ng_nu->data[i0+1+(j0+1)*nth]));
               }
               ng_u_l[sl]->data[i]=spec;
            }
         }
         add_grid_str(out_name_u,ng_u); ng_u=0;
         add_grid_str(out_name_diff_u,ng_u_m_th0); ng_u_m_th0=0;
         for (sl=0; sl<GD_LMAX; sl++) { add_grid_str(&out_name_u_l[sl][0],ng_u_l[sl]); ng_u_l[sl]=0; }
         free(plx); plx=0;

         // differentiate w.r.t theta
         for (j=0; j<nt; j++)
         {
            dtheta=ng2->ccoords[1+j*nth]-ng2->ccoords[0+j*nth];
            ng2->data[0+j*nth]=(-3*ng->data[0+j*nth]+4*ng->data[1+j*nth]-ng->data[2+j*nth])/2/dtheta;
            dtheta=ng2->ccoords[nth-1+j*nth]-ng2->ccoords[nth-2+j*nth];
            ng2->data[nth-1+j*nth]=(3*ng->data[nth-1+j*nth]-4*ng->data[nth-2+j*nth]+ng->data[nth-3+j*nth])/2/dtheta;
            for (i=1; i<(nth-1); i++)
            {
               dtheta=(ng2->ccoords[i+1+j*nth]-ng2->ccoords[i-1+j*nth])/2;
               ng2->data[i+j*nth]=(ng->data[i+1+j*nth]-ng->data[i-1+j*nth])/2/dtheta;
            }
         }

         if (error_state)
         {
            gfree(ng); gfree(ng2); gfree(ng_nu); gfree(ng_nu_m_th0); 
            error_state=0;
         }
         else
         {
            gfree(ng); // add_grid_str(?,ng);
            gfree(ng2); // add_grid_str(?,ng2);
            gfree(ng_nu); // add_grid_str(out_name_nu,ng_nu);
            gfree(ng_nu_m_th0); //add_grid_str(out_name_diff_nu,ng_nu_m_th0);
         }
         ng=ng2=ng_nu=ng_nu_m_th0=0;
      }
      if (nth>1) num++;
      t0_g=next_g(&t0_it);
      L_g=next_g(&L_it);
      lambda_g=next_g(&lambda_it);
      theta0_g=next_g(&theta0_it);
      if (gd_n0>0) n_g=next_g(&n_it);
      if (z_val) z_val_g=next_g(&z_val_it);
   }

clean_up:
   if (L) FREE_REG(L);
   if (lambda) FREE_REG(lambda);
   if (z_val) FREE_REG(z_val);
   if (t0) FREE_REG(t0);
   if (theta0) FREE_REG(theta0);
   if (gd_n) FREE_REG(gd_n);
   if (ng) gfree(ng);
   if (ng2) gfree(ng2);
   if (ng_nu) gfree(ng_nu);
   if (ng_u) gfree(ng_u);
   if (ng_nu_m_th0) gfree(ng_nu_m_th0);
   if (ng_u_m_th0) gfree(ng_u_m_th0);
   for (sl=0; sl<GD_LMAX; sl++) if (ng_u_l[sl]) gfree(ng_u_l[sl]);
}  

//------------------------------------------------------------------------------
// plots some function along a given geodesic(n), as a function of t or lambda
// also produces a file (lambda(t)), for coordinate transformations
//------------------------------------------------------------------------------
void sf_gd_concat_c(char *args)
{
   int i,found,z_valid=1,j;
   reg *z_val=0,*lambda=0,*nr=0; 
   char z_reg[MAX_NAME_LENGTH],out_name[MAX_NAME_LENGTH];
   char out_name_lt[MAX_NAME_LENGTH];
   int n,num,ind;
   s_iter z_val_it,lambda_it,n_it;
   grid *z_val_g=0,*ng,*lambda_g=0,*n_g=0,*ltg=0;
   double bbox[2],t,t_min,t_max;
   int shape[1],ltrace=1;
   int n0,use_l;

   z_reg[0]=0; // the register that will become the value of the new reg.

   if (args && (strlen(args)>0)) sscanf(args,"%i,%i,%s",&n0,&use_l,z_reg);

   printf("sf_gd_concat_c: n0=%i, use_l=%i, z-register=%s\n",n0,use_l,z_reg);

   if (use_l) 
   {
      sprintf(out_name,"gd_concat_c(%i,%s,lambda)",n0,z_reg);
      sprintf(out_name_lt,"lambda(t,%i)",n0);
      printf("lambda\t\t %s\n",z_reg);
   }
   else 
   {
      sprintf(out_name,"gd_concat_c(%i,%s,t)",n0,z_reg);
      printf("t\t\t %s\n",z_reg);
   }

   if (strlen(z_reg)==0) z_valid=0;

   if (!(nr=find_reg(gd_names[GD_N],1,1)))
      { printf("sf_gd_concat_c: cannot find register <%s>\n",gd_names[GD_N]); goto clean_up; }
   if (z_valid && !(strcmp(z_reg,gd_names[GD_N]))) z_valid=0;

   if (use_l)
   {
      if (!(lambda=find_reg(gd_names[GD_LAMBDA],1,1)))
         { printf("sf_gd_concat_c: cannot find register <%s>\n",gd_names[GD_LAMBDA]); goto clean_up; }
      if (z_valid && !(strcmp(z_reg,gd_names[GD_LAMBDA]))) z_valid=0;
   }

   if (z_valid)
   {
      if (!(z_val=find_reg(z_reg,1,1)))
         { printf("sf_gd_concat_c: cannot find register <%s>\n",z_reg); }
      z_valid=0;
   }

   if (use_l) lambda_g=init_s_iter(&lambda_it,lambda,0); else lambda_g=0;
   n_g=init_s_iter(&n_it,nr,0); 
   if (z_val) z_val_g=init_s_iter(&z_val_it,z_val,0); else z_val_g=0;

   save_s_iter(&n_it);
   if (use_l) save_s_iter(&lambda_it);
   num=0;
   t_min=1.0e10;
   t_max=0;
   while(n_g)
   {
      found=0;
      for (i=0; i<sizeof_data(n_g) && !(found); i++)
      {
         if (fuzzeq(n_g->data[i],(double)n0,1e-5)) 
         {
            num++;
            if (use_l) { t_min=min(t_min,lambda_g->data[i]);  t_max=max(t_max,lambda_g->data[i]); }
            else { t_min=min(t_min,n_g->time);  t_max=max(t_max,n_g->time); }
            found=1;
         }
      }
      n_g=next_g(&n_it);
      if (use_l) lambda_g=next_g(&lambda_it);
   }

   if (num>0)
   {
      shape[0]=num;
      if (!(ng=galloc(1,COORD_CURVILINEAR,shape,t_min,0,0,0)))
         { printf("sf_gd_concat_c: galloc failed\n"); goto clean_up; }
      if (use_l)
      {
         if (!(ltg=galloc(1,COORD_CURVILINEAR,shape,0.0,0,0,0)))
            { gfree(ng); printf("sf_gd_concat_c: galloc failed\n"); goto clean_up; }
      }

      n_g=restore_s_iter(&n_it);
      if (use_l) lambda_g=restore_s_iter(&lambda_it);
      num=0;
      while(n_g)
      {
         found=0;
         for (i=0; i<sizeof_data(n_g) && !(found); i++)
            if (fuzzeq(n_g->data[i],(double)n0,1e-5)) 
            {
               if (z_val) ng->data[num]=z_val_g->data[i]; else ng->data[num]=0;
               if (use_l) 
               { 
                  ng->coords[num]=lambda_g->data[i]; 
                  ltg->coords[num]=n_g->time;
                  ltg->data[num]=lambda_g->data[i];
               }
               else { ng->coords[num]=n_g->time; }
               printf("%16.12lf\t%lf\n",ng->coords[num],ng->data[num]);
               num++;
               found=1;
            }
         n_g=next_g(&n_it);
         if (use_l) lambda_g=next_g(&lambda_it);
         if (z_val) z_val_g=next_g(&z_val_it);
      }
      
      add_grid_str(out_name,ng);
      if (use_l) add_grid_str(out_name_lt,ltg);
   }
   else printf("sf_gd_concat_c ... no geodesic with number %i found\n",n0);

clean_up:
   if (lambda) FREE_REG(lambda);
   if (z_val) FREE_REG(z_val);
   if (nr) FREE_REG(nr);
}  

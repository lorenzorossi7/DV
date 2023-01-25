c======================================================================
c some graxi-specific functions
c======================================================================

c======================================================================
        subroutine separate_rb(f,fs,Nr,Nz)
        implicit none
        integer Nr,Nz
        real*8 f(Nr,Nz),fs(Nr,Nz)
 
        integer i,j,jo,io
 
        jo=1
        do i=1,Nr
           jo=mod(jo+1,2)
           do j=1,Nz,2
              if (j+jo.le.Nz) fs(i,1+(j-1)/2)=f(i,j+jo)
              if (j+1-jo.le.Nz) then
                 fs(i,1+(j-1)/2+(Nz-1)/2)=f(i,j+1-jo)
              else
                 fs(i,1+(j-1)/2+(Nz-1)/2)=f(i,j)
              end if
           end do
        end do  

        return
        end
             
c======================================================================
c average the entire function with a uniform weight stencil of
c size 2*n+1 x 2*n+1.
c
c if axis = 0 f(z=0)=0
c         = 1 df_drho(z=0)=0
c         = 2 just smooth
c======================================================================
        subroutine uni_avg(fn,fo,chr,CHR_excised,n,axis,Nrho,Nz)
        implicit none
        integer Nrho,Nz,axis,n
        real*8 fn(Nrho,Nz),fo(Nrho,Nz),chr(Nrho,Nz),CHR_excised
 
        integer i,j,i1,j1,wt
        real*8 sum
 
        do i=1,Nrho
           do j=1,Nz
              sum=0
              wt=0
              do i1=i-n,min(Nrho,i+n)
                 do j1=max(1,j-n),min(Nz,j+n)
                    if (i1.gt.0) then
                       if (chr(i1,j1).ne.CHR_excised) then
                          sum=sum+fo(i1,j1)
                          wt=wt+1
                       end if
                    else if (axis.eq.0) then
                       if (chr(-(i1-1)+1,j1).ne.CHR_excised) then
                          sum=sum-fo(-(i1-1)+1,j1)
                          wt=wt+1
                       end if
                    else if (axis.eq.1) then 
                       if (chr(-(i1-1)+1,j1).ne.CHR_excised) then
                          sum=sum+fo(-(i1-1)+1,j1)
                          wt=wt+1
                       end if
                    end if
                 end do
              end do
              if (wt.gt.0) then
                 fn(i,j)=sum/wt
              else
                 fn(i,j)=0
              end if
           end do
        end do
 
        return
        end   

c======================================================================
c apply an experimental KO filter to f (fn-new, fo-old)
c
c if axis = 0 zero f(z=0)
c         = 1 zero df_drho(z=0)
c         = 2 just backwards dissipate along axis
c
c k=1,2,... multiplies the characteristic wavelength targeted by the
c filter
c
c NOTE: eps is a number here
c
c m2 : 3,-14,26,-24,11,-2 [80]
c m1 : 2,-9,16,-14,6,-1 [48]
c
c======================================================================
        subroutine diss_ko2(fn,fo,chr,CHR_excised,eps,k,axis,Nrho,Nz)
        implicit none
        integer Nrho,Nz,axis,k
        real*8 fn(Nrho,Nz),fo(Nrho,Nz),eps,chr(Nrho,Nz) 
        real*8 CHR_excised
 
        integer i,j,is,imk,im2k,imeth,jmeth
        real*8 simk,sim2k
 
        do i=1,Nrho
           do j=1,Nz
              if (chr(i,j).eq.CHR_excised) then
                 fn(i,j)=0
              else
                 if (i.gt.(2*k).and.i.le.(Nrho-2*k)) then
                    if (chr(i-2*k,j).ne.CHR_excised) then
                       imeth=0
                    else if (chr(i-k,j).ne.CHR_excised) then
                       imeth=1
                    else
                       imeth=2
                    end if
                 else if (i.le.k) then
                    if (chr(1,j).ne.CHR_excised.and.axis.lt.2) then
                       imeth=0
                    else
                       imeth=2
                    end if
                 else if (i.le.2*k) then
                    if (chr(1,j).ne.CHR_excised.and.axis.lt.2) then
                       imeth=0
                    else if (chr(i-k,j).ne.CHR_excised) then
                       imeth=1
                    else
                       imeth=2
                    end if
                 else if (i.ge.(Nrho-k)) then
                    imeth=-2
                 else
                    imeth=-1
                 end if
                 if (j.gt.(2*k).and.j.le.(Nz-2*k)) then
                    if (chr(i,j-2*k).ne.CHR_excised.and.
     &                  chr(i,j+2*k).ne.CHR_excised) then
                       jmeth=0
                    else if (chr(i,j-k).eq.CHR_excised) then
                       jmeth=2
                    else if (chr(i,j-2*k).eq.CHR_excised) then
                       jmeth=2
                    else if (chr(i,j+k).eq.CHR_excised) then
                       jmeth=-2
                    else
                       jmeth=-1
                    end if
                 else if (j.le.k) then
                    jmeth=2
                 else if (j.le.2*k) then
                    if (chr(i,j-k).ne.CHR_excised) then
                       jmeth=1
                    else
                       jmeth=2
                    end if
                 else if (j.ge.(Nz-k)) then
                    jmeth=-2
                 else if (chr(i,j+k).eq.CHR_excised) then
                    jmeth=-2
                 else
                    jmeth=-1
                 end if
 
                 if (imeth.eq.0) then
                    imk=i-k
                    simk=1
                    if (imk.lt.1) then
                       imk=2-imk
                       !-----------------------------------------------
                       ! sim.k=-1 : applying filter assuming f is odd in rho
                       ! sim.k=1  : .. even
                       !-----------------------------------------------
                       if (axis.eq.0) simk=-1
                    end if
                    im2k=i-2*k
                    sim2k=1
                    if (im2k.lt.1) then
                       im2k=2-im2k
                       if (axis.eq.0) sim2k=-1
                    end if
                    fn(i,j)=fn(i,j)-(eps/16)*(
     &                    fo(i+2*k,j)-4*fo(i+1*k,j)+6*fo(i  ,j)
     &                    -4*simk*fo(imk,j)+sim2k*fo(im2k,j))
                 else if (imeth.eq.1) then
                    if (i+4*k.gt.Nrho) then
                       write(*,*) 'diss_ko2: k=',k,' too large'
                       return
                    end if
                    fn(i,j)=fn(i,j)-(eps/48)*(
     &                    2*fo(i-k,j)-9*fo(i,j)+16*fo(i+k,j)
     &                    -14*fo(i+2*k,j)+6*fo(i+3*k,j)-fo(i+4*k,j))
                 else if (imeth.eq.2) then
                    if (i+5*k.gt.Nrho) then
                       write(*,*) 'diss_ko2: k=',k,' too large'
                       return
                    end if
                    fn(i,j)=fn(i,j)-(eps/80)*(
     &                    3*fo(i,j)-14*fo(i+k,j)+26*fo(i+2*k,j)
     &                    -24*fo(i+3*k,j)+11*fo(i+4*k,j)-2*fo(i+5*k,j))
                 else if (imeth.eq.-1) then
                    if (i-4*k.lt.1) then
                       write(*,*) 'diss_ko2: k=',k,' too large'
                       return
                    end if
                    fn(i,j)=fn(i,j)-(eps/48)*(
     &                    2*fo(i+k,j)-9*fo(i,j)+16*fo(i-k,j)
     &                    -14*fo(i-2*k,j)+6*fo(i-3*k,j)-fo(i-4*k,j))
                 else if (imeth.eq.-2) then
                    if (i-5*k.lt.1) then
                       write(*,*) 'diss_ko2: k=',k,' too large'
                       return
                    end if       
                   fn(i,j)=fn(i,j)-(eps/80)*(
     &                    3*fo(i,j)-14*fo(i-k,j)+26*fo(i-2*k,j)
     &                    -24*fo(i-3*k,j)+11*fo(i-4*k,j)-2*fo(i-5*k,j))
                 end if
                 if (jmeth.eq.0) then
                    fn(i,j)=fn(i,j)-(eps/16)*(
     &                      fo(i,j+2*k)-4*fo(i,j+k)+6*fo(i,j)
     &                      -4*fo(i,j-k)+fo(i,j-2*k))
                 else if (jmeth.eq.1) then
                    if (j+4*k.gt.Nz) then
                       write(*,*) 'diss_ko2: k=',k,' too large'
                       return
                    end if
                    fn(i,j)=fn(i,j)-(eps/48)*(
     &                    2*fo(i,j-k)-9*fo(i,j)+16*fo(i,j+k)
     &                    -14*fo(i,j+2*k)+6*fo(i,j+3*k)-fo(i,j+4*k))
                 else if (jmeth.eq.2) then
                    if (j+5*k.gt.Nz) then
                       write(*,*) 'diss_ko2: k=',k,' too large'
                       return
                    end if
                    fn(i,j)=fn(i,j)-(eps/80)*(
     &                    3*fo(i,j)-14*fo(i,j+k)+26*fo(i,j+2*k)
     &                    -24*fo(i,j+3*k)+11*fo(i,j+4*k)-2*fo(i,j+5*k))
                 else if (jmeth.eq.-1) then
                    if (j-4*k.lt.1) then
                       write(*,*) 'diss_ko2: k=',k,' too large'
                       return
                    end if      
                    fn(i,j)=fn(i,j)-(eps/48)*(
     &                      2*fo(i,j+k)-9*fo(i,j)+16*fo(i,j-k)
     &                      -14*fo(i,j-2*k)+6*fo(i,j-3*k)-fo(i,j-4*k))
                 else if (jmeth.eq.-2) then
                    if (j-5*k.lt.1) then
                       write(*,*) 'diss_ko2: k=',k,' too large'
                       return
                    end if
                    fn(i,j)=fn(i,j)-(eps/80)*(
     &                    3*fo(i,j)-14*fo(i,j-k)+26*fo(i,j-2*k)
     &                    -24*fo(i,j-3*k)+11*fo(i,j-4*k)-2*fo(i,j-5*k))
                 end if
              end if
           end do
        end do 
 
        return
        end       

c======================================================================
c df=d(f/rho)/drho
c======================================================================
        subroutine d_overrho(f,df,chr,CHR_excised,
     &             rhomin,rhomax,Nrho,Nz)
        implicit none
        integer Nrho,Nz
        real*8 f(Nrho,Nz),df(Nrho,Nz),chr(Nrho,Nz)
        real*8 rhomin,rhomax,CHR_excised
 
        integer i,j
        real*8 drho,rho,rhoph,rhomh,rhop1,rhop2,rhom1,rhom2

        drho=(rhomax-rhomin)/(Nrho-1)

        do i=1,Nrho
           rho=(i-1)*drho+rhomin
           rhoph=rho+drho/2
           rhomh=rho-drho/2
           rhop1=rho+drho
           rhop2=rho+2*drho
           rhom1=rho-drho
           rhom2=rho-2*drho
           do j=1,Nz
              if (chr(i,j).eq.CHR_excised.or.i.eq.1) then
                 df(i,j)=0
              else if (i.eq.Nrho) then
                 df(i,j)=(3*f(i,j)/rho-4*f(i-1,j)/rhom1+
     &                   f(i-2,j)/rhom2)/2/drho
              else if (chr(i-1,j).eq.CHR_excised) then
                 df(i,j)=(-3*f(i,j)/rho+4*f(i+1,j)/rhop1-
     &                   f(i+2,j)/rhop2)/2/drho
              else
                 df(i,j)=((f(i+1,j)+f(i,j))/rhoph-
     &                    (f(i,j)+f(i-1,j))/rhomh)/2/drho
              end if
           end do
        end do

        return
        end

c======================================================================
c df=d( [df/drho]/rho )/drho
c======================================================================
        subroutine d_overrho_d(f,df,chr,CHR_excised,
     &             rhomin,rhomax,Nrho,Nz)
        implicit none
        integer Nrho,Nz
        real*8 f(Nrho,Nz),df(Nrho,Nz),chr(Nrho,Nz)
        real*8 rhomin,rhomax,CHR_excised
 
        integer i,j
        real*8 drho,rho,rhoph,rhomh,rhop1,rhop2,rhom1,rhom2

        drho=(rhomax-rhomin)/(Nrho-1)

        do i=1,Nrho
           rho=(i-1)*drho+rhomin
           rhoph=rho+drho/2
           rhomh=rho-drho/2
           rhop1=rho+drho
           rhop2=rho+2*drho
           rhom1=rho-drho
           rhom2=rho-2*drho
           do j=1,Nz
              if (chr(i,j).eq.CHR_excised.or.i.eq.1) then
                 df(i,j)=0
              else if (i.eq.Nrho) then
                 df(i,j)=(3*(3*f(i,j)-4*f(i-1,j)+f(i-2,j))/2/drho/rho-
     &                    4*(f(i,j)-f(i-2,j))/2/drho/rhom1+
     &                      (f(i-1,j)-f(i-3,j))/2/drho/rhom2)/2/drho
              else if (chr(i-1,j).eq.CHR_excised.and.(i+3).le.Nrho) then
                 df(i,j)=(((12*rho+11*drho)/(6*rho**2))*f(i,j) -
     &                    ((5*rho+3*drho)/(rho**2))*f(i+1,j) +
     &                    ((8*rho+3*drho)/(2*rho**2))*f(i+2,j) -
     &                    ((3*rho+drho)/(3*rho**2))*f(i+3,j))/
     &                    (drho**2) 
              else if (chr(i-1,j).eq.CHR_excised) then
                 df(i,j)=0 ! NOT HANDLING YET
                 write(*,*) 'd_overrho_d : unexpected chr!'
              else
                 df(i,j)=((f(i+1,j)-f(i,j))/rhoph-
     &                    (f(i,j)-f(i-1,j))/rhomh)/drho/drho
              end if
           end do
        end do

        return
        end

c======================================================================
c df=d(f)/d(rho^2)=1/2/rho df/drho
c======================================================================
        subroutine d_byrhosq(f,df,chr,CHR_excised,
     &             rhomin,rhomax,Nrho,Nz)
        implicit none
        integer Nrho,Nz
        real*8 f(Nrho,Nz),df(Nrho,Nz),chr(Nrho,Nz)
        real*8 rhomin,rhomax,CHR_excised
 
        integer i,j
        real*8 drho,rho,rhoph,rhomh,rhop1,rhop2,rhom1,rhom2

        drho=(rhomax-rhomin)/(Nrho-1)

        do i=1,Nrho
           rho=(i-1)*drho+rhomin
           rhoph=rho+drho/2
           rhomh=rho-drho/2
           rhop1=rho+drho
           rhop2=rho+2*drho
           rhom1=rho-drho
           rhom2=rho-2*drho
           do j=1,Nz
              if (chr(i,j).eq.CHR_excised.or.i.eq.1) then
                 df(i,j)=0
              else if (i.eq.Nrho) then
                 df(i,j)=(3*f(i,j)-4*f(i-1,j)+f(i-2,j))/4/drho/rho
              else if (chr(i-1,j).eq.CHR_excised.and.(i+2).le.Nrho) then
                 df(i,j)=(-3*f(i,j)+4*f(i+1,j)-f(i+2,j))/4/drho/rho
              else if (chr(i-1,j).eq.CHR_excised) then
                 df(i,j)=0 ! NOT HANDLING YET
                 write(*,*) 'd_byrhosq : unexpected chr!'
              else
                 df(i,j)=(f(i+1,j)-f(i-1,j))/(rhop1**2-rhom1**2)
              end if
           end do
        end do

        return
        end

c======================================================================
c Legendre polynomials Pl:
c P(0)=1
c P(1)=x
c P(l>=2)=((2*l-1)*x*P(l-1,x)-(l-1)*P(l-2,x))/l
c======================================================================
        real*8 function pl_x(x,l)
        implicit none
        real*8 x
        integer l

        integer cl
        real pl,plm1,tp

        if (l.eq.0) then
           pl_x=1
        else if (l.eq.0) then
           pl_x=x
        else
           pl=x
           plm1=1
           do cl=2,l
              tp=pl
              pl=((2*cl-1)*x*pl-(cl-1)*plm1)/cl
              plm1=tp
           end do
           pl_x=pl
        end if

        return
        end

c======================================================================
c Associated Legendre Polynomials Plm ... 0 <= m <= l
c
c See Numerical Recipes 6.8
c
c Pmm(x) = (-1)^m (2m-1)!! (1-x^2)^(m/2)
c
c where n!! is the product of odd integers less than or equal to n
c
c Pm+1_m(x)=x (2m+1) Pmm
c
c (l-m) Plm = x(2l-1)Pl-1_m - (l+m-1)Pl-2_m
c======================================================================
        real*8 function plm_x(x,l,m)
        implicit none
        real*8 x,pl_x
        integer l,m

        real*8 plm2_m,plm1_m,plm
        integer df,i

        if (m.eq.0) then
           plm_x=pl_x(x,l)
           return 
        end if

        plm_x=0

        if (m.lt.0.or.m.gt.l) then
           write(*,*) 'plm_x: error ... m out of bounds'
           return
        end if

        if (abs(x).gt.1) then
           write(*,*) 'plm_x: error ... x out of bounds'
           return
        end if

        df=-1
        do i=3,2*m-1,2
          df=-df*i
        end do

        ! pmm
        plm2_m=df*(1-x*x)**(0.5d0*m)
        if (l.eq.m) then
           plm_x=plm2_m
           return
        end if

        ! pm+1_m
        plm1_m=x*(2*m+1)*plm2_m
        if (l.eq.(m+1)) then
           plm_x=plm1_m
           return
        end if

        do i=m+2,l
           plm_x=(x*(2*i-1)*plm1_m-(i+m-1)*plm2_m)/(i-m)
           plm2_m=plm1_m
           plm1_m=plm_x
        end do

        return
        end

c======================================================================
c computes the complex Ylm spectral coefficient of a 
c complex function f(theta,phi) ... second order accurate:
c
c clm=int(f * bar(Ylm), theta = 0..Pi, phi = 0..2*pi)
c
c f_r=Re(f), f_i=Im(f)
c
c clm_r=Re(clm), crm_i=Im(clm)
c
c See Numerical Recipes sec 6.8 for definitions
c 
c======================================================================
        subroutine cylm_coeff(clm_r,clm_i,f_r,f_i,l,m,Ntheta,Nphi)
        implicit none
        integer Ntheta,Nphi,l,m
        real*8 f_r(Ntheta,Nphi),f_i(Ntheta,Nphi)
        real*8 clm_r,clm_i

        integer i,j,am
        real*8 x,plm_x,norm,k1,k2,dtheta,dphi,st,sum_r,sum_i,plm0
        real*8 f_r0,f_i0,smp,cmp,sgnm,cpi,f_r0e,f_i0e
        parameter (cpi=3.14159265359d0)

        clm_r=0
        clm_i=0

        am=abs(m)
        sgnm=1
        if (m.ne.0) sgnm=m/am

        if (am.gt.l) then
           write(*,*) 'cylm_coeff: error |m|>l'
           return
        end if

        ! normalization

        k1=1
        k2=1
        do i=2,l+am
           k1=k1*i
        end do
        do i=2,l-am
           k2=k2*i
        end do

        norm=sqrt((2*l+1)*k2/4/cpi/k1)

        ! because we compute plm0 with m>=0:
        if (m.lt.0) norm=norm*(-1)**m

        dtheta=cpi/(Ntheta-1)
        dphi=2*cpi/(Nphi-1)

        sum_r=0
        sum_i=0

        do i=1,Ntheta-1
           st=sin((i-0.5d0)*dtheta)
           x=cos((i-0.5d0)*dtheta)
           do j=1,Nphi-1
              smp=sin((j-0.5d0)*dphi*(-am*sgnm))
              cmp=cos((j-0.5d0)*dphi*(-am*sgnm))
              plm0=plm_x(x,l,am)
              if (i.eq.1) then
                 f_r0e=2*f_r(i+1,j)-f_r(i+2,j)+
     &                 2*f_r(i+1,j+1)-f_r(i+2,j+1)
                 f_i0e=2*f_i(i+1,j)-f_i(i+2,j)+
     &                 2*f_i(i+1,j+1)-f_i(i+2,j+1)
                 f_r0=(f_r0e+f_r(i+1,j)+f_r(i+1,j+1))/4
                 f_i0=(f_i0e+f_i(i+1,j)+f_i(i+1,j+1))/4
              else if (i.eq.Ntheta-1) then
                 f_r0e=2*f_r(i,j)-f_r(i-1,j)+
     &                 2*f_r(i,j+1)-f_r(i-1,j+1)
                 f_i0e=2*f_i(i,j)-f_i(i-1,j)+
     &                 2*f_i(i,j+1)-f_i(i-1,j+1)
                 f_r0=(f_r0e+f_r(i,j)+f_r(i,j+1))/4
                 f_i0=(f_i0e+f_i(i,j)+f_i(i,j+1))/4
              else
                 f_r0=(f_r(i,j)+f_r(i+1,j)+f_r(i,j+1)+f_r(i+1,j+1))/4
                 f_i0=(f_i(i,j)+f_i(i+1,j)+f_i(i,j+1)+f_i(i+1,j+1))/4
              end if

              sum_r=sum_r+plm0*(f_r0*cmp-f_i0*smp)*st
              sum_i=sum_i+plm0*(f_r0*smp+f_i0*cmp)*st
           end do
        end do

        clm_r=sum_r*dtheta*dphi*norm
        clm_i=sum_i*dtheta*dphi*norm

        return
        end

c======================================================================
c The following computes the spin -2, l=2, m=+-2 complex harmonic
c coefficient of f ... for the moment keeping l and m as arguments,
c though must be 2, +-2
c======================================================================
        subroutine cm2y2pm2_coeff(clm_r,clm_i,f_r,f_i,l,m,Ntheta,Nphi)
        implicit none
        integer Ntheta,Nphi,l,m
        real*8 f_r(Ntheta,Nphi),f_i(Ntheta,Nphi)
        real*8 clm_r,clm_i

        integer i,j,am
        real*8 x,norm,dtheta,dphi,st,sum_r,sum_i,xfac
        real*8 f_r0,f_i0,smp,cmp,sgnm,cpi
        parameter (cpi=3.14159265359d0)

        clm_r=0
        clm_i=0

        if (l.ne.2.or.abs(m).ne.2) then
           write(*,*) 'cm2y2pm2_coeff: l and |m| must be 2'
           return
        end if

        am=abs(m)
        sgnm=1
        if (m.ne.0) sgnm=m/am

        ! normalization

        norm=sqrt(5.0d0/(4*cpi))

        dtheta=cpi/(Ntheta-1)
        dphi=2*cpi/(Nphi-1)

        sum_r=0
        sum_i=0

        do i=1,Ntheta-1
           st=sin((i-0.5d0)*dtheta)
           x=cos((i-0.5d0)*dtheta)
           do j=1,Nphi-1
              smp=sin((j-0.5d0)*dphi*(-am*sgnm))
              cmp=cos((j-0.5d0)*dphi*(-am*sgnm))
              if (m.eq.2) then
                 xfac=0.25d0*(1+x)**2
              else
                 xfac=0.25d0*(1-x)**2
              end if
              if (i.eq.1) then
                 f_r0=(f_r(i+1,j)+f_r(i+1,j+1))/4
                 f_i0=(f_i(i+1,j)+f_i(i+1,j+1))/4
              else if (i.eq.Ntheta-1) then
                 f_r0=(f_r(i,j)+f_r(i,j+1))/4
                 f_i0=(f_i(i,j)+f_i(i,j+1))/4
              else
                 f_r0=(f_r(i,j)+f_r(i+1,j)+f_r(i,j+1)+f_r(i+1,j+1))/4
                 f_i0=(f_i(i,j)+f_i(i+1,j)+f_i(i,j+1)+f_i(i+1,j+1))/4
              end if

              sum_r=sum_r+xfac*(f_r0*cmp-f_i0*smp)*st
              sum_i=sum_i+xfac*(f_r0*smp+f_i0*cmp)*st
           end do
        end do

        clm_r=sum_r*dtheta*dphi*norm
        clm_i=sum_i*dtheta*dphi*norm

        return
        end
c======================================================================
c The following computes the spin -2, l=2..6, |m|<=2 complex harmonic
c coefficient of f 
c
c the (l,m) component is returned in clm_r(l-1,m+l+1), clm_i(l-1,m+l+1)
c======================================================================
        subroutine cm2yl6m_coeff(clm_r,clm_i,f_r,f_i,Ntheta,Nphi)
        implicit none
        integer Ntheta,Nphi
        real*8 f_r(Ntheta,Nphi),f_i(Ntheta,Nphi)
        real*8 clm_r(5,13),clm_i(5,13)

        integer i,j,l,m
        real*8 x,dtheta,dphi,st
        real*8 xfac,q(5,13)
        real*8 f_r0,f_i0,smp,cmp,cpi,f_r0e,f_i0e
        parameter (cpi=3.14159265359d0)

        do i=1,5
           do j=1,13
              clm_r(i,j)=0
              clm_i(i,j)=0
           end do
        end do

        dtheta=cpi/(Ntheta-1)
        dphi=2*cpi/(Nphi-1)

        do i=1,Ntheta-1
           st=sin((i-0.5d0)*dtheta)
           x=cos((i-0.5d0)*dtheta)
           call eval_m2ylm_l6(q,x)
           do j=1,Nphi-1
              ! extrapolate to axis
              if (i.eq.1) then
                 f_r0e=2*f_r(i+1,j)-f_r(i+2,j)+
     &                 2*f_r(i+1,j+1)-f_r(i+2,j+1)
                 f_i0e=2*f_i(i+1,j)-f_i(i+2,j)+
     &                 2*f_i(i+1,j+1)-f_i(i+2,j+1)
                 f_r0=(f_r0e+f_r(i+1,j)+f_r(i+1,j+1))/4
                 f_i0=(f_i0e+f_i(i+1,j)+f_i(i+1,j+1))/4
              else if (i.eq.Ntheta-1) then
                 f_r0e=2*f_r(i,j)-f_r(i-1,j)+
     &                 2*f_r(i,j+1)-f_r(i-1,j+1)
                 f_i0e=2*f_i(i,j)-f_i(i-1,j)+
     &                 2*f_i(i,j+1)-f_i(i-1,j+1)
                 f_r0=(f_r0e+f_r(i,j)+f_r(i,j+1))/4
                 f_i0=(f_i0e+f_i(i,j)+f_i(i,j+1))/4
              else
                 f_r0=(f_r(i,j)+f_r(i+1,j)+f_r(i,j+1)+
     &                 f_r(i+1,j+1))/4
                 f_i0=(f_i(i,j)+f_i(i+1,j)+f_i(i,j+1)+
     &                 f_i(i+1,j+1))/4
              end if
              do l=2,6
                 do m=-l,l
                    smp=sin((j-0.5d0)*dphi*(-m))
                    cmp=cos((j-0.5d0)*dphi*(-m))
                       
                    clm_r(l-1,l+m+1)=clm_r(l-1,l+m+1)+
     &                 q(l-1,l+m+1)*(f_r0*cmp-f_i0*smp)*st*
     &                 dtheta*dphi
                    clm_i(l-1,l+m+1)=clm_i(l-1,l+m+1)+
     &                 q(l-1,l+m+1)*(f_r0*smp+f_i0*cmp)*st*
     &                 dtheta*dphi
                 end do
              end do
           end do
        end do

        return
        end
c======================================================================
c for testing ... fills f_r, f_i with requested ylm mode
c======================================================================
        subroutine fill_cylm(f_r,f_i,l,m,Ntheta,Nphi)
        implicit none
        integer Ntheta,Nphi,l,m
        real*8 f_r(Ntheta,Nphi),f_i(Ntheta,Nphi)

        integer i,j,am
        real*8 x,plm_x,norm,k1,k2,dtheta,dphi,plm0
        real*8 smp,cmp,sgnm,cpi
        parameter (cpi=3.14159265359d0)

        am=abs(m)
        sgnm=1
        if (m.ne.0) sgnm=m/am

        if (am.gt.l) then
           write(*,*) 'cylm_coeff: error |m|>l'
           return
        end if

        ! normalization

        k1=1
        k2=1
        do i=2,l+am
           k1=k1*i
        end do
        do i=2,l-am
           k2=k2*i
        end do

        norm=sqrt((2*l+1)*k2/4/cpi/k1)
        if (m.lt.0) norm=norm*(-1)**m

        dtheta=cpi/(Ntheta-1)
        dphi=2*cpi/(Nphi-1)

        do i=1,Ntheta
           x=cos((i-1)*dtheta)
           do j=1,Nphi
              smp=sin((j-1)*dphi*(am*sgnm))
              cmp=cos((j-1)*dphi*(am*sgnm))
              plm0=plm_x(x,l,am)
              f_r(i,j)=norm*plm0*cmp
              f_i(i,j)=norm*plm0*smp
           end do
        end do

        return
        end
c======================================================================
c evaluates q= _{-2} Y_{l,m}/e^{im\phi} at the given x=cos(theta).
c FOR ALL VALUES OF l,m with 2<=l<=6.
c
c mapole generated ... see maple_cm2ylm
c
c returns l,m coefficient in q[l-1,l+m+1]
c
c m<=l
c l>=|-2|
c======================================================================
        subroutine eval_m2ylm_l6(q,x)
        implicit real*8 (t)
        real*8 q(5,13),x

        include 'm2yl6m.inc'

        return 
        end

c======================================================================
c for testing ... fills f_r, f_i with requested spin -2 l=2, m=+-2
c mode mode
c======================================================================
        subroutine fill_cm2y2pm2(f_r,f_i,l,m,Ntheta,Nphi)
        implicit none
        integer Ntheta,Nphi,l,m
        real*8 f_r(Ntheta,Nphi),f_i(Ntheta,Nphi)

        integer i,j,am
        real*8 x,xfac,dtheta,dphi
        real*8 smp,cmp,sgnm,cpi
        parameter (cpi=3.14159265359d0)
        real*8 q(5,13)

        if (l.gt.6.or.l.lt.2.or.abs(m).gt.l) then
           write(*,*) 'fill_cm2y2pm2: l must be 2..6, |m|<=l'
           return
        end if

        am=abs(m)
        sgnm=1
        if (m.ne.0) sgnm=m/am

        dtheta=cpi/(Ntheta-1)
        dphi=2*cpi/(Nphi-1)

        do i=1,Ntheta
           x=cos((i-1)*dtheta)
           call eval_m2ylm_l6(q,x)
           do j=1,Nphi
              smp=sin((j-1)*dphi*(am*sgnm))
              cmp=cos((j-1)*dphi*(am*sgnm))
              xfac=q(l-1,l+m+1)
              f_r(i,j)=xfac*cmp
              f_i(i,j)=xfac*smp
           end do
        end do

        return
        end

c======================================================================
c integrates a function f^n*Pl(l,x) along a surface of constant r
c with flat space 2-metric. Choose r at least a few points within
c the outer boundaries.
c The area/(4pir^2)=one is returned as a measure of the accuracy.
c fc=f(rho=r,z=0)
c
c Pl(l,x) are the Legendre polynomials above, with x=cos(theta)=z/r
c======================================================================
        subroutine int_f_ds(f,intf,r,one,n,l,fc,rhomin,rhomax,
     &         zmin,zmax,Nrho,Nz)
        implicit none
        integer Nrho,Nz,l
        real*8 f(Nrho,Nz),intf,r,one,n,rhomin,rhomax,zmin,zmax,fc
        real*8 pl_x,x
 
        include 'chr.inc'
 
        logical done
        integer i,j
        real*8 rho0,z0,darea,dxn,drho0,dz0,area
        real*8 r_rho,r_z,r0,drho,dz,frac_rho,frac_z,f0,cpi
        parameter (cpi=3.14159265359d0)
 
        area=0
        one=0
        intf=0
 
        drho=(rhomax-rhomin)/(Nrho-1)
        dz=(zmax-zmin)/(Nz-1)

        j=(r-zmin)/dz+1
        if (j.ge.Nz.or.j.le.1) then
           write(*,*) 'int_f_ds: r=',r,' outside of grid'
           return
        end if
        if (rhomin.ge.drho) then
           write(*,*) 'int_f_ds: currently rhomin must equal zero'
           return
        end if
        if (r.eq.0) return

        rho0=0
        z0=r
        done=.false.
        do while(.not.done)
           i=(rho0-rhomin)/drho+1
           j=(z0-zmin)/dz+1
           if (i.gt.Nrho-3) done=.true.
           if (j.gt.Nz-3.or.j.lt.4) done=.true.
           r_rho=rho0/r
           r_z=z0/r
           !-----------------------------------------------------
           ! tangent to r=const is in (r_z,-r_rho) dir; normalize
           ! so that maximum change in rho or z is drho/4:
           !-----------------------------------------------------
           dxn=max(abs(r_z),abs(r_rho))
           drho0=(r_z/dxn)*drho/4
           dz0=(-r_rho/dxn)*drho/4
           darea=2*cpi*rho0*sqrt(drho0**2+dz0**2)
           area=area+darea
           frac_rho=(rho0-(i-1)*drho)/drho
           frac_z=(z0-(j-1)*dz-zmin)/dz
           f0=       frac_rho *   frac_z *f(i+1,j+1)+
     &            (1-frac_rho)*   frac_z *f(i,j+1)+
     &            (  frac_rho)*(1-frac_z)*f(i+1,j)+
     &            (1-frac_rho)*(1-frac_z)*f(i,j) 
           x=z0/r
           intf=intf+(f0**n)*darea*pl_x(x,l)
           rho0=rho0+drho0
           z0=z0+dz0
           if (rho0.lt.0) then
              done=.true.
           end if
        end do

        one=area/4/cpi/r/r
        j=(0-zmin)/dz+1
        i=r/drho+1
        fc=0
        if (i.lt.Nrho.and.j.lt.Nz.and.j.gt.1) fc=f(i,j)
 
        return
        end   

c======================================================================
c computes the z-axis proper distance between two excised regions,
c or if not excised, then between maxima of psi
c======================================================================
        subroutine distance_2BH(psi,chr,d,dz,Nrho,Nz)
        implicit none
        integer Nrho,Nz
        real*8 psi(Nrho,Nz),chr(Nrho,Nz)
        real*8 dz,d

        include 'chr.inc'

        integer j,reg

        reg=1
        d=0
        do j=2,Nz-1
           if (chr(1,j).ne.CHR_excised.and.reg.eq.3) then
              d=d+psi(1,j)*psi(1,j)*dz
           end if
           if (chr(1,j+1).eq.CHR_excised.and.
     &         chr(1,j).ne.CHR_excised) reg=reg+1
           if (chr(1,j+1).ne.CHR_excised.and.
     &         chr(1,j).eq.CHR_excised) reg=reg+1
        end do

        if (reg.eq.1) then
           d=0
           do j=2,Nz-1
              if (reg.eq.2) d=d+psi(1,j)*psi(1,j)*dz
              if (psi(1,j).gt.psi(1,j+1).and.
     &            psi(1,j).gt.psi(1,j-1)) reg=reg+1
           end do

           ! set to -1 if more than 2 maxima
           if (reg.gt.3) d=-1
        else if (reg.eq.3) then   ! single hole
           d=0
        end if

        return
        end

c======================================================================
c eval --- evaluate a UNIFORM grid function f at a point rho0, z0,
c using order n interpolation (only n=0/1, nearest point/linear).
c The interpolation only uses points from the non-excised part
c of the grid, as specified via chr
c When n=0, if the requested point is within the excised zone,
c then, based upon ct (cos(theta)), can move 1 point away to
c find f.
c
c k (>=1) defines the basic cell block used to extrapolate
c======================================================================
        real*8 function eval(f,rho0,z0,n,k,ct,chr,rho,z,Nrho,Nz)
        implicit none
        integer Nrho,Nz,n,k
        real*8 f(Nrho,Nz),chr(Nrho,Nz)
        real*8 rho(Nrho),z(Nz)
        real*8 rho0,z0,ct
 
        include 'chr.inc'
 
        integer i,j,ns,s,done
        parameter (ns=15)
        integer delta_i(ns),delta_j(ns),dj
        real*8 drho,dz,rhof,zf   
 
        logical ltrace
        parameter (ltrace=.false.)
 
        data delta_i/1,0,-1,-1, 0, 0,1,1, 0,-1,-1,-1,0,0,0/
        data delta_j/0,1, 0, 0,-1,-1,0,0,-1, 0, 0, 0,1,1,1/
 
        drho=rho(2)-rho(1)
        dz=z(2)-z(1)
        eval=0
 
        ! locate nearest grid point (round off)
 
        i=(rho0-rho(1))/drho+1.49d0
        j=(z0-z(1))/dz+1.49d0
 
c        if (ltrace) write(0,*) 'eval: i0,j0=',i,j
 
        if (i.lt.1 .or. i.gt.Nrho .or. j.lt.1 .or. j.gt.Nz) then
c           write(0,*) 'eval: error --- out of function range'
c           write(0,*) '      i,j=',i,j,' ;  Nrho,Nz=',Nrho,Nz,
c     &                ' rho0,z0=',rho0,z0
           return
        end if
        if (n.ne.1 .and. n.ne.0) then
c          write(0,*) 'eval: only n=0 - round to nearest point' 
c          write(0,*) '           n=1 - linear interpolation supported'
c          stop
        end if
 
        if (n.eq.0) then
           if (chr(i,j).eq.CHR_excised) then
              if (ct.gt.(1/sqrt(2.0d0))) then
                 j=j-1
              else if (ct.lt.(-1/sqrt(2.0d0))) then
                 j=j+1
              else
                 i=i+1
              end if
              if (chr(i,j).eq.CHR_excised) then
c                 write(0,*) 'eval: Error -- requested evaluation',
c    &                  ' point is in the excised zone'
              else
                 eval=f(i,j)
              end if
           else
              eval=f(i,j)
           end if
           return
        end if
 
        ! define (i,j) to be lower left corner of cell 
        ! that will be used to interpolate from
 
        if (i.eq.Nrho) then
           i=i-1
        end if
        if (j.eq.Nz) then
          j=j-1
        end if
 
        !search for a non-excised-cell in a prescribed pattern
        done=0
        s=0
        do while(done.eq.0 .and. s.lt.ns)
           if (((i+k).le.Nrho.and.(j+k).le.Nz
     &          .and.i.gt.0.and.j.gt.k).and.
     &         (chr(i,j).ne.CHR_excised).and.
     &         (chr(i,j).ne.CHR_repopulate).and.
     &         (chr(i+k,j).ne.CHR_excised).and.
     &         (chr(i+k,j).ne.CHR_repopulate).and.
     &         (chr(i,j+k).ne.CHR_excised).and.
     &         (chr(i,j+k).ne.CHR_repopulate).and.
     &         (chr(i+k,j+k).ne.CHR_excised).and.
     &         (chr(i+k,j+k).ne.CHR_repopulate)) then
              done=1
              dj=1
           else if (((i+k).le.Nrho.and.(j-k).le.Nz
     &          .and.i.gt.0.and.j.gt.k).and.
     &         (chr(i,j).ne.CHR_excised).and.
     &         (chr(i,j).ne.CHR_repopulate).and.
     &         (chr(i+k,j).ne.CHR_excised).and.
     &         (chr(i+k,j).ne.CHR_repopulate).and.
     &         (chr(i,j-k).ne.CHR_excised).and.
     &         (chr(i,j-k).ne.CHR_repopulate).and.
     &         (chr(i+k,j-k).ne.CHR_excised).and.
     &         (chr(i+k,j-k).ne.CHR_repopulate)) then
              done=1
              dj=-1
           else
              s=s+1
              i=i+delta_i(s)         
              j=j+delta_j(s)
           end if
        end do
 
        if (done.ne.0) then
           rhof=(rho0-rho(i))/drho/k
           zf=dj*(z0-z(j))/dz/k
           eval=(1-rhof)*(1-zf)*f(i  ,j)+
     &          (  rhof)*(1-zf)*f(i+k,j)+
     &          (  rhof)*(  zf)*f(i+k,j+dj*k)+
     &          (1-rhof)*(  zf)*f(i  ,j+dj*k)
        else
c           write(0,*) 'eval: Error -- requested evaluation point ',
c    &                'is too far within excised zone'  
           return
        end if
 
        return
        end       

c======================================================================
c a simplified version of eval above, that only extrapolates
c (linearly) along grid lines (for excision only ... 
c doesn't interpolate) ... k is the step size
c======================================================================
        real*8 function eval0(f,i,j,k,chr,Nrho,Nz)
        implicit none
        integer Nrho,Nz,k,i,j
        real*8 f(Nrho,Nz),chr(Nrho,Nz)
 
        include 'chr.inc'
 
        real*8 frac,f1,f2,sum,f3
 
        logical ltrace
        parameter (ltrace=.false.)
 
        eval0=0

        if (i.lt.1 .or. i.gt.Nrho .or. j.lt.1 .or. j.gt.Nz) then
           write(0,*) 'eval0: error --- out of function range'
           write(0,*) '      i,j=',i,j,' ;  Nrho,Nz=',Nrho,Nz
           return
        end if

        if (chr(i,j).ne.CHR_excised.and.chr(i,j).ne.CHR_repopulate) then
           write(0,*) 'eval0: error --- point not excised'
           return
        end if

        sum=0
        f1=0
        f2=0
        f3=0
        frac=(k+1.0d0)/k
        if (j.gt.(k+1).and.(chr(i,j-1).ne.CHR_excised.and.
     &                      chr(i,j-1).ne.CHR_repopulate)) then
           sum=sum+1
           f1=frac*f(i,j-1)+(1-frac)*f(i,j-1-k)
        else if (j.le.(nz-k-1).and.(chr(i,j+1).ne.CHR_excised.and.
     &                              chr(i,j+1).ne.CHR_repopulate)) then
           sum=sum+1
           f1=frac*f(i,j+1)+(1-frac)*f(i,j+1+k)
        end if
        if (i.le.(nrho-k-1).and.(chr(i+1,j).ne.CHR_excised.and.
     &                           chr(i+1,j).ne.CHR_repopulate)) then
           sum=sum+1
           f2=frac*f(i+1,j)+(1-frac)*f(i+1+k,j)
        end if
        if (j.gt.(k+1).and.i.le.(nrho-k-1).and.
     &      (chr(i+1,j-1).ne.CHR_excised.and.
     &       chr(i+1,j-1).ne.CHR_repopulate)) then
           sum=sum+1
           f3=frac*f(i+1,j-1)+(1-frac)*f(i+1+k,j-1-k)
        else if (j.le.(nz-k-1).and.i.le.(nrho-k-1).and.
     &           (chr(i+1,j+1).ne.CHR_excised.and.
     &            chr(i+1,j+1).ne.CHR_repopulate)) then
           sum=sum+1
           f3=frac*f(i+1,j+1)+(1-frac)*f(i+1+k,j+1+k)
        end if

        if (sum.eq.0) then
           write(0,*) 'eval0: error --- sum=0. i,j=',i,j
           return
        end if

        eval0=(f1+f2+f3)/sum
 
        return
        end       

c======================================================================
c experimental extend for odd-functions
c======================================================================
        subroutine extend_odd(f,k,chr,first_n,axis,rho,z,nrho,nz)
        implicit none
        integer nrho,nz,k,first_n,axis
        real*8 f(nrho,nz),chr(nrho,nz)
        real*8 rho(nrho),z(nz)
 
        include 'chr.inc'

        real*8 f0,i0
        integer i,j

        if (rho(1).ne.0.0d0) then
           write(*,*) 'extend_odd: error ... rho(1) must = 0'
           return
        end if

        do j=1,nz
           chr(nrho,j)=CHR_interior
           i0=0
           do i=nrho-1,2,-1
              if (i0.eq.0.and.(chr(i,j).eq.CHR_excised.or.
     &                         chr(i,j).eq.CHR_repopulate)) then
                 i0=i+1
                 f0=f(i+1,j)
              end if
              if (chr(i,j).eq.CHR_excised.or.
     &            chr(i,j).eq.CHR_repopulate) then
                 f(i,j)=f0*(i-1.0d0)/(i0-1.0d0)
              end if
              chr(i,j)=CHR_interior
           end do
           f(1,j)=0
           chr(1,j)=CHR_interior
        end do

        return
        end
 
c======================================================================
c Fills in the excised region of a function using eval() above
c (see eval() for definition of k)
c
c eps_delta is only applied *after* first_n iterations
c (set to zero when smoothing, and say 2 when repopulating)
c
c NOTE: chr is DESTROYED in the process!
c if axis=0, assumes function is odd
c======================================================================
        subroutine extend(f,k,chr,first_n,axis,rho,z,nrho,nz)
        implicit none
        integer nrho,nz,k,first_n,axis
        real*8 f(nrho,nz),chr(nrho,nz)
        real*8 rho(nrho),z(nz)
 
        include 'chr.inc'
 
        integer i,j,iter,max_i,max_j,min_j,avg_width,i2,j2
        logical is_excised
        real*8 eval,avg,sum,max_delta,eps_delta,delta,ceps_delta,eval0
        parameter (max_delta=0.0d0,eps_delta=0.25d0,avg_width=8)
        logical use_odd_extend
        parameter (use_odd_extend=.true.)
 
        integer CHR_excised2
        parameter (CHR_excised2=3623)

        if (use_odd_extend.and.axis.eq.0) then
           call extend_odd(f,k,chr,first_n,axis,rho,z,nrho,nz)
           return
        end if
 
        sum=1.0d-10
        avg=1.0d-20
        max_i=1
        max_j=1
        min_j=nz
        do i=1,nrho
           do j=1,nz
              if (chr(i,j).eq.CHR_repopulate.or.
     &            chr(i,j).eq.CHR_excised) chr(i,j)=CHR_excised2
              if (chr(i,j).ne.CHR_excised2) then
                 chr(i,j)=CHR_interior
              else
                 max_i=max(min(nrho-2,i+2),max_i)
                 max_j=max(min(nz-2,j+2),max_j)
                 min_j=min(max(2,j-2),min_j)
                 f(i,j)=0.0d0
              end if
           end do
        end do
 
        !---------------------------------------------------------------
        ! First calculate the local average along the excision
        ! boundary, extending it throughout the interior. To avoid
        ! an extra temporary variable, we store the average in the
        ! excised part of f (we don't want a 'moving average' that
        ! gets updates as we extrapolate --- that would defeat
        ! the purpose of the average)
        !---------------------------------------------------------------
        is_excised=.true.
        iter=0
        do while(is_excised)
           is_excised=.false.
           do j=min_j+iter,max_j-iter
              do i=1,max_i-iter
                 if (chr(i,j).eq.CHR_excised2.and.(
     &               chr(i,j+1).ne.CHR_excised2.or.
     &               chr(i,j-1).ne.CHR_excised2.or.                      
     &               chr(i+1,j).ne.CHR_excised2.or.
     &               chr(i+1,j+1).ne.CHR_excised2.or.
     &               chr(i+1,j-1).ne.CHR_excised2)) then
                    sum=0
                    avg=0
                    do i2=max(1,i-avg_width),min(nrho-1,i+avg_width)
                       do j2=max(2,j-avg_width),min(nz-1,j+avg_width)
                          if (chr(i2,j2).ne.CHR_excised2.and.(
     &                        chr(i2,j2+1).eq.CHR_excised2.or.
     &                        chr(i2,j2-1).eq.CHR_excised2.or.
     &                        chr(i2+1,j2).eq.CHR_excised2.or.
     &                        chr(max(1,i2-1),j2).eq.CHR_excised2)) then
                             if (axis.eq.0.and.(i2.le.(avg_width-i+1)))
     &                           then
                                sum=sum+2
                             else
                                avg=avg+f(i2,j2)
                                sum=sum+1
                             end if
                          end if
                       end do
                    end do
                    if (sum.gt.0) then
                       f(i,j)=avg/sum
                       if (f(i,j).eq.0.0d0) f(i,j)=1.0d-30
                    else
                       write(*,*) 'extend: ERROR .... sum=0!'
                       stop
                    end if
                 else if (chr(i,j).eq.CHR_excised2) then
                    is_excised=.true.
                 end if
              end do
           end do
           do j=min_j+iter,max_j-iter
              do i=1,max_i-iter
                 if (chr(i,j).eq.CHR_excised2.and.f(i,j).ne.0.0d0)
     &              chr(i,j)=CHR_excised
              end do
           end do
           iter=iter+1
        end do
 
        !---------------------------------------------------------------
        ! now extrapolate
        !---------------------------------------------------------------
        is_excised=.true.
        iter=0
        do while(is_excised)
           is_excised=.false.
           do j=min_j+iter,max_j-iter
              do i=1,max_i-iter
                 if (chr(i,j).eq.CHR_excised.and.(
     &               chr(i,j+1).eq.CHR_interior.or.
     &               chr(i,j-1).eq.CHR_interior.or.
     &               chr(i+1,j).eq.CHR_interior
     &              .or.chr(i+1,j+1).eq.CHR_interior.or.
     &               chr(i+1,j-1).eq.CHR_interior
     &               )) then
                    chr(i,j)=CHR_repopulate
                 else if (chr(i,j).eq.CHR_excised) then
                    is_excised=.true.                                       
                 end if
              end do
           end do
           do j=min_j+iter,max_j-iter
              do i=1,max_i-iter
                 if (chr(i,j).eq.CHR_repopulate) then
                    avg=f(i,j)
!                    f(i,j)=eval(f,rho(i),z(j),1,k,0.0d0,chr,
!     &                          rho,z,nrho,nz)
                    f(i,j)=eval0(f,i,j,k,chr,Nrho,Nz)
                    if (f(i,j).ne.0.0d0) then
                       delta=abs((f(i,j)-avg)/avg)
                       if (iter.gt.first_n) then
                          ceps_delta=min(1.0d0,eps_delta*
     &                      (1.0d0+(iter-first_n)/5))
                       else
                          ceps_delta=0.0d0
                       end if
                       if (delta.gt.max_delta) then
                          f(i,j)=f(i,j)-ceps_delta*(f(i,j)-avg)
                       end if
                    end if
                 end if
              end do
           end do
           do j=min_j+iter,max_j-iter
              do i=1,max_i-iter
                 if (chr(i,j).eq.CHR_repopulate) chr(i,j)=CHR_interior
              end do
           end do
           iter=iter+1
        end do
 
        return
        end
 
c======================================================================
c apply KO filter in the rho-direction in a region k*3 points from
c the axis of f (fn-new, fo-old)
c
c if axis = 0 zero f(z=0)
c         = 1 zero df_drho(z=0)
c         = 2 just z-dissipate f(z=0)
c
c k=1,2,... multiplies the characteristic wavelength targeted by the
c filter
c
c NOTE: eps is an array!
c======================================================================
        subroutine diss_ko_axis(fn,fo,eps,k,axis,nrho,nz)
        implicit none
        integer nrho,nz,axis,k
        real*8 fn(nrho,nz),fo(nrho,nz),eps(nrho,nz)
 
        integer i,j,is,imk,im2k
        real*8 simk,sim2k
 
        do i=1,nrho
           do j=1,nz
              fn(i,j)=fo(i,j)
           end do
        end do                                                             
 
        if (axis.eq.2) then
           is=2*k+1
        else
           is=1
        end if
 
        do i=is,min(3*k,nrho-2*k)
           do j=3,nz-3
              imk=i-k
              simk=1
              if (imk.lt.1) then
                 imk=2-imk
                 !--------------------------------------------------
                 ! sim.k=-1 : applying filter assuming f is odd in rho
                 ! sim.k=1  : .. even
                 !--------------------------------------------------
                 if (axis.eq.0) simk=-1
              end if
              im2k=i-2*k
              sim2k=1
              if (im2k.lt.1) then
                 im2k=2-im2k
                 if (axis.eq.0) sim2k=-1
              end if
              fn(i,j)=fn(i,j)-(eps(i,j)/16)*(
     &                fo(i+2*k,j)-4*fo(i+1*k,j)+6*fo(i  ,j)
     &                -4*simk*fo(imk,j)+sim2k*fo(im2k,j))
           end do
        end do
 
        i=1
        if (axis.eq.0) then
           do j=2,nz-1
              fn(i,j)=0
           end do
        else if (axis.eq.1) then
           do j=2,nz-1
              fn(i,j)=(4*fn(i+1,j)-fn(i+2,j))/3
           end do
        end if
 
        return
        end
 
c======================================================================
c apply KO filter to entire f (fn-new, fo-old)
c
c if axis = 0 zero f(z=0)
c         = 1 zero df_drho(z=0)
c         = 2 just z-dissipate f(z=0)
c
c k=1,2,... multiplies the characteristic wavelength targeted by the
c filter
c
c NOTE: eps is an array!
c======================================================================
        subroutine diss_ko(fn,fo,eps,k,axis,nrho,nz)
        implicit none
        integer nrho,nz,axis,k
        real*8 fn(nrho,nz),fo(nrho,nz),eps(nrho,nz)
 
        integer i,j,is,imk,im2k                                            
        real*8 simk,sim2k
 
        do i=1,nrho
           do j=1,nz
              fn(i,j)=fo(i,j)
           end do
        end do
 
        if (axis.eq.2) then
           is=2*k+1
        else
           is=1
        end if
 
        do i=is,nrho-2*k
           do j=1+2*k,nz-2*k
              imk=i-k
              simk=1
              if (imk.lt.1) then
                 imk=2-imk
                 !--------------------------------------------------
                 ! sim.k=-1 : applying filter assuming f is odd in rho
                 ! sim.k=1  : .. even
                 !--------------------------------------------------
                 if (axis.eq.0) simk=-1
              end if
              im2k=i-2*k
              sim2k=1
              if (im2k.lt.1) then
                 im2k=2-im2k
                 if (axis.eq.0) sim2k=-1
              end if
              fn(i,j)=fn(i,j)-(eps(i,j)/16)*(
     &                fo(i+2*k,j)-4*fo(i+1*k,j)+6*fo(i  ,j)
     &                -4*simk*fo(imk,j)+sim2k*fo(im2k,j)
     &               +fo(i,j+2*k)-4*fo(i,j+1*k)+6*fo(i,j)
     &                -4*fo(i,j-1*k)+fo(i,j-2*k))
           end do
        end do
 
        i=1
        if (axis.eq.2) then
           do i=1,2*k
              do j=1+2*k,nz-2*k
                 fn(i,j)=fn(i,j)-(eps(i,j)/16)*(
     &                +fo(i,j+2*k)-4*fo(i,j+1*k)+6*fo(i,j)
     &                -4*fo(i,j-1*k)+fo(i,j-2*k))
              end do
           end do
        else if (axis.eq.0) then
           do j=2,nz-1
              fn(i,j)=0
           end do
        else if (axis.eq.1) then
           do j=2,nz-1
              fn(i,j)=(4*fn(i+1,j)-fn(i+2,j))/3
           end do
        end if
 
        return
        end
                                                                           
c======================================================================
c makes an eps() array:
c
c flag=0 : eps(i,j)=eps_max 
c flag=1 : eps(i,j)=eps_max where excised
c
c======================================================================
        subroutine fill_eps(eps,eps_max,flag,chr,Nrho,Nz)
        implicit none
        integer Nrho,Nz,flag
        real*8 eps(Nrho,Nz),chr(Nrho,Nz),eps_max

        include 'chr.inc'
 
        integer i,j

        if (flag.eq.0) then
           do i=1,Nrho
              do j=1,Nz
                 eps(i,j)=eps_max
              end do
           end do
        else if (flag.eq.1) then
           do i=1,Nrho
              do j=1,Nz
                 if (chr(i,j).eq.CHR_excised) then 
                    eps(i,j)=eps_max
                 else 
                    eps(i,j)=0.0d0
                 end if
              end do
           end do
        end if

        return
        end
              
c======================================================================
c a couple of functions used by apply_dis_ex_bdy below
c
c i,j is assumed to be in the range (1..Nrho-2),(3,Nz-2)
c======================================================================
        real*8 function get_avg_delta(f,chr,i,j,Nrho,Nz)
        implicit none
        integer Nrho,Nz,i,j
        real*8 f(Nrho,Nz),chr(Nrho,Nz)
 
        include 'chr.inc'
 
        real*8 sumz,avgz,sumrho,avgrho,avg
        integer i2,j2
        integer width
        parameter (width=4)
 
        !-----------------------------------------------
        ! measure average undivided gradients of f in a
        ! small neigborhood of (i,j)
        !-----------------------------------------------
        avgz=0
        sumz=1.0d-10
        avgrho=0
        sumrho=1.0d-10
        do i2=max(1,i-width),min(Nrho,i+width)
           do j2=max(1,j-width),min(Nz,j+width)
              if (i2.lt.(i+width).and.
     &            chr(i2,j2).ne.CHR_excised) then
                 avgrho=avgrho+(f(i2+1,j2)-f(i2,j2))
                 sumrho=sumrho+1
              end if
              if (j2.lt.(j+width).and.
     &            chr(i2,j2).ne.CHR_excised.and.
     &            chr(i2,j2+1).ne.CHR_excised) then
                 avgz=avgz+(f(i2,j2+1)-f(i2,j2))
                 sumz=sumz+1
              end if
           end do
        end do
        avg=abs(avgz/sumz)+abs(avgrho/sumrho)
 
        get_avg_delta=avg
 
        return
        end
 
c======================================================================
c apply_dis_ex_bdy ---
c
c Applies a Kreiss-Oliger style dissipation filter along the excision
c boundary to f, directed at high frequency components tangent to the
c boundary
c
c OR some form of smoothing
c
c chr2 and fn are work arrays.
c======================================================================
        subroutine apply_dis_ex_bdy(f,chr,f_surf,chr2,fn,
     &                              eps,use_exact_tangent,rho,z,
     &                              Nrho,Nz)
        implicit none
        integer Nrho,Nz
        real*8 rho(Nrho),z(Nz)
        real*8 f(Nrho,Nz),chr(Nrho,Nz),f_surf(Nrho,Nz)
        real*8 fn(Nrho,Nz),chr2(Nrho,Nz)
        real*8 eps
        integer use_exact_tangent
        character *64 name

        include 'chr.inc'

        real*8 ct,st,fp1,fp2,fm1,fm2,rho0,z0,a,b,h,drho,dz,eval
        real*8 chr_dump(5,5)
        integer i,j,i2,j2,n

        real*8 cpi
        parameter (cpi=3.14159265359d0)
        logical ltrace,cvtrace,ltrace2
        parameter (ltrace=.false.,cvtrace=.false.,ltrace2=.false.)
        logical smooth,use_line,second_line
        parameter (smooth=.true.,use_line=.false.,second_line=.false.)
        integer smooth_length,num
        parameter (smooth_length=1)
        real*8 sum,wght,x,avg_delta,y
        !--------------------------------------------------------------
        ! wght_mthd for smoothing:
        ! 1 -- uniform
        ! 2 -- 'threshold activated f-delta'
        !--------------------------------------------------------------
        integer wght_mthd
        parameter (wght_mthd=2)
        real*8 eps2,max_df_surf,df_thresh,get_avg_delta,avg,rel_df
        parameter (eps2=0.5d0,df_thresh=1.5d0)

        a=1
        b=1

        drho=rho(2)-rho(1)
        dz=z(2)-z(1)

        !--------------------------------------------------------------
        ! only apply dissipation away from physical boundaries
        ! along the excision boundary and where we might not
        ! do so in the RNPL
        ! Also, to avoid unforseen effects, we do not alter
        ! the original function f during the operation.
        !--------------------------------------------------------------
        do i=2,Nrho-2
           do j=3,Nz-2
              if ((chr(i,j).eq.CHR_ex_bdy).or.
     &            ((chr(i,j).ne.CHR_excised).and.
     &             ((chr(i-1,j).eq.CHR_ex_bdy).or.
     &              (chr(i,j-1).eq.CHR_ex_bdy).or.
     &              (chr(i,j+1).eq.CHR_ex_bdy)))) then
                 if (smooth.or.i.eq.2) then
                    if (wght_mthd.eq.2) then
                       avg_delta=get_avg_delta(f,chr,i,j,Nrho,Nz)
                       max_df_surf=0
                       do i2=max(1,i-2),i+2
                          do j2=j-2,j+2
                            if (chr(i2,j2).eq.CHR_excised)
     &                          max_df_surf=max(max_df_surf,
     &                         abs(f_surf(i2,j2)-f_surf(i,j)))
                          end do
                       end do
                       sum=0
                       avg=0
                       !-----------------------------------------------
                       ! weight via distance from point, as 
                       ! measured by f_surf; and note slightly larger
                       ! region overwhich we sum than with wght_mthd=1
                       !-----------------------------------------------
                       do i2=max(1,i-2),i+2
                          do j2=j-2,j+2
                             if (chr(i2,j2).ne.CHR_excised) then
                                wght=max_df_surf-
     &                             abs(f_surf(i2,j2)-f_surf(i,j))
                                avg=avg+wght*f(i2,j2)
                                sum=sum+wght
                             end if
                          end do
                       end do
                       avg=avg/sum
                       fn(i,j)=f(i,j)
                       if (avg_delta.gt.0.0d0) then
                          rel_df=(abs(f(i,j)-avg))/avg_delta
                          if (rel_df.gt.df_thresh)
     &                       fn(i,j)=f(i,j)-eps2*(f(i,j)-avg)
                       end if
                    else         
                       sum=0
                       fn(i,j)=0
                       do i2=i-1,i+1
                          do j2=j-1,j+1
                             if (chr(i2,j2).ne.CHR_excised) then 
                                wght=1
                                sum=sum+wght
                                fn(i,j)=fn(i,j)+wght*f(i2,j2)
                             end if
                          end do
                       end do
                       fn(i,j)=fn(i,j)/sum
                    end if
                 else ! KO-style dissipation

                 end if
              else
                 fn(i,j)=f(i,j)
              end if
           end do
        end do

        do i=2,Nrho-2
           do j=3,Nz-2
              f(i,j)=fn(i,j)
           end do
        end do

        ! for DV version --- QFIT
 
        i=1
        do j=3,Nz-2
           f(i,j)=(1.0d0/3.0d0)*(4.0d0*f(i+1,j)-f(i+2,j))
        end do 

        return
        end

c======================================================================
c a couple of routines from dmatlib_ex
c======================================================================

c-----------------------------------------------------------------------
c
c modified version of dvi4q1 from Matt's dveclib ... added 'skips'
c s1,s2 so that we can interpolate along rows of a 2d matrix
c (see dmi4q1) ... set s1=s2=1 to recover original version
c Also, if use_chr=1, then the fine grid is only updated
c when chr()=chr_val
c
c     Returns IRHO : 1 cubically interpolated refinement of V1 in V2.
c
c-----------------------------------------------------------------------
c
      subroutine dvi4q1_s(v1,v2,n1,irho,s1,s2,chr,chr_val,use_chr)
c
         implicit     logical*1 (a-z)
c
         integer      irho, n1, s1, s2
         real*8       v1(n1*s1),    v2(1), chr(1), chr_val
c
         real*8       c0, c1, c2, c3, rhom1, sigma
         integer      i, j, use_chr
c
         real*8       half,     mhalf,     sixth,     msixth
         parameter  ( half   =  0.5000 0000 0000 0000 d0,
     *                mhalf  = -0.5000 0000 0000 0000 d0,
     *                sixth  =  0.1666 6666 6666 6667 d0,
     *                msixth = -0.1666 6666 6666 6667 d0  )
c
         rhom1 = 1.0d0 / irho
c
         do 10 i = 1 , n1
            if (use_chr.eq.0.or.(chr(irho*(i-1)*s2+1).eq.chr_val))
     *      v2(irho*(i-1)*s2+1)=v1((i-1)*s1+1)
 10      continue
c
         do 20 j = 1 , irho - 1
            sigma = rhom1 * j
            c0 = msixth * (sigma - 1.0d0) * (sigma - 2.0d0) *
     *                    (sigma - 3.0d0)
            c1 = half   *  sigma          * (sigma - 2.0d0) *
     *                    (sigma - 3.0d0)
            c2 = mhalf  *  sigma          * (sigma - 1.0d0) *
     *                    (sigma - 3.0d0)
            c3 = sixth  *  sigma          * (sigma - 1.0d0) *
     *                    (sigma - 2.0d0)
            if (use_chr.eq.0.or.(chr(j*s2+1).eq.chr_val))
     *      v2(j*s2+1)=c0*v1(1)+c1*v1(s1+1)+c2*v1(2*s1+1)+c3*v1(3*s1+1)
 20      continue
c
         do 50 j = 1 , irho - 1
            sigma = 1.0d0 + rhom1 * j
            c0 = msixth * (sigma - 1.0d0) * (sigma - 2.0d0) *
     *                    (sigma - 3.0d0)
            c1 = half   *  sigma          * (sigma - 2.0d0) *
     *                    (sigma - 3.0d0)
            c2 = mhalf  *  sigma          * (sigma - 1.0d0) *
     *                    (sigma - 3.0d0)
            c3 = sixth  *  sigma          * (sigma - 1.0d0) *
     *                    (sigma - 2.0d0)
            do 30 i = 1 , n1 - 3
               if (use_chr.eq.0.or.(chr((irho*i+j)*s2+1).eq.chr_val))
     *         v2((irho*i+j)*s2+1)=c0*v1((i-1)*s1+1)+c1*v1(i*s1+1)+
     *                             c2*v1((i+1)*s1+1)+c3*v1((i+2)*s1+1)
 30         continue
 50      continue
c

         do 60 j = 1 , irho - 1
            sigma = 2.0d0 + rhom1 * j
            c0 = msixth * (sigma - 1.0d0) * (sigma - 2.0d0) *
     *                    (sigma - 3.0d0)
            c1 = half   *  sigma          * (sigma - 2.0d0) *
     *                    (sigma - 3.0d0)
            c2 = mhalf  *  sigma          * (sigma - 1.0d0) *
     *                    (sigma - 3.0d0)
            c3 = sixth  *  sigma          * (sigma - 1.0d0) *
     *                    (sigma - 2.0d0)
            if (use_chr.eq.0.or.(chr((irho*(n1-2)+j)*s2+1).eq.chr_val))
     *      v2((irho*(n1-2)+j)*s2+1)=
     *          c0*v1((n1-4)*s1+1)+c1*v1((n1-3)*s1+1)+
     *          c2*v1((n1-2)*s1+1)+c3*v1((n1-1)*s1+1)
 60      continue
c
         return
c
      end

c---------------------------------------------------------------------------
c 2D version of dvi4q1_s ... calls dvi4q1_s along first the
c rows then columns of the fine grid to achieve a 2D interpolation
c
c uf - fine grid, dimension nxf*nyf
c uc - coarse grid, dimension ncf*ncf
c
c (if_co,jf_co) is the position of the fine grid within the coarse
c grid.
c
c irho:1 is fine to coarse ratio
c
c---------------------------------------------------------------------------
        subroutine dmi4q1(uf,uc,nxc,nyc,nxf,nyf,if_co,jf_co,irho)
        implicit none
        integer nxc,nyc,nxf,nyf,if_co,jf_co,irho
        real*8 uf(nxf,nyf),uc(nxc,nyc)

        integer ic,jc,if,jf,ics,ice,jcs,jce,ifs,jfs,ife,jfe

        logical ltrace
        parameter(ltrace=.false.)

        if (if_co.gt.nxc.or.jf_co.gt.nyc) return
        if (if_co.lt.1) then
           ics=1
           ifs=(1-if_co)*irho+1
        else
           ics=if_co
           ifs=1
        end if
        ice=min(nxc,if_co+(nxf-1)/irho)
        ife=ifs+(ice-ics)*irho
        if (ice.lt.1) return
        if (jf_co.lt.1) then
           jcs=1
           jfs=(1-jf_co)*irho+1
        else
           jcs=jf_co
           jfs=1
        end if
        jce=min(nyc,jf_co+(nyf-1)/irho)
        jfe=jfs+(jce-jcs)*irho
        if (jce.lt.1) return

        if (ltrace) then 
           write(*,*) 'dmi4q1:'
           write(*,*) 'nxc,nyc,nxf,nyf:',nxc,nyc,nxf,nyf
           write(*,*) 'ics,ice,jcs,jce:',ics,ice,jcs,jce
           write(*,*) 'ifs,ife,jfs,jfe:',ifs,ife,jfs,jfe
        end if

        !---------------------------------------------------------------
        ! first fill in the fine grids rows where they line up with
        ! coarse grid rows
        !---------------------------------------------------------------
        ic=ics
        do if=ifs,ife,irho
           call dvi4q1_s(uc(ic,jcs),uf(if,jfs),jce-jcs+1,irho,nxc,nxf,
     &                   uf,0.0d0,0)
           ic=ic+1
        end do
        !---------------------------------------------------------------
        ! now fill in the colums of the fine grid. 
        !---------------------------------------------------------------
        do jf=jfs,jfe
           call dvi4q1_s(uf(ifs,jf),uf(ifs,jf),ice-ics+1,irho,irho,1,
     &                   uf,0.0d0,0)
        end do

        return
        end

c======================================================================
c PAMR dissipation filter
c======================================================================

c---------------------------------------------------------------------------
c applies a simple KO filter to f, where mask != mask_off, and modified
c near excision (all, if do_bdy=1) boundaries as follows:
c
c with undivided operators Up(f) = f(i+1)-f(i)
c                          Um(f) = f(i)-f(i-1)
c
c Interior: eps*w4*(Up Um)^2
c left+1  : eps*w3*(Up Up Um)
c left    : eps*w2*(Up Um)
c right-1 : eps*w3*(Um Um Up)
c right   : eps*w2*(Up Um)
c
c NOTE: update DV version too
c---------------------------------------------------------------------------
        subroutine dmdiss3d_ex(f,work,eps,do_bdy,phys_bdy_type,even,odd,
     &                         mask,mask_off,nx,ny,
     &                         nz,chr,ex,ind_sweeps)
        implicit none
        integer nx,ny,nz,do_bdy,phys_bdy_type(6),even,odd
        real*8 f(nx,ny,nz),work(nx,ny,nz),mask(nx,ny,nz),eps,mask_off
        real*8 chr(nx,ny,nz),ex
        logical ind_sweeps

        integer i,j,k,bo1,bo2
        real*8 eps_eff,f_hf,norm_f
        real*8 w4,w3,w2
c        parameter (w4=1.0d0/16.0d0,w3=1.0d0/16.0d0,w2=1.0d0/16.0d0)
        parameter (w4=1.0d0/16.0d0,w3=1.0d0/8.0d0,w2=1.0d0/4.0d0)
        integer pass,npass

        eps_eff=eps

        if (do_bdy.eq.0) then
           bo1=0
           bo2=0
        else
           bo1=1
           bo2=2
        end if

        npass=1
        if (ny.gt.1.and.ind_sweeps) npass=2
        if (nz.gt.1.and.ind_sweeps) npass=3

        do pass=1,npass

         do i=1,nx
           do j=1,ny
              do k=1,nz
                 work(i,j,k)=f(i,j,k)
              end do
           end do
         end do

         do i=1,nx
           do j=1,ny
              do k=1,nz
                 if (mask(i,j,k).ne.mask_off.and.
     &               (chr(i,j,k).ne.ex)) then

                   if (.not.ind_sweeps.or.pass.eq.1) then
                    f_hf=0
                    if (i.gt.2.and.i.lt.(nx-1).and.
     &                  ((chr(i-2,j,k).ne.ex.and.chr(i-1,j,k).ne.ex.and.
     &                    chr(i+2,j,k).ne.ex.and.chr(i+1,j,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                     work(i-2,j,k)+work(i+2,j,k)
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                    else if (i.eq.2.and.phys_bdy_type(1).eq.odd.and.
     &                  ((chr(i-1,j,k).ne.ex.and.
     &                    chr(i+2,j,k).ne.ex.and.chr(i+1,j,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (-work(i,j,k))+work(i+2,j,k)
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                    else if (i.eq.1.and.phys_bdy_type(1).eq.odd.and.
     &                 ((chr(i+2,j,k).ne.ex.and.chr(i+1,j,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (-work(i+2,j,k))+work(i+2,j,k)
     &                -4*((-work(i+1,j,k))+work(i+1,j,k))+6*work(i,j,k))
                    else if (i.eq.Nx-1.and.phys_bdy_type(2).eq.odd.and.
     &                  ((chr(i+1,j,k).ne.ex.and.
     &                    chr(i-2,j,k).ne.ex.and.chr(i-1,j,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                     work(i-2,j,k)+(-work(i,j,k))
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                    else if (i.eq.Nx.and.phys_bdy_type(2).eq.odd.and.
     &                 ((chr(i-2,j,k).ne.ex.and.chr(i-1,j,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    work(i-2,j,k)+(-work(i-2,j,k))
     &                -4*(work(i-1,j,k)+(-work(i-1,j,k)))+6*work(i,j,k))
                    else if (i.eq.2.and.phys_bdy_type(1).eq.even.and.
     &                  ((chr(i-1,j,k).ne.ex.and.
     &                    chr(i+2,j,k).ne.ex.and.chr(i+1,j,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (work(i,j,k))+work(i+2,j,k)
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                    else if (i.eq.1.and.phys_bdy_type(1).eq.even.and.
     &                 ((chr(i+2,j,k).ne.ex.and.chr(i+1,j,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (work(i+2,j,k))+work(i+2,j,k)
     &                -4*((work(i+1,j,k))+work(i+1,j,k))+6*work(i,j,k))
                    else if (i.eq.Nx-1.and.phys_bdy_type(2).eq.even.and.
     &                  ((chr(i+1,j,k).ne.ex.and.
     &                    chr(i-2,j,k).ne.ex.and.chr(i-1,j,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                     work(i-2,j,k)+(work(i,j,k))
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                    else if (i.eq.Nx.and.phys_bdy_type(2).eq.even.and.
     &                 ((chr(i-2,j,k).ne.ex.and.chr(i-1,j,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    work(i-2,j,k)+(work(i-2,j,k))
     &                -4*(work(i-1,j,k)+(work(i-1,j,k)))+6*work(i,j,k))
                    else if (i.gt.(2-bo1).and.i.lt.(nx-1).and.
     &                  (chr(i-1,j,k).ne.ex.and.chr(i+1,j,k).ne.ex.and.
     &                   chr(i+2,j,k).ne.ex)) 
     &              then
                       f_hf=w3*(
     &                    -work(i-1,j,k)+3*work(i,j,k)
     &                  -3*work(i+1,j,k)+work(i+2,j,k))
                    else if (i.gt.2.and.i.lt.(nx-1+bo1).and.
     &                  (chr(i-1,j,k).ne.ex.and.chr(i+1,j,k).ne.ex.and.
     &                   chr(i-2,j,k).ne.ex) ) 
     &              then
                       f_hf=w3*(
     &                    -work(i+1,j,k)+3*work(i,j,k)
     &                  -3*work(i-1,j,k)+work(i-2,j,k))
                    else if ((i.gt.(2-bo2).or.
     &                       (i.eq.2.and.chr(1,j,k).eq.ex))
     &                       .and.i.lt.(nx-1).and.
     &                  (chr(i+1,j,k).ne.ex.and.chr(i+2,j,k).ne.ex) )
     &              then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i+1,j,k)+work(i+2,j,k))
                    else if (i.gt.2.and.(i.lt.(nx-1+bo2).or.
     &                       (i.eq.(nx-1).and.chr(nx,j,k).eq.ex)).and.
     &                  (chr(i-1,j,k).ne.ex.and.chr(i-2,j,k).ne.ex) )
     &              then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i-1,j,k)+work(i-2,j,k))
                    end if

                    f(i,j,k)=f(i,j,k)-eps_eff*f_hf
                   end if

                   if (.not.ind_sweeps.or.pass.eq.2) then
                    f_hf=0
                    if (j.gt.2.and.j.lt.(ny-1).and.
     &                  ((chr(i,j-2,k).ne.ex.and.chr(i,j-1,k).ne.ex.and.
     &                    chr(i,j+2,k).ne.ex.and.chr(i,j+1,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                     work(i,j-2,k)+work(i,j+2,k)
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                    else if (j.eq.2.and.phys_bdy_type(3).eq.odd.and.
     &                  ((chr(i,j-1,k).ne.ex.and.
     &                    chr(i,j+2,k).ne.ex.and.chr(i,j+1,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (-work(i,j,k))+work(i,j+2,k)
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                    else if (j.eq.1.and.phys_bdy_type(3).eq.odd.and.
     &                  ((chr(i,j+2,k).ne.ex.and.chr(i,j+1,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (-work(i,j+2,k))+work(i,j+2,k)
     &                -4*((-work(i,j+1,k))+work(i,j+1,k))+6*work(i,j,k))
                    else if (j.eq.Ny-1.and.phys_bdy_type(4).eq.odd.and.
     &                  ((chr(i,j+1,k).ne.ex.and.
     &                    chr(i,j-2,k).ne.ex.and.chr(i,j-1,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                     work(i,j-2,k)+(-work(i,j,k))
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                    else if (j.eq.Ny.and.phys_bdy_type(4).eq.odd.and.
     &                  ((chr(i,j-2,k).ne.ex.and.chr(i,j-1,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    work(i,j-2,k)+(-work(i,j-2,k))
     &                -4*(work(i,j-1,k)+(-work(i,j-1,k)))+6*work(i,j,k))
                    else if (j.eq.2.and.phys_bdy_type(3).eq.even.and.
     &                  ((chr(i,j-1,k).ne.ex.and.
     &                    chr(i,j+2,k).ne.ex.and.chr(i,j+1,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (work(i,j,k))+work(i,j+2,k)
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                    else if (j.eq.1.and.phys_bdy_type(3).eq.even.and.
     &                  ((chr(i,j+2,k).ne.ex.and.chr(i,j+1,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (work(i,j+2,k))+work(i,j+2,k)
     &                -4*((work(i,j+1,k))+work(i,j+1,k))+6*work(i,j,k))
                    else if (j.eq.Ny-1.and.phys_bdy_type(4).eq.even.and.
     &                  ((chr(i,j+1,k).ne.ex.and.
     &                    chr(i,j-2,k).ne.ex.and.chr(i,j-1,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                     work(i,j-2,k)+(work(i,j,k))
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                    else if (j.eq.Ny.and.phys_bdy_type(4).eq.even.and.
     &                  ((chr(i,j-2,k).ne.ex.and.chr(i,j-1,k).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    work(i,j-2,k)+(work(i,j-2,k))
     &                -4*(work(i,j-1,k)+(work(i,j-1,k)))+6*work(i,j,k))
                    else if (j.gt.(2-bo1).and.j.lt.(ny-1).and.
     &                  (chr(i,j-1,k).ne.ex.and.chr(i,j+1,k).ne.ex.and.
     &                   chr(i,j+2,k).ne.ex)) 
     &              then
                       f_hf=w3*(
     &                    -work(i,j-1,k)+3*work(i,j,k)
     &                  -3*work(i,j+1,k)+work(i,j+2,k))
                    else if (j.gt.2.and.j.lt.(ny-1+bo1).and.
     &                  (chr(i,j-1,k).ne.ex.and.chr(i,j+1,k).ne.ex.and.
     &                   chr(i,j-2,k).ne.ex) ) 
     &              then
                       f_hf=w3*(
     &                    -work(i,j+1,k)+3*work(i,j,k)
     &                  -3*work(i,j-1,k)+work(i,j-2,k))
                    else if ((j.gt.(2-bo2).or.
     &                       (j.eq.2.and.chr(i,1,k).eq.ex))
     &                       .and.j.lt.(ny-1).and.
     &                  (chr(i,j+1,k).ne.ex.and.chr(i,j+2,k).ne.ex) )
     &              then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i,j+1,k)+work(i,j+2,k))
                    else if (j.gt.2.and.(j.lt.(ny-1+bo2).or.
     &                       (j.eq.(ny-1).and.chr(i,ny,k).eq.ex)).and.
     &                  (chr(i,j-1,k).ne.ex.and.chr(i,j-2,k).ne.ex) )
     &              then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i,j-1,k)+work(i,j-2,k))
                    end if

                    f(i,j,k)=f(i,j,k)-eps_eff*f_hf
                   end if

                   if (.not.ind_sweeps.or.pass.eq.3) then
                    f_hf=0
                    if (k.gt.2.and.k.lt.(nz-1).and.
     &                  ((chr(i,j,k-2).ne.ex.and.chr(i,j,k-1).ne.ex.and.
     &                    chr(i,j,k+2).ne.ex.and.chr(i,j,k+1).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                     work(i,j,k-2)+work(i,j,k+2)
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                    else if (k.eq.2.and.phys_bdy_type(5).eq.odd.and.
     &                  ((chr(i,j,k-1).ne.ex.and.
     &                    chr(i,j,k+2).ne.ex.and.chr(i,j,k+1).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (-work(i,j,k))+work(i,j,k+2)
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                    else if (k.eq.1.and.phys_bdy_type(5).eq.odd.and.
     &                  ((chr(i,j,k+2).ne.ex.and.chr(i,j,k+1).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (-work(i,j,k+2))+work(i,j,k+2)
     &                -4*((-work(i,j,k+1))+work(i,j,k+1))+6*work(i,j,k))
                    else if (k.eq.Nz-1.and.phys_bdy_type(6).eq.odd.and.
     &                  ((chr(i,j,k-2).ne.ex.and.chr(i,j,k-1).ne.ex.and.
     &                    chr(i,j,k+1).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                     work(i,j,k-2)+(-work(i,j,k))
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                    else if (k.eq.Nz.and.phys_bdy_type(6).eq.odd.and.
     &                  ((chr(i,j,k-2).ne.ex.and.chr(i,j,k-1).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    work(i,j,k-2)+(-work(i,j,k-2))
     &                -4*(work(i,j,k-1)+(-work(i,j,k-1)))+6*work(i,j,k))
                    else if (k.eq.2.and.phys_bdy_type(5).eq.even.and.
     &                  ((chr(i,j,k-1).ne.ex.and.
     &                    chr(i,j,k+2).ne.ex.and.chr(i,j,k+1).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (work(i,j,k))+work(i,j,k+2)
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                    else if (k.eq.1.and.phys_bdy_type(5).eq.even.and.
     &                  ((chr(i,j,k+2).ne.ex.and.chr(i,j,k+1).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    (work(i,j,k+2))+work(i,j,k+2)
     &                -4*((work(i,j,k+1))+work(i,j,k+1))+6*work(i,j,k))
                    else if (k.eq.Nz-1.and.phys_bdy_type(6).eq.even.and.
     &                  ((chr(i,j,k-2).ne.ex.and.chr(i,j,k-1).ne.ex.and.
     &                    chr(i,j,k+1).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                     work(i,j,k-2)+(work(i,j,k))
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                    else if (k.eq.Nz.and.phys_bdy_type(6).eq.even.and.
     &                  ((chr(i,j,k-2).ne.ex.and.chr(i,j,k-1).ne.ex)) )
     &              then
                       f_hf=w4*(
     &                    work(i,j,k-2)+(work(i,j,k-2))
     &                -4*(work(i,j,k-1)+(work(i,j,k-1)))+6*work(i,j,k))
                    else if (k.gt.(2-bo1).and.k.lt.(nz-1).and.
     &                  (chr(i,j,k-1).ne.ex.and.chr(i,j,k+1).ne.ex.and.
     &                   chr(i,j,k+2).ne.ex)) 
     &              then
                       f_hf=w3*(
     &                    -work(i,j,k-1)+3*work(i,j,k)
     &                  -3*work(i,j,k+1)+work(i,j,k+2))
                    else if (k.gt.2.and.k.lt.(nz-1+bo1).and.
     &                  (chr(i,j,k-1).ne.ex.and.chr(i,j,k+1).ne.ex.and.
     &                   chr(i,j,k-2).ne.ex) ) 
     &              then
                       f_hf=w3*(
     &                    -work(i,j,k+1)+3*work(i,j,k)
     &                  -3*work(i,j,k-1)+work(i,j,k-2))
                    else if ((k.gt.(2-bo2).or.
     &                       (k.eq.2.and.chr(i,j,1).eq.ex))
     &                       .and.k.lt.(nz-1).and.
     &                  (chr(i,j,k+1).ne.ex.and.chr(i,j,k+2).ne.ex) )
     &              then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i,j,k+1)+work(i,j,k+2))
                    else if (k.gt.2.and.(k.lt.(nz-1+bo2).or.
     &                       (k.eq.(nz-1).and.chr(i,j,nz).eq.ex)).and.
     &                  (chr(i,j,k-1).ne.ex.and.chr(i,j,k-2).ne.ex) )
     &              then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i,j,k-1)+work(i,j,k-2))
                    end if

                    f(i,j,k)=f(i,j,k)-eps_eff*f_hf
                   end if

                 end if
              end do
           end do
         end do
        end do

        return
        end

c---------------------------------------------------------------------------
c applies a simple KO filter to f, where mask != mask_off.
c if do_bdy, then stencil modified as with excision below near boundaries
c
c if (do_ex) then chr is an excision mask ... and for now call
c dmdiss3d_ex
c
c phys_bdy_type specifies wether a given boundary is even, odd (or something
c else, which means the flag is ignored)
c
c NOTE: update DV version too 
c---------------------------------------------------------------------------
        subroutine dmdiss3d(f,work,eps,do_bdy,phys_bdy_type,even,odd,
     &                      mask,mask_off,nx,ny,nz,chr,ex,do_ex)
        implicit none
        integer nx,ny,nz,do_ex,do_bdy,phys_bdy_type(6),even,odd
        real*8 f(nx,ny,nz),work(nx,ny,nz),mask(nx,ny,nz),eps,mask_off
        real*8 chr(nx,ny,nz),ex

        integer i,j,k
        real*8 eps_eff,f_hf,norm_f
        real*8 w4,w3,w2
c        parameter (w4=1.0d0/16.0d0,w3=1.0d0/16.0d0,w2=1.0d0/16.0d0)
        parameter (w4=1.0d0/16.0d0,w3=1.0d0/8.0d0,w2=1.0d0/4.0d0)
        integer pass,npass,cdo_bdy(6)
        logical ind_sweeps
        parameter (ind_sweeps=.true.)

        if (do_ex.ne.0) then
           call dmdiss3d_ex(f,work,eps,do_bdy,phys_bdy_type,even,odd,
     &                      mask,mask_off,nx,ny,nz,chr,ex,ind_sweeps)
           return
        end if

        do i=1,6
           cdo_bdy(i)=do_bdy
           if (phys_bdy_type(i).eq.even.or.phys_bdy_type(i).eq.odd)
     &        cdo_bdy(i)=1
        end do

        eps_eff=eps

        npass=1
        if (ny.gt.1.and.ind_sweeps) npass=2
        if (nz.gt.1.and.ind_sweeps) npass=3

        do pass=1,npass

         do i=1,nx
           do j=1,ny
              do k=1,nz
                 work(i,j,k)=f(i,j,k)
              end do
           end do
         end do

         do i=1,nx
           do j=1,ny
              do k=1,nz
                 if (mask(i,j,k).ne.mask_off) then
                   if (.not.ind_sweeps.or.pass.eq.1) then
                    f_hf=0
                    if (i.gt.2.and.i.lt.(nx-1)) then
                       f_hf=w4*(
     &                     work(i-2,j,k)+work(i+2,j,k)
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                    else 
                     if (cdo_bdy(1).ne.0.and.Nx.ge.5) then
                      if (i.eq.2.and.phys_bdy_type(1).eq.odd) then
                       f_hf=w4*(
     &                    (-work(i,j,k))+work(i+2,j,k)
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                      else if (i.eq.2.and.phys_bdy_type(1).eq.even) then
                       f_hf=w4*(
     &                    (work(i,j,k))+work(i+2,j,k)
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                      else if (i.eq.2) then
                       f_hf=w3*(
     &                    -work(i-1,j,k)+3*work(i,j,k)
     &                  -3*work(i+1,j,k)+work(i+2,j,k))
                      else if (i.eq.1.and.phys_bdy_type(1).eq.odd) then
                       f_hf=w4*(
     &                    (-work(i+2,j,k))+work(i+2,j,k)
     &                -4*((-work(i+1,j,k))+work(i+1,j,k))+6*work(i,j,k))
                      else if (i.eq.1.and.phys_bdy_type(1).eq.even) then
                       f_hf=w4*(
     &                     (work(i+2,j,k))+work(i+2,j,k)
     &                 -4*((work(i+1,j,k))+work(i+1,j,k))+6*work(i,j,k))
                      else if (i.eq.1) then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i+1,j,k)+work(i+2,j,k))
                      end if
                     end if
                     if (cdo_bdy(2).ne.0.and.Nx.ge.5) then
                      if (i.eq.(nx-1).and.phys_bdy_type(2).eq.odd) then
                       f_hf=w4*(
     &                     work(i-2,j,k)+(-work(i,j,k))
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                      else if (i.eq.(nx-1).and.phys_bdy_type(2).eq.even) 
     &                then
                       f_hf=w4*(
     &                     work(i-2,j,k)+(work(i,j,k))
     &                 -4*(work(i-1,j,k)+work(i+1,j,k))+6*work(i,j,k))
                      else if (i.eq.(nx-1)) then
                       f_hf=w3*(
     &                    -work(i+1,j,k)+3*work(i,j,k)
     &                  -3*work(i-1,j,k)+work(i-2,j,k))
                      else if (i.eq.nx.and.phys_bdy_type(2).eq.odd) then
                       f_hf=w4*(
     &                     work(i-2,j,k)+(-work(i-2,j,k))
     &                -4*(work(i-1,j,k)+(-work(i-1,j,k)))+6*work(i,j,k))
                      else if (i.eq.nx.and.phys_bdy_type(2).eq.even) 
     &                then
                       f_hf=w4*(
     &                     work(i-2,j,k)+(work(i-2,j,k))
     &                 -4*(work(i-1,j,k)+(work(i-1,j,k)))+6*work(i,j,k))
                      else if (i.eq.nx) then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i-1,j,k)+work(i-2,j,k))
                      end if
                     end if
                    end if

                    f(i,j,k)=f(i,j,k)-eps_eff*f_hf
                   end if

                   if (.not.ind_sweeps.or.pass.eq.2) then
                    f_hf=0
                    if (j.gt.2.and.j.lt.(ny-1)) then
                       f_hf=w4*(
     &                     work(i,j-2,k)+work(i,j+2,k)
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                    else 
                     if (cdo_bdy(3).ne.0.and.Ny.ge.5) then
                      if (j.eq.2.and.phys_bdy_type(3).eq.odd) then
                       f_hf=w4*(
     &                     (-work(i,j,k))+work(i,j+2,k)
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                      else if (j.eq.2.and.phys_bdy_type(3).eq.even) then
                       f_hf=w4*(
     &                    (work(i,j,k))+work(i,j+2,k)
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                      else if (j.eq.2) then
                       f_hf=w3*(
     &                    -work(i,j-1,k)+3*work(i,j,k)
     &                  -3*work(i,j+1,k)+work(i,j+2,k))
                      else if (j.eq.1.and.phys_bdy_type(3).eq.odd) then
                       f_hf=w4*(
     &                    (-work(i,j+2,k))+work(i,j+2,k)
     &                -4*((-work(i,j+1,k))+work(i,j+1,k))+6*work(i,j,k))
                      else if (j.eq.1.and.phys_bdy_type(3).eq.even) then
                       f_hf=w4*(
     &                     (work(i,j+2,k))+work(i,j+2,k)
     &                 -4*((work(i,j+1,k))+work(i,j+1,k))+6*work(i,j,k))
                      else if (j.eq.1) then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i,j+1,k)+work(i,j+2,k))
                      end if
                     end if
                     if (cdo_bdy(4).ne.0.and.Ny.ge.5) then
                      if (j.eq.(ny-1).and.phys_bdy_type(4).eq.odd) then
                       f_hf=w4*(
     &                     work(i,j-2,k)+(-work(i,j,k))
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                      else if (j.eq.(ny-1).and.phys_bdy_type(4).eq.even)
     &                then
                       f_hf=w4*(
     &                     work(i,j-2,k)+(work(i,j,k))
     &                 -4*(work(i,j-1,k)+work(i,j+1,k))+6*work(i,j,k))
                      else if (j.eq.(ny-1)) then
                       f_hf=w3*(
     &                    -work(i,j+1,k)+3*work(i,j,k)
     &                  -3*work(i,j-1,k)+work(i,j-2,k))
                      else if (j.eq.ny.and.phys_bdy_type(4).eq.odd) then
                       f_hf=w4*(
     &                    work(i,j-2,k)+(-work(i,j-2,k))
     &                -4*(work(i,j-1,k)+(-work(i,j-1,k)))+6*work(i,j,k))
                      else if (j.eq.ny.and.phys_bdy_type(4).eq.even) 
     &                then
                       f_hf=w4*(
     &                     work(i,j-2,k)+(work(i,j-2,k))
     &                 -4*(work(i,j-1,k)+(work(i,j-1,k)))+6*work(i,j,k))
                      else if (j.eq.ny) then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i,j-1,k)+work(i,j-2,k))
                      end if
                     end if
                    end if
         
                    f(i,j,k)=f(i,j,k)-eps_eff*f_hf
                   end if

                   if (.not.ind_sweeps.or.pass.eq.3) then
                    f_hf=0
                    if (k.gt.2.and.k.lt.(nz-1)) then
                       f_hf=w4*(
     &                     work(i,j,k-2)+work(i,j,k+2)
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                    else 
                     if (cdo_bdy(5).ne.0.and.Nz.ge.5) then
                      if (k.eq.2.and.phys_bdy_type(5).eq.odd) then
                       f_hf=w4*(
     &                    (-work(i,j,k))+work(i,j,k+2)
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                      else if (k.eq.2.and.phys_bdy_type(5).eq.even) then
                       f_hf=w4*(
     &                    (work(i,j,k))+work(i,j,k+2)
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                      else if (k.eq.2) then
                       f_hf=w3*(
     &                    -work(i,j,k-1)+3*work(i,j,k)
     &                  -3*work(i,j,k+1)+work(i,j,k+2))
                      else if (k.eq.1.and.phys_bdy_type(5).eq.odd) then
                       f_hf=w4*(
     &                    (-work(i,j,k+2))+work(i,j,k+2)
     &                -4*((-work(i,j,k+1))+work(i,j,k+1))+6*work(i,j,k))
                      else if (k.eq.1.and.phys_bdy_type(5).eq.even) then
                       f_hf=w4*(
     &                     (work(i,j,k+2))+work(i,j,k+2)
     &                 -4*((work(i,j,k+1))+work(i,j,k+1))+6*work(i,j,k))
                      else if (k.eq.1) then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i,j,k+1)+work(i,j,k+2))
                      end if
                     end if
                     if (cdo_bdy(6).ne.0.and.Nz.ge.5) then
                      if (k.eq.(nz-1).and.phys_bdy_type(6).eq.odd) then
                       f_hf=w4*(
     &                     work(i,j,k-2)+(-work(i,j,k))
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                      else if (k.eq.(nz-1).and.phys_bdy_type(6).eq.even) 
     &                then
                       f_hf=w4*(
     &                     work(i,j,k-2)+(work(i,j,k))
     &                 -4*(work(i,j,k-1)+work(i,j,k+1))+6*work(i,j,k))
                      else if (k.eq.(nz-1)) then
                       f_hf=w3*(
     &                    -work(i,j,k+1)+3*work(i,j,k)
     &                  -3*work(i,j,k-1)+work(i,j,k-2))
                      else if (k.eq.nz.and.phys_bdy_type(6).eq.odd) then
                       f_hf=w4*(
     &                    work(i,j,k-2)+(-work(i,j,k-2))
     &                -4*(work(i,j,k-1)+(-work(i,j,k-1)))+6*work(i,j,k))
                      else if (k.eq.nz.and.phys_bdy_type(6).eq.even) 
     &                then
                       f_hf=w4*(
     &                     work(i,j,k-2)+(work(i,j,k-2))
     &                 -4*(work(i,j,k-1)+(work(i,j,k-1)))+6*work(i,j,k))
                      else if (k.eq.nz) then
                       f_hf=w2*(
     &                     work(i,j,k)-2*work(i,j,k-1)+work(i,j,k-2))
                      end if
                     end if
                    end if

                    f(i,j,k)=f(i,j,k)-eps_eff*f_hf
                   end if
                 end if
              end do
           end do
         end do
        end do

        return
        end
c---------------------------------------------------------------------------
c repopulates all grid points of f *one* point in from an excision surface,
c via at most (linear[2]/quadratic[3]/cubic[4]) extrapolation from adjacent
c cells
c
c   * --- o --- o --- o --- o
c
c   f_ex  f1    f2    f3    f4
c
c 2nd order: f_ex = 2*f1 - f2
c
c 3rd order: f_ex = 3*f1 - 3*f2 + f3
c
c 3rd order: f_ex = 4*f1 - 6*f2 + 4*f3 - f4
c
c io = interpolation order
c---------------------------------------------------------------------------
        subroutine dmrepop3d1(f,chr,ex,io,nx,ny,nz)
        implicit none
        integer nx,ny,nz,io
        real*8 f(nx,ny,nz)
        real*8 chr(nx,ny,nz),ex

        integer i,j,k,wx,wy,wz
        real*8 fx,fy,fz

        if (io.lt.2.or.io.gt.4) then
           write(*,*) 'dmrepop3d1: only linear(2), quadratic(3)'
           write(*,*) 'and cubic(4) supported at this time'
           return
        end if

        do i=1,Nx
           do j=1,Ny
              do k=1,Nz
                 if (chr(i,j,k).eq.ex) then
                    wx=0
                    wy=0
                    wz=0
                    fx=0
                    fy=0
                    fz=0

                    if ((i.eq.3.or.(i.ge.3.and.io.eq.2)).and.
     &                  chr(i-1,j,k).ne.ex.and.chr(i-2,j,k).ne.ex) then
                       fx=2*f(i-1,j,k)-f(i-2,j,k)
                       wx=2
                    else if ((i.eq.4.or.(i.ge.4.and.io.eq.3)).and.
     &                  chr(i-1,j,k).ne.ex.and.chr(i-2,j,k).ne.ex.and.
     &                  chr(i-3,j,k).ne.ex) then
                       fx=3*(f(i-1,j,k)-f(i-2,j,k))+f(i-3,j,k)
                       wx=3
                    else if ((i.ge.5.and.io.eq.4).and.
     &                  chr(i-1,j,k).ne.ex.and.chr(i-2,j,k).ne.ex.and.
     &                  chr(i-3,j,k).ne.ex.and.chr(i-4,j,k).ne.ex) then
                       fx= 4*(f(i-1,j,k)+f(i-3,j,k))
     &                    -6*f(i-2,j,k)-f(i-4,j,k)
                       wx=4
                    else if ((i.eq.Nx-2.or.(i.le.Nx-2.and.io.eq.2)).and.
     &                  chr(i+1,j,k).ne.ex.and.chr(i+2,j,k).ne.ex) then
                       fx=2*f(i+1,j,k)-f(i+2,j,k)
                       wx=2
                    else if ((i.eq.Nx-3.or.(i.le.Nx-3.and.io.eq.3)).and.
     &                  chr(i+1,j,k).ne.ex.and.chr(i+2,j,k).ne.ex.and.
     &                  chr(i+3,j,k).ne.ex) then
                       fx=3*(f(i+1,j,k)-f(i+2,j,k))+f(i+3,j,k)
                       wx=3
                    else if ((i.le.Nx-4.and.io.eq.4).and.
     &                  chr(i+1,j,k).ne.ex.and.chr(i+2,j,k).ne.ex.and.
     &                  chr(i+3,j,k).ne.ex.and.chr(i+4,j,k).ne.ex) then
                       fx= 4*(f(i+1,j,k)+f(i+3,j,k))
     &                    -6*f(i+2,j,k)-f(i+4,j,k)
                       wx=4
                    end if

                    if ((j.eq.3.or.(j.ge.3.and.io.eq.2)).and.
     &                  chr(i,j-1,k).ne.ex.and.chr(i,j-2,k).ne.ex) then
                       fy=2*f(i,j-1,k)-f(i,j-2,k)
                       wy=2
                    else if ((j.eq.4.or.(j.ge.4.and.io.eq.3)).and.
     &                  chr(i,j-1,k).ne.ex.and.chr(i,j-2,k).ne.ex.and.
     &                  chr(i,j-3,k).ne.ex) then
                       fy=3*(f(i,j-1,k)-f(i,j-2,k))+f(i,j-3,k)
                       wy=3
                    else if ((j.ge.5.and.io.eq.4).and.
     &                  chr(i,j-1,k).ne.ex.and.chr(i,j-2,k).ne.ex.and.
     &                  chr(i,j-3,k).ne.ex.and.chr(i,j-4,k).ne.ex) then
                       fy= 4*(f(i,j-1,k)+f(i,j-3,k))
     &                    -6*f(i,j-2,k)-f(i,j-4,k)
                       wy=4
                    else if ((j.eq.Ny-2.or.(j.le.Ny-2.and.io.eq.2)).and.
     &                  chr(i,j+1,k).ne.ex.and.chr(i,j+2,k).ne.ex) then
                       fy=2*f(i,j+1,k)-f(i,j+2,k)
                       wy=2
                    else if ((j.eq.Ny-3.or.(j.le.Ny-3.and.io.eq.3)).and.
     &                  chr(i,j+1,k).ne.ex.and.chr(i,j+2,k).ne.ex.and.
     &                  chr(i,j+3,k).ne.ex) then
                       fy=3*(f(i,j+1,k)-f(i,j+2,k))+f(i,j+3,k)
                       wy=3
                    else if ((j.le.Ny-4.and.io.eq.4).and.
     &                  chr(i,j+1,k).ne.ex.and.chr(i,j+2,k).ne.ex.and.
     &                  chr(i,j+3,k).ne.ex.and.chr(i,j+4,k).ne.ex) then
                       fy= 4*(f(i,j+1,k)+f(i,j+3,k))
     &                    -6*f(i,j+2,k)-f(i,j+4,k)
                       wy=4
                    end if

                    if ((k.eq.3.or.(k.ge.3.and.io.eq.2)).and.
     &                  chr(i,j,k-1).ne.ex.and.chr(i,j,k-2).ne.ex) then
                       fz=2*f(i,j,k-1)-f(i,j,k-2)
                       wz=2
                    else if ((k.eq.4.or.(k.ge.4.and.io.eq.3)).and.
     &                  chr(i,j,k-1).ne.ex.and.chr(i,j,k-2).ne.ex.and.
     &                  chr(i,j,k-3).ne.ex) then
                       fz=3*(f(i,j,k-1)-f(i,j,k-2))+f(i,j,k-3)
                       wz=3
                    else if ((k.ge.5.and.io.eq.4).and.
     &                  chr(i,j,k-1).ne.ex.and.chr(i,j,k-2).ne.ex.and.
     &                  chr(i,j,k-3).ne.ex.and.chr(i,j,k-4).ne.ex) then
                       fz= 4*(f(i,j,k-1)+f(i,j,k-3))
     &                    -6*f(i,j,k-2)-f(i,j,k-4)
                       wz=4
                    else if ((k.eq.Nz-2.or.(k.le.Nz-2.and.io.eq.2)).and.
     &                  chr(i,j,k+1).ne.ex.and.chr(i,j,k+2).ne.ex) then
                       fz=2*f(i,j,k+1)-f(i,j,k+2)
                       wz=2
                    else if ((k.eq.Nz-3.or.(k.le.Nz-3.and.io.eq.3)).and.
     &                  chr(i,j,k+1).ne.ex.and.chr(i,j,k+2).ne.ex.and.
     &                  chr(i,j,k+3).ne.ex) then
                       fz=3*(f(i,j,k+1)-f(i,j,k+2))+f(i,j,k+3)
                       wz=3
                    else if ((k.le.Nz-4.and.io.eq.4).and.
     &                  chr(i,j,k+1).ne.ex.and.chr(i,j,k+2).ne.ex.and.
     &                  chr(i,j,k+3).ne.ex.and.chr(i,j,k+4).ne.ex) then
                       fz= 4*(f(i,j,k+1)+f(i,j,k+3))
     &                    -6*f(i,j,k+2)-f(i,j,k+4)
                       wz=4
                    end if
                    if (wx+wy+wz.ne.0) then
                       !-----------------------------------------------
                       ! only blend extrapolations of the same order
                       !-----------------------------------------------
                       if (wx.gt.wy) wy=0
                       if (wx.gt.wz) wz=0
                       if (wy.gt.wx) wx=0
                       if (wy.gt.wz) wz=0
                       if (wz.gt.wx) wx=0
                       if (wz.gt.wy) wy=0
                       f(i,j,k)=(wx*fx+wy*fy+wz*fz)/(wx+wy+wz)
                    end if
                 end if
              end do
           end do
        end do

        return
        end

c---------------------------------------------------------------------------
c Kreiss-Oliger-like smoothing of R (or theta, or whatever)
c
c + removes *isolated* "spikes" that sometimes arise in late time
c distorted holes prior to merger with rem_spikes option
c (non-axisym only!)
c 
c-----------------------------------------------------------------------
        subroutine smooth_ah_r(AH_R,AH_var,AH_w1,AH_eps,
     &                         AH_Ntheta,AH_Nphi)
        implicit none
        integer AH_Ntheta,AH_Nphi
        real*8 AH_R(AH_Ntheta,AH_Nphi),AH_eps
        real*8 AH_w1(AH_Ntheta,AH_Nphi)
        real*8 AH_var(AH_Ntheta,AH_Nphi)
        
        integer i,j,jp1,jp2,jm1,jm2
        real*8 r0,r1,rip1,rip2,rim1,rim2,rjp1,rjp2,rjm1,rjm2,r

        logical rem_spikes
        parameter (rem_spikes=.true.)
        real*8 max_var,avg_var,v1,v2,v3,v4
        parameter (max_var=5)

        if (AH_Nphi.gt.1.and.rem_spikes) then
           avg_var=0
           do i=1,AH_Ntheta
              do j=1,AH_Nphi
                 AH_var(i,j)=0
              end do
           end do
           do i=2,AH_Ntheta-1
              do j=1,AH_Nphi-1
                 jm1=j-1
                 if (jm1.lt.1) jm1=jm1+AH_Nphi-1
                 jp1=j+1
                 if (jp1.gt.AH_Nphi) jp1=jp1-AH_Nphi+1
                 r=AH_R(i,j)
                 v1=(r-AH_R(i+1,j))**2
                 v2=(r-AH_R(i-1,j))**2
                 v3=(r-AH_R(i,jp1))**2
                 v4=(r-AH_R(i,jm1))**2
                 !-----------------------------------------------------
                 ! by subtracting the max, we are only sensitive
                 ! to isolated spikes (i.e, the points adjacent to 
                 ! the spike will now have a small variance)
                 !-----------------------------------------------------
                 AH_var(i,j)=sqrt(v1+v2+v3+v4-max(v1,v2,v3,v4))
                 avg_var=avg_var+AH_var(i,j)
              end do
           end do
           avg_var=avg_var/AH_Nphi/(AH_Ntheta-2)
           do i=1,AH_Ntheta
              do j=1,AH_Nphi
                 AH_var(i,j)=AH_var(i,j)/avg_var
              end do
           end do
           do i=2,AH_Ntheta-1
              do j=1,AH_Nphi-1
                 jm1=j-1
                 if (jm1.lt.1) jm1=jm1+AH_Nphi-1
                 jp1=j+1
                 if (jp1.gt.AH_Nphi) jp1=jp1-AH_Nphi+1
                 if (AH_var(i,j).gt.max_var.and.
     &               AH_var(i+1,j).lt.max_var.and.
     &               AH_var(i-1,j).lt.max_var.and.
     &               AH_var(i,jp1).lt.max_var.and.
     &               AH_var(i,jm1).lt.max_var) then
                    AH_r(i,j)=0.25d0*(AH_R(i+1,j)+AH_R(i-1,j)+
     &                                AH_R(i,jp1)+AH_R(i,jm1))
                 end if
              end do
           end do
           do i=1,AH_Ntheta
              AH_r(i,AH_Nphi)=AH_r(i,1)
           end do
        end if

        r0=0
        r1=0
        do i=1,AH_Nphi
           r0=r0+AH_R(1,i)
           r1=r1+AH_R(AH_Ntheta,i)
        end do
        r0=r0/AH_Nphi
        r1=r1/AH_Nphi
        do i=1,AH_Nphi
           AH_r(1,i)=r0
           AH_r(AH_Ntheta,i)=r1
        end do

        do i=1,AH_Ntheta
           do j=1,AH_Nphi
              AH_w1(i,j)=AH_r(i,j)
           end do
        end do

        do i=2,AH_Ntheta-1
           do j=1,AH_Nphi
              r=AH_w1(i,j)
              rip1=AH_w1(i+1,j)
              rim1=AH_w1(i-1,j)
              if (i.eq.(AH_Ntheta-1)) then
                 rip2=r
              else
                 rip2=AH_w1(i+2,j)
              end if
              if (i.eq.2) then
                 rim2=r
              else
                 rim2=AH_w1(i-2,j)
              end if
              if (AH_Nphi.lt.5) then
                 jp1=j
                 jm1=j
                 jp2=j
                 jm2=j
              else
                 jp1=j+1
                 jp2=j+2
                 jm1=j-1
                 jm2=j-2
                 if (jp1.gt.AH_Nphi) jp1=jp1-AH_Nphi+1
                 if (jp2.gt.AH_Nphi) jp2=jp2-AH_Nphi+1
                 if (jm1.lt.1) jm1=jm1+AH_Nphi-1
                 if (jm2.lt.1) jm2=jm2+AH_Nphi-1
              end if
              rjp1=AH_w1(i,jp1)
              rjp2=AH_w1(i,jp2)
              rjm1=AH_w1(i,jm1)
              rjm2=AH_w1(i,jm2)

              AH_r(i,j)=r-AH_eps/16*(
     &          rip2+rim2-4*(rip1+rim1)+6*r +
     &          rjp2+rjm2-4*(rjp1+rjm1)+6*r)
           end do
        end do
        
        do i=1,AH_Ntheta
           AH_r(i,1)=AH_r(i,AH_Nphi)
        end do

        return 
        end

c======================================================================
c try to find best-fit ellipsoid match to AH shape ... replaces
c AH_R with that shape
c
c the equation for an ellipse with semi-majour axis rx at an angle phi0
c to phi=0, with semi-minor axis ry, and 'z' axis rz is
c
c t=theta
c p=phi
c p0=phi0
c
c st=sin(theta), ct=cos(theta), etc
c
c 1/r^2 = st^2/rx^2*(cp*cp0+sp*sp0)^2 + st^2/ry^2*(-cp*sp0+sp*cp0)^2 + ct^2/rz^2
c
c======================================================================
        subroutine gh3d_ah_best_fit(AH_r,AH_Ntheta,AH_Nphi)
        implicit none
        integer AH_Ntheta,AH_Nphi
        real*8 AH_R(AH_Ntheta,AH_Nphi)
        
        real*8 PI,frac,phi0
        integer MAX_N
        parameter (PI=3.14159265359d0,MAX_N=200)
        logical USE_MIND,FIND_BEST_FIT
        parameter (USE_MIND=.true.,FIND_BEST_FIT=.true.)

        real*8 ex_r0(3),AH_xc(3),ex_xc0(3)

        real*8 x,y,z,theta,phi,dtheta,dphi,dx,dy,maxd,d0(MAX_N),mind
        real*8 AH_r0_bf
        real*8 xmin,xmax,ymin,ymax,zmin,zmax
        real*8 x_smj,x_smn,y_smj,y_smn,x_mind,y_mind
        real*8 ct,st,cp,sp,cp0,sp0
        integer i,j,i0,j_phi0,jp1,jm1,hnp,hnt,j_mind
        real*8 phi_bf,dphi_bf,phi_bf_range,phi_t,phi_bf_err,phi_t_err
        parameter (phi_bf_range=0.8,dphi_bf=0.02)

        hnp=(AH_Nphi-1)/2
        hnt=(AH_Ntheta-1)/2
        dtheta=PI/(AH_Ntheta-1)
        dphi=2*PI/(AH_Nphi-1)

        AH_xc(1)=0
        AH_xc(2)=0
        AH_xc(3)=0
        ex_xc0(1)=0
        ex_xc0(2)=0
        ex_xc0(3)=0
        xmin=1
        xmax=-1
        ymin=1
        ymax=-1
        zmin=1
        zmax=-1

        !------------------------------------------------------------
        ! find phi0, the angle of the semi-majour axis relative to 
        ! the x axis along theta=Pi/2 ... the following calculation
        ! will always set phi0 between 0 and Pi
        !------------------------------------------------------------
        i0=hnt+1
        theta=(i0-1)*dtheta
        maxd=0
        mind=100
        do j=1,hnp
           phi=(j-1)*dphi
           dx=2/PI*atan(AH_R(i0,j)*cos(phi)*sin(theta)+AH_xc(1))-
     &        2/PI*atan(AH_R(i0,j+hnp)*
     &                  cos(phi+PI)*sin(theta)+AH_xc(1))
           dy=2/PI*atan(AH_R(i0,j)*sin(phi)*sin(theta)+AH_xc(2))-
     &        2/PI*atan(AH_R(i0,j+hnp)*
     &                  sin(phi+PI)*sin(theta)+AH_xc(2))
           d0(j)=sqrt(dx**2+dy**2)
           if (d0(j).gt.maxd) then
              maxd=d0(j)
              phi0=phi
              j_phi0=j
           end if
           if (d0(j).lt.mind) then
              mind=d0(j)
              j_mind=j
           end if
        end do
        d0(hnp+1)=d0(1)

        do i=1,AH_Ntheta
           do j=1,AH_Nphi

              theta=(i-1)*dtheta
              phi=(j-1)*dphi

              x=2/PI*atan(AH_R(i,j)*cos(phi)*sin(theta)+AH_xc(1))
              y=2/PI*atan(AH_R(i,j)*sin(phi)*sin(theta)+AH_xc(2))

              xmin=min(x,xmin)
              ymin=min(y,ymin)

              xmax=max(x,xmax)
              ymax=max(y,ymax)

              z=2/PI*atan(AH_R(i,j)*cos(theta)+AH_xc(3))

              zmin=min(z,zmin)
              zmax=max(z,zmax)

              ! save positions of semi-major/minor axis

              if (i.eq.((AH_Ntheta-1)/2+1).and.j.eq.j_phi0) then
                 x_smj=x
                 y_smj=y
              end if
              if (i.eq.((AH_Ntheta-1)/2+1).and.
     &                  j.eq.(j_phi0+(AH_Nphi-1)/4)) then
                 x_smn=x
                 y_smn=y
              end if
              if (i.eq.((AH_Ntheta-1)/2+1).and.
     &                  j.eq.j_mind) then
                 x_mind=x
                 y_mind=y
              end if
           end do
        end do

        ! hnp+1 maps to 1

        jp1=j_phi0+1
        if (jp1.gt.hnp) jp1=1
        jm1=j_phi0-1
        if (jm1.lt.1) jm1=hnp

        ! the following interpolates in phi for a better guess at the max

        if ((d0(j_phi0)-d0(jm1)).gt.(d0(j_phi0)-d0(jp1))) then
           frac=(d0(jp1)-d0(jm1))/(d0(j_phi0)-d0(jm1))
           if (frac.lt.0.or.frac.gt.1) then
              write(*,*) 'fill_ex_params: WARNING frac(1)=',frac
           end if
           phi0=phi0+frac/2*dphi
        else
           frac=(d0(jm1)-d0(jp1))/(d0(j_phi0)-d0(jp1))
           if (frac.lt.0.or.frac.gt.1) then
              write(*,*) 'fill_ex_params: WARNING frac(2)=',frac
           end if
           phi0=phi0-frac/2*dphi
        end if
        ex_r0(1)=sqrt((x_smj-ex_xc0(1))**2+(y_smj-ex_xc0(2))**2)
        ex_r0(2)=sqrt((x_smn-ex_xc0(1))**2+(y_smn-ex_xc0(2))**2)
        if (USE_MIND) then
           ex_r0(2)=sqrt((x_mind-ex_xc0(1))**2+(y_mind-ex_xc0(2))**2)
        end if
        ex_r0(3)=(zmax-zmin)/2

        ! reconstruct Ellipse

        if (FIND_BEST_FIT) then
           phi_bf=phi0-phi_bf_range/2
           phi_bf_err=1000
           do phi_t=phi0-phi_bf_range/2,phi0+phi_bf_range/2,dphi_bf
              phi_t_err=0
              i0=hnt+1
              st=1
              ct=0
              cp0=cos(phi_t)
              sp0=sin(phi_t)
              do j=1,AH_Nphi
                 phi=(j-1)*dphi
                 sp=sin(phi)
                 cp=cos(phi)
                 AH_r0_bf=1/sqrt(st**2/ex_r0(1)**2*(cp*cp0+sp*sp0)**2 +
     &                           st**2/ex_r0(2)**2*(-cp*sp0+sp*cp0)**2 +
     &                           ct**2/ex_r0(3)**2)
                 AH_r0_bf=AH_r0_bf*PI/2
                 phi_t_err=phi_t_err+(AH_r0_bf-AH_r(i0,j))**2
              end do
              phi_t_err=sqrt(phi_t_err)/AH_Nphi
              write(*,*) 'phi0=,',phi0,' phi_t=',phi_t,
     *                   ' phi_t_err=',phi_t_err
              if (phi_t_err.lt.phi_bf_err) then
                 phi_bf=phi_t
                 phi_bf_err=phi_t_err
              end if
           end do
           phi0=phi_bf
           if (phi0.lt.0) phi0=phi0-PI
           if (phi0.gt.PI) phi0=phi0-PI
        end if

        write(*,*) 'ex_r0=',ex_r0, ' phi0=',phi0

        cp0=cos(phi0)
        sp0=sin(phi0)
        do i=1,AH_Ntheta
           do j=1,AH_Nphi
              theta=(i-1)*dtheta
              phi=(j-1)*dphi
              st=sin(theta)
              ct=cos(theta)
              sp=sin(phi)
              cp=cos(phi)
              AH_r(i,j)=1/sqrt(st**2/ex_r0(1)**2*(cp*cp0+sp*sp0)**2 +
     &                         st**2/ex_r0(2)**2*(-cp*sp0+sp*cp0)**2 + 
     &                         ct**2/ex_r0(3)**2)
              ! compactification should be neglegible for these r's,
              ! expect for the factor of PI/2
              AH_r(i,j)=PI/2*AH_r(i,j)
           end do
        end do

        return
        end
c======================================================================
c calculates the quadrupoler contribution to a 4-times integrated
c in time psi4, mulitplied by r0, for a two-point particle system
c======================================================================
        subroutine fill_i4psi4(i4psi4_r,i4psi4_i,r0,m1,x1,y1,z1,
     &                         m2,x2,y2,z2,Ntheta,Nphi)
        implicit none
        integer Ntheta,Nphi
        real*8 r0,m1,x1,y1,z1,m2,x2,y2,z2
        real*8 i4psi4_r(Ntheta,Nphi),i4psi4_i(Ntheta,Nphi)

        integer i,j,k,l,a,b
        real*8 PI,theta,phi
        parameter (PI=3.14159265359d0)

        real*8 I_kl(3,3),r1,r2,p_kl(3,3),n_k(3),J_kl(3,3)
        real*8 t_h(3),p_h(3),J_tt,J_pp,J_tp

        r1=sqrt(x1**2+y1**2+z1**2)
        r2=sqrt(x2**2+y2**2+z2**2)

        !reduced quadrupole moment:

        I_kl(1,1)=m1*(x1**2-r1**2/3) + m2*(x2**2-r2**2/3)
        I_kl(1,2)=m1*(x1*y1) + m2*(x2*y2)
        I_kl(1,3)=m1*(x1*z1) + m2*(x2*z2)
        I_kl(2,2)=m1*(y1**2-r1**2/3) + m2*(y2**2-r2**2/3)
        I_kl(2,3)=m1*(y1*z1) + m2*(y2*z2)
        I_kl(3,3)=m1*(z1**2-r1**2/3) + m2*(z2**2-r2**2/3)

        I_kl(2,1)=I_kl(1,2)
        I_kl(3,1)=I_kl(1,3)
        I_kl(3,2)=I_kl(2,3)

        do i=1,Ntheta
           do j=1,Nphi
              theta=(i-1)*PI/(Ntheta-1)
              phi=(j-1)*2*PI/(Nphi-1)

              ! measurement direction
              
              n_k(1)=sin(theta)*cos(phi)
              n_k(2)=sin(theta)*sin(phi)
              n_k(3)=cos(theta)

              ! unit vectors in theta, phi and r direction (which is n)

              t_h(1)=cos(theta)*cos(phi)
              t_h(2)=cos(theta)*sin(phi)
              t_h(3)=-sin(theta)
              
              p_h(1)=-sin(phi)
              p_h(2)= cos(phi)
              p_h(3)=0

              ! projection tensor
              
              do k=1,3
                 do l=1,3
                    p_kl(k,l)=0
                    if (k.eq.l) p_kl(k,l)=1
                    p_kl(k,l)=p_kl(k,l)-n_k(k)*n_k(l)
                 end do
              end do

              ! Projection of I, cartesian coordinates, with 1/r factor

              do a=1,3
                 do b=1,3
                    J_kl(a,b)=0
                    do k=1,3
                       do l=1,3
                          J_kl(a,b)=J_kl(a,b)+2*I_kl(k,l)*
     &                       (       p_kl(a,k)*p_kl(b,l)
     &                        -0.5d0*p_kl(k,l)*p_kl(a,b))
                       end do
                    end do
                 end do
              end do

              ! project J onto spherical polar basis

              J_tt=0
              J_tp=0
              J_pp=0

              do a=1,3
                 do b=1,3
                    J_tt=J_tt+J_kl(a,b)*t_h(a)*t_h(b)
                    J_pp=J_pp+J_kl(a,b)*p_h(a)*p_h(b)
                    J_tp=J_tp+J_kl(a,b)*t_h(a)*p_h(b)
                 end do
              end do

              ! components of 4-times integrated psi4

              i4psi4_r(i,j)=0.5d0*(J_tt-J_pp)
              i4psi4_i(i,j)=-J_tp

           end do
        end do

        return
        end
c======================================================================
c 1D smoothing ... dissipates f, with eps, wavelengths 1..k,
c m iterations
c======================================================================
        subroutine diss_ko_1d(f,eps,k,m,n)
        implicit none
        integer n,k,m
        real*8 f(n),eps

        integer i,j,k0
        integer n_max
        parameter (n_max=5000)
        real*8 w(n_max)

        if (n.gt.n_max) then
           write(*,*) 'diss_ko_1d ... error : array too large'
           return
        end if

        do j=1,m
           do k0=k,1,-1
              do i=1,n
                 w(i)=f(i)
              end do
              do i=2*k+1,n-2*k
                 f(i)=w(i)-
     &            eps*(w(i-2*k)+w(i+2*k)-4*(w(i-k)+w(i+k))+6*w(i))/16
              end do
           end do
        end do

        return
        end

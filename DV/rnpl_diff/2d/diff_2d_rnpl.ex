########################################################################
# single grid, masked difference operators for DV
########################################################################
system parameter int memsiz := 10000000

rec2 coordinates t,x1,x2

uniform rec2 grid g2 [1:Nx1][1:Nx2] {x1min:x1max} {x2min:x2max}

float f2 on g2
float mask2 on g2

float f2_x1 on g2
float f2_x2 on g2
float f2_x1_CN on g2
float f2_x2_CN on g2

parameter float mask_val  :=  1.0

operator ZERO(f,x1) := 0
operator ZERO(f,x2) := 0
operator D2d_0(f,x1) := (<0>f[1][0]-<0>f[-1][0])/(2*dx1)
operator D2d_0_CN(f,x1) := (<0>f[1][0]-<0>f[0][0])/(dx1)
operator D2d_BW(f,x1) := (3*<0>f[0][0]-4*<0>f[-1][0]+<0>f[-2][0])/(2*dx1)
operator D2d_FW(f,x1) := (-3*<0>f[0][0]+4*<0>f[1][0]-<0>f[2][0])/(2*dx1)
operator D2d_0(f,x2) := (<0>f[0][1]-<0>f[0][-1])/(2*dx2)
operator D2d_0_CN(f,x2) := (<0>f[0][1]-<0>f[0][0])/(dx2)
operator D2d_BW(f,x2) := (3*<0>f[0][0]-4*<0>f[0][-1]+<0>f[0][-2])/(2*dx2)
operator D2d_FW(f,x2) := (-3*<0>f[0][0]+4*<0>f[0][1]-<0>f[0][2])/(2*dx2)

#=======================================================================
# directives for excision pre-processor
#=======================================================================
#%define mask(mask2;mask_val;[0][0])
#%define masked_op(D2d_0;x1;[-1][0]->D2d_FW,[1][0]->D2d_BW)
#%define masked_op(D2d_0;x2;[0][-1]->D2d_FW,[0][1]->D2d_BW)
#%define masked_op(D2d_0_CN;x1;[1][0]->ZERO)
#%define masked_op(D2d_0_CN;x2;[0][1]->ZERO)
#%define masked_op(D2d_FW;x1;)
#%define masked_op(D2d_BW;x1;)
#%define masked_op(D2d_FW;x2;)
#%define masked_op(D2d_BW;x2;)

#-----------------------------------------------------------------------
residual f2_x1
{ 
   [1:1][1:Nx2] := f2_x1 = $D2d_FW(f2,x1); 
   [2:Nx1-1][1:Nx2] := f2_x1 = $D2d_0(f2,x1); 
   [Nx1:Nx1][1:Nx2] := f2_x1 = $D2d_BW(f2,x1); 
}
residual f2_x2 
{ 
   [1:Nx1][1:1] := f2_x2 = $D2d_FW(f2,x2); 
   [1:Nx1][2:Nx2-1] := f2_x2 = $D2d_0(f2,x2); 
   [1:Nx1][Nx2:Nx2] := f2_x2 = $D2d_BW(f2,x2); 
}
residual f2_x1_CN
{
   [1:Nx1-1][1:Nx2] := f2_x1_CN = $D2d_0_CN(f2,x1);
   [Nx1:Nx1][1:Nx2] := <0>f2_x1_CN[0][0] = <0>f2_x1_CN[-1][0];
} 
residual f2_x2_CN
{
   [1:Nx1][1:Nx2-1] := f2_x2_CN = $D2d_0_CN(f2,x2);
   [1:Nx1][Nx2:Nx2] := <0>f2_x2_CN[0][0] = <0>f2_x2_CN[0][-1];
} 

auto update f2_x1
auto update f2_x2
auto update f2_x1_CN
auto update f2_x2_CN

########################################################################
# single grid, masked difference operators for DV
########################################################################
system parameter int memsiz := 10000000

rec3 coordinates t,x1,x2,x3

uniform rec3 grid g3 [1:Nx1][1:Nx2][1:Nx3] {x1min:x1max} {x2min:x2max} {x3min:x3max}

float f3 on g3
float mask3 on g3

float f3_x1 on g3
float f3_x2 on g3
float f3_x3 on g3
float f3_x1_CN on g3
float f3_x2_CN on g3
float f3_x3_CN on g3

parameter float mask_val  :=  1.0

operator ZERO(f,x1) := (0)
operator ZERO(f,x2) := (0)
operator ZERO(f,x3) := (0)
operator D3d_0(f,x1) := (<0>f[1][0][0]-<0>f[-1][0][0])/(2*dx1)
operator D3d_0_CN(f,x1) := (<0>f[1][0][0]-<0>f[0][0][0])/(dx1)
operator D3d_BW(f,x1) := (3*<0>f[0][0][0]-4*<0>f[-1][0][0]+<0>f[-2][0][0])/(2*dx1)
operator D3d_FW(f,x1) := (-3*<0>f[0][0][0]+4*<0>f[1][0][0]-<0>f[2][0][0])/(2*dx1)
operator D3d_0(f,x2) := (<0>f[0][1][0]-<0>f[0][-1][0])/(2*dx2)
operator D3d_0_CN(f,x2) := (<0>f[0][1][0]-<0>f[0][0][0])/(dx2)
operator D3d_BW(f,x2) := (3*<0>f[0][0][0]-4*<0>f[0][-1][0]+<0>f[0][-2][0])/(2*dx2)
operator D3d_FW(f,x2) := (-3*<0>f[0][0][0]+4*<0>f[0][1][0]-<0>f[0][2][0])/(2*dx2)
operator D3d_0(f,x3) := (<0>f[0][0][1]-<0>f[0][0][-1])/(2*dx3)
operator D3d_0_CN(f,x3) := (<0>f[0][0][1]-<0>f[0][0][0])/(dx3)
operator D3d_BW(f,x3) := (3*<0>f[0][0][0]-4*<0>f[0][0][-1]+<0>f[0][0][-2])/(2*dx3)
operator D3d_FW(f,x3) := (-3*<0>f[0][0][0]+4*<0>f[0][0][1]-<0>f[0][0][2])/(2*dx3)

#=======================================================================
# directives for excision pre-processor
#=======================================================================
#%define mask(mask3;mask_val;[0][0][0])

#%define masked_op(D3d_0;x1;[-1][0][0]->D3d_FW,[1][0][0]->D3d_BW)
#%define masked_op(D3d_0;x2;[0][-1][0]->D3d_FW,[0][1][0]->D3d_BW)
#%define masked_op(D3d_0;x3;[0][0][-1]->D3d_FW,[0][0][1]->D3d_BW)
#%define masked_op(D3d_0_CN;x1;[1][0][0]->ZERO)
#%define masked_op(D3d_0_CN;x2;[0][1][0]->ZERO)
#%define masked_op(D3d_0_CN;x3;[0][0][1]->ZERO)
#%define masked_op(D3d_FW;x1;)
#%define masked_op(D3d_BW;x1;)
#%define masked_op(D3d_FW;x2;)
#%define masked_op(D3d_BW;x2;)
#%define masked_op(D3d_FW;x3;)
#%define masked_op(D3d_BW;x3;)

#-----------------------------------------------------------------------
residual f3_x1 
{ 
   [1:1][1:Nx2][1:Nx3] := f3_x1 = $D3d_FW(f3,x1); 
   [2:Nx1-1][1:Nx2][1:Nx3] := f3_x1 = $D3d_0(f3,x1); 
   [Nx1:Nx1][1:Nx2][1:Nx3] := f3_x1 = $D3d_BW(f3,x1); 
}
residual f3_x2 
{ 
   [1:Nx1][1:1][1:Nx3] := f3_x2 = $D3d_FW(f3,x2); 
   [1:Nx1][2:Nx2-1][1:Nx3] := f3_x2 = $D3d_0(f3,x2); 
   [1:Nx1][Nx2:Nx2][1:Nx3] := f3_x2 = $D3d_BW(f3,x2); 
}
residual f3_x3 
{ 
   [1:Nx1][1:Nx2][1:1] := f3_x3 = $D3d_FW(f3,x3); 
   [1:Nx1][1:Nx2][2:Nx3-1] := f3_x3 = $D3d_0(f3,x3); 
   [1:Nx1][1:Nx2][Nx3:Nx3] := f3_x3 = $D3d_BW(f3,x3); 
}
residual f3_x1_CN
{
   [1:Nx1-1][1:Nx2][1:Nx3] := f3_x1_CN = $D3d_0_CN(f3,x1);
   [Nx1:Nx1][1:Nx2][1:Nx3] := <0>f3_x1_CN[0][0][0] = <0>f3_x1_CN[-1][0][0];
}
residual f3_x2_CN
{
   [1:Nx1][1:Nx2-1][1:Nx3] := f3_x2_CN = $D3d_0_CN(f3,x2);
   [1:Nx1][Nx2:Nx2][1:Nx3] := <0>f3_x2_CN[0][0][0] = <0>f3_x2_CN[0][-1][0];
}    
residual f3_x3_CN
{
   [1:Nx1][1:Nx2][1:Nx3-1] := f3_x3_CN = $D3d_0_CN(f3,x3);
   [1:Nx1][1:Nx2][Nx3:Nx3] := <0>f3_x3_CN[0][0][0] = <0>f3_x3_CN[0][0][-1];
}    

auto update f3_x1
auto update f3_x2
auto update f3_x3
auto update f3_x1_CN
auto update f3_x2_CN
auto update f3_x3_CN

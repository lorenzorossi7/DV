########################################################################
# single grid, masked difference operators for DV
########################################################################
system parameter int memsiz := 10000000

rec1 coordinates t,x1

uniform rec1 grid g1 [1:Nx1] {x1min:x1max}

float f1 on g1
float mask1 on g1

float f1_x1 on g1
float f1_x1_CN on g1
parameter float mask_val  :=  1.0

operator D1d_0(f,x1) := (<0>f[1]-<0>f[-1])/(2*dx1)
operator D1d_0_CN(f,x1) := (<0>f[1]-<0>f[0])/(dx1)
operator ZERO(f,x1) := (0)
operator D1d_BW(f,x1) := (3*<0>f[0]-4*<0>f[-1]+<0>f[-2])/(2*dx1)
operator D1d_FW(f,x1) := (-3*<0>f[0]+4*<0>f[1]-<0>f[2])/(2*dx1)

#=======================================================================
# directives for excision pre-processor
#=======================================================================
#%define mask(mask1;mask_val;[0])

#%define masked_op(D1d_0;x1;[-1]->D1d_FW,[1]->D1d_BW)
#%define masked_op(D1d_0_CN;x1;[1]->ZERO)
#%define masked_op(D1d_FW;x1;)
#%define masked_op(D1d_BW;x1;)

#-----------------------------------------------------------------------
residual f1_x1 
{ 
   [1:1] := f1_x1 = $D1d_FW(f1,x1); 
   [2:Nx1-1] := f1_x1 = $D1d_0(f1,x1); 
   [Nx1:Nx1] := f1_x1 = $D1d_BW(f1,x1); 
}

residual f1_x1_CN
{
   [1:Nx1-1] := f1_x1_CN = $D1d_0_CN(f1,x1);
   [Nx1:Nx1] := <0>f1_x1_CN[0] = <0>f1_x1_CN[-1];
}

auto update f1_x1
auto update f1_x1_CN

c======================================================================
c 2D cluster routines
c======================================================================

c======================================================================
c The following is a 2D interface to the 3D routine obtained from
c Dale Choi's web page:
c http://einstein.drexel.edu/~dale/public_html/clusterer/cls.html
c (in dc_clusterer.f)
c
c chr(Nx,Ny) : the (INTEGER!) characteristic function (0 or 1) identifying 
c            points flagged for refinement
c 
c cls(6*max_cls) : an output array listing the set of clusters found,
c                via coordinate bounding boxes [(i1,j1,i2,j2),(),()...]
c                NOTE: 6*max_cls of space is required, as dc_clusterer.f
c                      works with 3D grids
c
c no_cls : is set to the number of clusters found
c
c max_cls : maximum number of clusters 
c
c min_width,buf_wid,min_eff,blk_fac: dc_clusterer.f parameters
c
c work : integer array of size 6*(max_cls)
c 
c======================================================================
        subroutine dc_cls_2d(chr,cls,no_cls,max_cls,minwidth,buf_wid,
     &     min_eff,blk_fac,work,Nx,Ny)
        implicit none
        integer Nx,Ny,minwidth,buf_wid,no_cls,max_cls,blk_fac
        integer work(6*(max_cls)),cls(6*max_cls)
        integer chr(Nx,Ny)
        real*8 min_eff

        integer lb(3),ub(3),box_stk_lb,box_stk_ub
        integer i,i1,i2,j1,j2,shape1,shape2,shape3

        logical ltrace
        parameter (ltrace=.false.)

        lb(1)=1
        lb(2)=1
        lb(3)=1
        ub(1)=Nx
        ub(2)=Ny
        ub(3)=1
        shape1=Nx
        shape2=Ny
        shape3=1
        box_stk_lb=1
        box_stk_ub=box_stk_lb+3*max_cls

        call dagh_cls_3d(chr,lb,ub,shape1,shape2,shape3,
     &       minwidth,buf_wid,max_cls,
     &       no_cls,work(box_stk_lb),work(box_stk_ub),min_eff,blk_fac)

        if (ltrace) write(*,*) 
     &      'dc_cls_2d:',no_cls,' clusters returned by dagh_cls_3d:'
        do i=1,min(no_cls,max_cls)
           i1=work(box_stk_lb+(i-1)*3)
           j1=work(box_stk_lb+(i-1)*3+1)
           i2=work(box_stk_ub+(i-1)*3)
           j2=work(box_stk_ub+(i-1)*3+1)
           cls((i-1)*4+1)=i1
           cls((i-1)*4+2)=j1
           cls((i-1)*4+3)=i2
           cls((i-1)*4+4)=j2
           if (ltrace) then
              write(*,87) i,i1,j1,i2,j2
87            format(1i4,' [(',1i5,',',1i5,')(',1i5,',',1i5,')]')
           end if
        end do

        return
        end
           

        



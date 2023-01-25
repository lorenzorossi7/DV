
c-----------------------------------------------------------------------
c
! FP changed all write(8,*) to write(*,*)
c
c
c  Creation date: Wed Jul 13 09:39:26 CDT 1994
c    Written by Reid Guenther and Mijan Huq.
c   Modified by Dale Choi ( Aug. 1996 ) 
c
c   Ref: Berger and Rigoutsos, IEEE Trans. Vol. 21, No. 5, 1991.
c
c
c Routine to carry out Clustering in 3d ad using 
c
c
c NX/Y/Z  := Size of the box.
c box_max := Maximum number of boxes.
c min_eff := Threshold value for efficiency. (Real*8)
c box     := 3D integer Array containing characteristic function. 
c            Size NX*NY*NZ.
c box_stk := Integer Stack containing box characterstics. Size 6*box_max
c             box_stk(1,i) = x coordinate of lower most corner of box i
c             box_stk(2,i) = y coordinate of lower most corner of box i
c             box_stk(3,i) = z coordinate of lower most corner of box i
c             box_stk(4,i) = x coordinate of upper most corner of box i
c             box_stk(5,i) = y coordinate of upper most corner of box i
c             box_stk(6,i) = z coordinate of upper most corner of box i
c wksp    := 1D integer Work Array. Size 2*NX + 2*NY + 2*NZ
c
c-----------------------------------------------------------------------
c
c this routine is a DAGH wrapper for the cluster wrap routine below.
c
c
c FP --- changed shape(3) -> shape1, shape2, shape3
c for compatability with the local f77 compiler
c-----------------------------------------------------------------------
c(8/25/96) added blk_fac and changed blk_wid to minwidth
c    (currently, blk_fac := 2)
      subroutine dagh_cls_3d(box, lb, ub, shape1, shape2, shape3,
     &                    minwidth, buf_wid,    
     &                    box_max, box_no, box_stk_lb, box_stk_ub,    
     &                    min_eff, blk_fac)

      implicit none

! input variables
         integer    lb(3),ub(3),shape1,shape2,shape3
         integer    box(shape1,shape2,shape3)
         integer    minwidth, buf_wid, box_max, blk_fac
         real*8     min_eff

! output variables
         integer    box_no
         integer    box_stk_lb(3,box_max), box_stk_ub(3,box_max)

! routine variables
         integer    buf_box(shape1,shape2,shape3),shape(3)
         integer    clstr, nx, ny, nz

         integer    box_stk_max
         parameter (box_stk_max = 1000)

         integer    box_stk(6,box_stk_max)
         integer    nmax_wksp
         parameter (nmax_wksp = 6 000)
         integer    wksp(nmax_wksp)

         integer    stride_x, stride_y, stride_z

         integer    box_pt, i, j, k, l, m, n 
         integer    num_flag, totalpt, buf_num_flag,
     &              interpt,  boxbdry, totbdry, bboxinte,
     &              inf2,inf3,inf4,inf5,inf6,inf7,inf8,
     &              totalpt2
         real*8     overall_eff

         logical    ltrace
         parameter (ltrace = .false.)


         open(unit=11,file='cls.flow',status='unknown')

         shape(1)=shape1
         shape(2)=shape2
         shape(3)=shape3

         if (ltrace)
     &     write(6,*) 'minwidth, buf_wid, box_max, blk_fac = '
     &               ,minwidth, buf_wid, box_max, blk_fac

         if ( ltrace ) then
          do i=1,shape(1)
            write(*,*) 'i=',i
            do j=1,shape(2)
              write(*,*) 'j=',j
              do k=1,shape(3)
                write(*,*) 'k=',k,' box(',i,',',j,',',k,')= ',box(i,j,k)
              enddo
            enddo
          enddo
         endif


         nx = shape(1)
         ny = shape(2)
         nz = shape(3)
         if (6*max(nx,ny,nz).gt.nmax_wksp) then
            write(*,*)'DAGH_CLS>>> Warning 6*max(shape(*)) is ',     
     &                ' greater than nmax_wksp ',nmax_wksp
            write(*,*)'DAGH_CLS>>> shape(1) ',shape(1)
            write(*,*)'DAGH_CLS>>> shape(2) ',shape(2)
            write(*,*)'DAGH_CLS>>> shape(3) ',shape(3)
            box_no = 0
            return
         end if

         if (box_max .gt. box_stk_max) then
            write(*,*)'DAGH_CLS>>> Warning box_stk ',box_stk,      
     &                ' greater than box_stk_max ',box_stk_max
            return
         end if

c-----------------------------------------------------------------------
c Check for all_zero characteristic input
c If characteristic input is identically zero, no box should be returned
c-----------------------------------------------------------------------
         num_flag = 0
         do i=1,nx
            do j=1,ny
               do k=1,nz
                  num_flag = num_flag + box(i,j,k)
               enddo
            enddo
         enddo

         if (ltrace) write(11,*) 'Reid: num_flag(no buf) = ',num_flag

         if ( num_flag .eq. 0 ) then
            if (ltrace) 
     &         write(6,*) 'No points are flagged/No boxes returned'
             box_no = 0
              do i = 1, box_no
                 do j = 1,3
                    box_stk_lb(j,i) = 0
                    box_stk_ub(j,i) = 0
                 enddo
              enddo
            return
         endif

c-----------------------------------------------------------------------
c Buffering -- Flag neighboring points before clustering
c              (do not enlarge grids after clustering)
c-----------------------------------------------------------------------

c Initialize box_stk
         do i = 1, box_stk_max
            do j = 1,6
               box_stk(j,i) = 0
            enddo
         enddo

c     initialize buf_box
         do l = 1, nx
            do m = 1, ny
               do n = 1, nz
                  buf_box(l,m,n) = 0
               enddo
            enddo 
         enddo

c     flagging neighboring points
         do i=1,nx
           do j=1,ny
             do k=1,nz
              
               if( box(i,j,k) .eq. 1 ) then
                  do l = i-buf_wid, i+buf_wid
                    do m = j-buf_wid, j+buf_wid
                      do n = k-buf_wid, k+buf_wid 
                         if( l .le. nx .and. l .ge. 1 
     &                 .and. m .le. ny .and. m .ge. 1
     &                 .and. n .le. nz .and. n .ge. 1 ) then
                             buf_box(l,m,n) = 1 
                         endif
                      enddo
                    enddo
                  enddo
               endif

             enddo
           enddo
         enddo

c     copy buf_box to box(which will be given to clusterer)
c      and zero buf_box(which will be used later)
         do i=1,nx
           do j=1,ny
             do k=1,nz
              box(i,j,k) = buf_box(i,j,k)
              buf_box(i,j,k) = 0
             enddo
           enddo                                            
         enddo

c     counting number of buffered flagged points 
         buf_num_flag = 0
         do i=1,nx
            do j=1,ny
               do k=1,nz
                  buf_num_flag = buf_num_flag + box(i,j,k)
               enddo
            enddo
         enddo
         if (ltrace) write(11,*) 'Reid: num_flag(buf) = ', buf_num_flag


c-----------------------------------------------------------------------
c Calling main clusterer
c-----------------------------------------------------------------------
         box_no = clstr(nx,ny,nz,box_max,minwidth,                   
     &                  box,box_stk,wksp,min_eff,blk_fac)


c-----------------------------------------------------------------------
c Diagnostics
c-----------------------------------------------------------------------
c Compute overall efficiency
c a right way of counting total points in the bboxes
c ( didn't count interface points twice, though )
c use buf_box to compute number of interface points

         boxbdry = 0
         totbdry = 0
         bboxinte= 0

         do box_pt = 1, box_no
            do i = box_stk(1,box_pt), box_stk(4,box_pt)
               do j = box_stk(2,box_pt), box_stk(5,box_pt)
                  do k = box_stk(3,box_pt), box_stk(6,box_pt)
                     box(i,j,k) = 1
                     if ( buf_box(i,j,k) .eq. 0 ) then
                          buf_box(i,j,k) = 1
                     else if ( buf_box(i,j,k) .eq. 1 ) then
                          buf_box(i,j,k) = 2
                     else if ( buf_box(i,j,k) .eq. 2 ) then
                          buf_box(i,j,k) = 3
                     else if ( buf_box(i,j,k) .eq. 3 ) then
                          buf_box(i,j,k) = 4
                     else if ( buf_box(i,j,k) .eq. 4 ) then
                          buf_box(i,j,k) = 5
                     else if ( buf_box(i,j,k) .eq. 5 ) then
                          buf_box(i,j,k) = 6
                     else if ( buf_box(i,j,k) .eq. 6 ) then
                          buf_box(i,j,k) = 7
                     else if ( buf_box(i,j,k) .eq. 7 ) then
                          buf_box(i,j,k) = 8
                     endif
                  end do
               end do
            end do

            bboxinte=  bboxinte + 
     &               ( box_stk(4,box_pt) - box_stk(1,box_pt) - 1 )
     &              *( box_stk(5,box_pt) - box_stk(2,box_pt) - 1 )
     &              *( box_stk(6,box_pt) - box_stk(3,box_pt) - 1 )

c   boxbdry counts interface twice
            boxbdry =  boxbdry + 8 + 4*
     &               ( box_stk(4,box_pt) - box_stk(1,box_pt) - 1
     &               + box_stk(5,box_pt) - box_stk(2,box_pt) - 1
     &               + box_stk(6,box_pt) - box_stk(3,box_pt) - 1 )  
     &             + 2*(box_stk(4,box_pt) - box_stk(1,box_pt) - 1)
     &                *(box_stk(5,box_pt) - box_stk(2,box_pt) - 1)
     &             + 2*(box_stk(5,box_pt) - box_stk(2,box_pt) - 1)
     &                *(box_stk(6,box_pt) - box_stk(3,box_pt) - 1)
     &             + 2*(box_stk(4,box_pt) - box_stk(1,box_pt) - 1)
     &                *(box_stk(6,box_pt) - box_stk(3,box_pt) - 1)
         end do

         totalpt = 0
         interpt = 0
         inf2 = 0
         inf3 = 0
         inf4 = 0
         inf5 = 0
         inf6 = 0
         inf7 = 0
         inf8 = 0 
         totalpt2 = 0
         do i = 1, nx
            do j = 1, ny
               do k = 1, nz
                  if( box(i,j,k) .eq. 1 ) then
                      totalpt = totalpt + 1
                  endif
                  if( buf_box(i,j,k) .ge. 1 ) then
                      totalpt2 = totalpt2 + 1
                  endif
                  if( buf_box(i,j,k) .ge. 2 ) then
                      interpt = interpt + 1
                  endif 
                  if( buf_box(i,j,k) .eq. 2 ) then
                      inf2 = inf2 + 1
                  else if ( buf_box(i,j,k) .eq. 3 ) then
                      inf3 = inf3 + 1
                  else if ( buf_box(i,j,k) .eq. 4 ) then
                      inf4 = inf4 + 1
                  else if ( buf_box(i,j,k) .eq. 5 ) then
                      inf5 = inf5 + 1
                  else if ( buf_box(i,j,k) .eq. 6 ) then
                      inf6 = inf6 + 1
                  else if ( buf_box(i,j,k) .eq. 7 ) then
                      inf7 = inf7 + 1
                  else if ( buf_box(i,j,k) .eq. 8 ) then
                      inf8 = inf8 + 1
                  endif
               enddo  
            enddo
         enddo

         overall_eff = real(num_flag) / real(totalpt)

         totbdry = boxbdry - 2*inf2 - 3*inf3 - 4*inf4 - 5*inf5 
     &                     - 6*inf6 - 7*inf7 - 8*inf8

         if (ltrace) then 
         write(11,*) 'Reid: no. of flagged points(no buf)= ',num_flag
         write(11,*) 'Reid: totalpt(buf)                 = ',totalpt
         write(11,*) 'Reid: overall_eff = ',overall_eff
         write(11,*) 'Reid: box_no(buf)                  = ',box_no
         write(11,*) 'Reid: '
         write(11,*) 'Reid: total boxbdry pts(multicnt)  = ',boxbdry
         write(11,*) 'Reid: no. of interface points      <=',interpt
         write(11,*) 'Reid: total real bdry points       >=',totbdry
         write(11,*) 'Reid: total bdry points cnted once = ',
     &                                                interpt+totbdry
         write(11,*) 'Reid: total bbox interior points   = ',bboxinte
         write(11,*) 'Reid: totalpt2(buf)                = ',totalpt2
         write(11,*) 'Reid: '
         write(11,*) 'Reid: interface points bet. 2 grid = ',inf2
         write(11,*) 'Reid: interface points bet. 3 grid = ',inf3
         write(11,*) 'Reid: interface points bet. 4 grid = ',inf4
         write(11,*) 'Reid: interface points bet. 5 grid = ',inf5
         write(11,*) 'Reid: interface points bet. 6 grid = ',inf6
         write(11,*) 'Reid: interface points bet. 7 grid = ',inf7
         write(11,*) 'Reid: interface points bet. 8 grid = ',inf8
         end if


c-----------------------------------------------------------------------
c   now define box_stk_lb and box_stk_ub in terms of global coordinates
c   defined by lb and ub and shape
c-----------------------------------------------------------------------
         stride_x = (ub(1) - lb(1))/(shape(1) - 1)
         stride_y = (ub(2) - lb(2))/(shape(2) - 1)
c-----------------------------------------------------------------------
c FP --- modified to work with 2D grids
c-----------------------------------------------------------------------
         if (shape(3).gt.1) then
            stride_z = (ub(3) - lb(3))/(shape(3) - 1)
         else
            stride_z = 0 
         end if

         do box_pt = 1, box_no
            box_stk_lb(1,box_pt) = lb(1) +                           
     &                            (box_stk(1,box_pt)-1)*stride_x
            box_stk_lb(2,box_pt) = lb(2) +                          
     &                            (box_stk(2,box_pt)-1)*stride_y
            box_stk_lb(3,box_pt) = lb(3) +                           
     &                            (box_stk(3,box_pt)-1)*stride_z

            box_stk_ub(1,box_pt) = lb(1) +                        
     &                            (box_stk(4,box_pt)-1)*stride_x
            box_stk_ub(2,box_pt) = lb(2) +                         
     &                            (box_stk(5,box_pt)-1)*stride_y
            box_stk_ub(3,box_pt) = lb(3) +                           
     &                            (box_stk(6,box_pt)-1)*stride_z
         end do


      return
      end
c END of main routine

!-----------------------------------------------------------------------
!
!  this routine is a wrapper for the clustering algorithm and sets
!   up fortran workspace.
!
!-----------------------------------------------------------------------
      integer function clstr(nx,ny,nz,box_max,minwidth,                
     &                       box,box_stk,wksp,min_eff,blk_fac)

         implicit none

         integer  cmp_clstr

         integer  nx, ny, nz, box_max, minwidth, blk_fac
         integer  box_stk(6,box_max), wksp(2*nx+2*ny+2*nz)
         real*8   min_eff
         integer  box(nx,ny,nz)
         integer  i,j

         logical trace
         parameter (trace=.false.)

!
! Memory allocation of : sigx, sigy, lplx, lply.
!
         integer sigx, sigy, sigz, lplx, lply, lplz

         sigx = 1
         sigy = nx + 1
         sigz = nx + ny + 1
         lplx = nx + ny + nz + 1
         lply = 2*nx + ny + nz + 1
         lplz = 2*nx + 2*ny + nz + 1


         if (trace) write(6,*) 'I am in function clstr'

         if (trace) then
            write(*,*)'nx =',nx
            write(*,*)'ny =',ny
            write(*,*)'nz =',nz
            write(*,*)'box_max =',box_max
            write(*,*)'sigx =',sigx
            write(*,*)'sigy =',sigy
            write(*,*)'sigz =',sigz
            write(*,*)'lplx =',lplx
            write(*,*)'lply =',lply
            write(*,*)'lplz =',lplz
            write(*,*)'min_eff =',min_eff
c            write(*,*)'box(1,1,1):',box(1,1,1)
c            write(*,*)'box(11,1,1):',box(11,1,1)
c            write(*,*)'box(4,7,9):',box(4,7,9)
c            write(*,*)'box(6,10,12):',box(6,10,12)
c            write(*,*)'box(3,6,4):',box(3,6,4)
c            write(*,*)'box(2,13,5):',box(2,13,5)
c            write(*,*)'box(7,7,7):',box(7,7,7)
c            write(*,*)'box(1,11,1):',box(1,11,1)
c            write(*,*)'box(1,1,11):',box(1,1,11)
c            write(*,*)'box(nx,ny,nz):',box(nx,ny,nz)
         end if

!
!  Call the cluster function which returns the number of boxs found.
!
         clstr = cmp_clstr(nx,ny,nz,box_max,minwidth, box, box_stk,   
     &                     wksp(sigx),wksp(sigy),wksp(sigz),              
     &                     wksp(lplx),wksp(lply),wksp(lplz),
     &                     min_eff,blk_fac)

         return

      end


!-----------------------------------------------------------------------
!
! Internal cluster routine to compute boxs as described by the
!  algorithm given in the Berger & Rigoutsos paper (IEEE Transactions on
!  systems, man, and cybernetics. VOL 21, NO. 5, Sept/Oct 1991).
!
!-----------------------------------------------------------------------
      integer function cmp_clstr(nx,ny,nz,box_max,minwidth,box,box_stk, 
     &                  sigx,sigy,sigz,lplx, lply,lplz, min_eff,blk_fac)

         implicit none

         integer  nx, ny, nz, box_max, minwidth, blk_fac
         integer  box_stk(6,box_max)
         integer  sigx(nx),sigy(ny),sigz(nz),lplx(nx),lply(ny),lplz(nz)
         real*8   min_eff
         integer  box(nx,ny,nz)

c  local variables
         integer  i, flag, no_box, j, ind, x_wd, y_wd, z_wd, k
         integer  l,m,n
         real*8   box_eff
         real*8   cur_box_eff
         character*20 fname

         logical     trace,            trace_gr
         parameter ( trace = .false. , trace_gr = .false. )


         call InitStack(box_stk, 1, 1, 1, nx, ny, nz, no_box, box_max)
         i=1
 100     continue
            cur_box_eff = box_eff(box, box_stk(1,i), nx, ny, nz)
                if (trace) then
                   write(*,*)'after computing cur_box_eff'
                   write(*,*)'i =',i
                   write(*,*)'cur_box_eff ',cur_box_eff
                   write(*,*)'min_eff ',min_eff
                   write(*,*)'box_stk(1,i) ',box_stk(1,i)
                   write(*,*)'box_stk(2,i) ',box_stk(2,i)
                   write(*,*)'box_stk(3,i) ',box_stk(3,i)
                   write(*,*)'box_stk(4,i) ',box_stk(4,i)
                   write(*,*)'box_stk(5,i) ',box_stk(5,i)
                   write(*,*)'box_stk(6,i) ',box_stk(6,i)
c                   write(*,*)'box(1,1,1):',box(1,1,1)
c                   write(*,*)'box(5,5,5):',box(5,5,5)
c                   write(*,*)'box(11,1,1):',box(11,1,1)
c                   write(*,*)'box(1,11,1):',box(1,11,1)
c                   write(*,*)'box(1,1,11):',box(1,1,11)
c                   write(*,*)'box(nx,ny,nz):',box(nx,ny,nz)
                end if
c
c  If the efficiency of the current box is less than the threshold 
c  try to improve by splitting.
c
            if ( cur_box_eff .lt. min_eff ) then

               call cmp_sig_lpl(box,sigx,sigy,sigz,                
     &                      lplx,lply,lplz,box_stk(1,i),nx,ny,nz,i)

               if (trace) then
                  write(*,*)'i =',i
                  write(*,*)'cur_box_eff =',cur_box_eff
                  write(*,*)'nx_lo =',box_stk(1,i)
                  write(*,*)'ny_lo =',box_stk(2,i)
                  write(*,*)'nz_lo =',box_stk(3,i)
                  write(*,*)'nx_hi =',box_stk(4,i)
                  write(*,*)'ny_hi =',box_stk(5,i)
                  write(*,*)'nz_hi =',box_stk(6,i)
                  do k = box_stk(1,i),box_stk(4,i)
                     write(6,10) 'sigx(',k,')=',sigx(k),          
     &                          ' lplx(',k,')=',lplx(k)
                  enddo
                  do j = box_stk(2,i),box_stk(5,i)
                     write(6,10) 'sigy(',j,')=',sigy(j),           
     &                          ' lply(',j,')=',lply(j)
                  end do
               end if

               flag = 0
               call find_split(nx,ny,nz,no_box,i,minwidth,         
     &                        box_stk,                           
     &                        sigx, sigy, sigz, lplx, lply,       
     &                        lplz, flag, box_max, blk_fac)

               if (flag.eq.0) then
                   i = i + 1
               end if

            else
               i = i + 1
            end if

!
!  keep trying to divide the inefficient boxes until done or
!   run out of space in the stack
!
         if (i .le. no_box .and. no_box .lt. box_max) then
            goto 100
         else if (i .le. box_max) then

c---------------------------------------------------------------
c Later: we might want to add routine to visit low-efficient box
c        again and do something to improve efficiency
c        merging and/or bisection
c---------------------------------------------------------------
c  NOT a good algorithm below(cause segmentation violation)
c           do j = i,box_max
c              call cmp_sig_lpl(box,sigx,sigy,sigz,                  
c    &                      lplx,lply,lplz,box_stk(1,j),nx,ny,nz,j)
c              call reduce(j,box_stk(1,j),minwidth,sigx,sigy,sigz
c    &                    ,nx,ny,nz, blk_fac)
c           end do
c  up to here---------------------------------------------------

         end if

         cmp_clstr = no_box

 
! this is needed to conform to Manish Parishar's demand for boxes
! with integer widths of blk_wd.  The strange form of the if statement
! occurs because the smooshing of the boxes finds the correct leftmost
! boundaries of the box and they do not need to be moved by one point to
! the left.
c--------------------------------------------------------
c I don't understand this! this blocking factor stuff!
c        if (blk_wd.gt.1) then
c           do i = 1,no_box
c              if (mod(box_stk(1,i)-2,blk_wd).eq.0.and.              
c    &             box_stk(1,i).ne.1) then
c                 box_stk(1,i) = box_stk(1,i) -1
c              end if
c              if (mod(box_stk(2,i)-2,blk_wd).eq.0.and.               
c    &             box_stk(2,i).ne.1) then
c                 box_stk(2,i) = box_stk(2,i) -1
c              end if
c              if (mod(box_stk(3,i)-2,blk_wd).eq.0.and.              
c    &             box_stk(3,i).ne.1) then
c                 box_stk(3,i) = box_stk(3,i) -1
c              end if
c           end do
c        end if
c--------------------------------------------------------

      return
 10   format(a,i3,a,i6,a,i3,a,i8)
 20   format(i4,f10.4,i6,8i4)
      end

!-----------------------------------------------------------------------
!
! subroutine that computes the signatures and laplacians of the given box
!
!-----------------------------------------------------------------------
      subroutine cmp_sig_lpl(box, sigx, sigy, sigz, lplx, lply,     
     &                       lplz, box_stk, nx, ny, nz, a)

      implicit none

         integer nx, ny, nz
         integer sigx(nx),sigy(ny),sigz(nz),box_stk(6)
         integer lplx(nx),lply(ny),lplz(nz)
         integer  box(nx,ny,nz)

         integer i, j, k,x_lo, y_lo, z_lo, x_hi, y_hi, z_hi, tmpl, tmpr
         integer a

         logical     trace
         parameter ( trace = .false. )


         call l_ivls(sigx,0,nx)
         call l_ivls(sigy,0,ny)
         call l_ivls(sigz,0,nz)
         x_lo = box_stk(1)
         y_lo = box_stk(2)
         z_lo = box_stk(3)
         x_hi = box_stk(4)
         y_hi = box_stk(5)
         z_hi = box_stk(6)

         if (trace) then
            write(*,*)'I am in cmp_sig_lpl'
            write(*,*)'a= ',a
            write(*,*)'nx ',nx
            write(*,*)'ny ',ny
            write(*,*)'nz ',nz
            write(*,*)'x_lo ',x_lo
            write(*,*)'y_lo ',y_lo
            write(*,*)'z_lo ',z_lo
            write(*,*)'x_hi ',x_hi
            write(*,*)'y_hi ',y_hi
            write(*,*)'z_hi ',z_hi
         end if

         do i = x_lo, x_hi
            do j = y_lo, y_hi
               do k = z_lo, z_hi
                  sigx(i) = sigx(i) + box(i,j,k)
               end do
            end do
         end do

         do j = y_lo, y_hi
            do i = x_lo, x_hi
               do k = z_lo, z_hi
                  sigy(j) = sigy(j) + box(i,j,k)
               end do
            end do
         end do

         do k = z_lo, z_hi
            do j = y_lo, y_hi
               do i = x_lo, x_hi
                  sigz(k) = sigz(k) + box(i,j,k)
               end do
            end do
         end do

! make sure outer laplacians are always zero otherwise the (i/j)+/-1
!  references will pollute the algorithm
!         lplx(x_lo) = 0
!         lplx(x_hi) = 0
!         lply(y_lo) = 0
!         lply(y_hi) = 0
!         lply(z_lo) = 0
!         lply(z_hi) = 0
!         do i = x_lo+1,x_hi -1
!            lplx(i) = sigx(i+1)-2*sigx(i)+sigx(i-1)
!         end do
!
!         do j = y_lo+1,y_hi -1
!            lply(j) = sigy(j+1)-2*sigy(j)+sigy(j-1)
!         end do
!
!         do j = z_lo+1,z_hi -1
!            lplz(j) = sigz(j+1)-2*sigz(j)+sigz(j-1)
!         end do


! compute laplacians by the difference of 
! the absolute value of the gradients
!  - this should take care of some of 
! the problems of using the above laplacian.
         call l_ivls(lplx,0,nx)
         call l_ivls(lply,0,ny)
         call l_ivls(lplz,0,nz)

         do i = x_lo, x_hi-1
            do j = y_lo, y_hi
               do k = z_lo, z_hi
                  lplx(i) = lplx(i) + abs(box(i,j,k)-box(i+1,j,k))
               end do
            end do
         end do

         do j = y_lo, y_hi-1
            do i = x_lo, x_hi
               do k = z_lo, z_hi
                  lply(j) = lply(j) + abs(box(i,j,k)-box(i,j+1,k))
               end do
            end do
         end do

         do k = z_lo, z_hi-1
            do j = y_lo, y_hi
               do i = x_lo, x_hi
                  lplz(k) = lplz(k) + abs(box(i,j,k)-box(i,j,k+1))
               end do
            end do
         end do

         do i = x_hi-1,x_lo+1,-1
            lplx(i) = lplx(i)-lplx(i-1)
         end do

         do j = y_hi-1,y_lo+1,-1
            lply(j) = lply(j)-lply(j-1)
         end do

         do k = z_hi-1,z_lo+1,-1
            lplz(k) = lplz(k)-lplz(k-1)
         end do


         lplx(x_lo) = 0
         lply(y_lo) = 0
         lplz(z_lo) = 0
         lplx(x_hi) = 0
         lply(y_hi) = 0
         lplz(z_hi) = 0

      return
      end

!-----------------------------------------------------------------------
!
!  subroutine that computes the efficiency of the given box
!   box_eff = (number of flagged mesh points in the box)/
!              (total number of mesh points in the box)
!
!-----------------------------------------------------------------------
      double precision function box_eff(box, box_stk, nx, ny, nz)

      implicit none

         integer nx, ny, nz
         integer box_stk(6)
         integer box(nx,ny,nz)

         integer flg_pts, i, j, k, x_lo, y_lo, z_lo, x_hi, y_hi, z_hi

         x_lo = box_stk(1)
         y_lo = box_stk(2)
         z_lo = box_stk(3)
         x_hi = box_stk(4)
         y_hi = box_stk(5)
         z_hi = box_stk(6)

         flg_pts = 0

         do i = x_lo, x_hi
            do j = y_lo, y_hi
               do k = z_lo, z_hi
                  flg_pts = flg_pts + (box(i,j,k))
               end do
            end do
         end do

         box_eff = 1.0d0*flg_pts/                                
     &               (x_hi-x_lo+1.0d0)/                           
     &               (y_hi-y_lo+1.0d0)/(z_hi-z_lo+1.0d0)

      return
      end

!-----------------------------------------------------------------------
!
! Subroutine that tries to reduce the size of the box if there are empty
!  regions along the *sides* of the boxes.
!
!-----------------------------------------------------------------------
      subroutine reduce(stkid,box_stk,minwidth,sigx,sigy,sigz,nx,ny,nz,
     &                  blk_fac)

      implicit none

         integer stkid, minwidth, nx, ny, nz, blk_fac
         integer box_stk(6), sigx(nx), sigy(ny), sigz(nz)
         integer i, j, k, nx_lo, nx_hi, ny_lo, ny_hi, nz_lo, nz_hi

         logical     trace
         parameter ( trace = .false. )

         nx_lo = box_stk(1)
         ny_lo = box_stk(2)
         nz_lo = box_stk(3)
         nx_hi = box_stk(4)
         ny_hi = box_stk(5)
         nz_hi = box_stk(6)

         if (trace) then
            i = stkid
            write(*,*)'REDUCE>> before running'
            write(*,*)'stkid =',i
            write(*,*)'nx_lo =',box_stk(1)
            write(*,*)'ny_lo =',box_stk(2)
            write(*,*)'nz_lo =',box_stk(3)
            write(*,*)'nx_hi =',box_stk(4)
            write(*,*)'ny_hi =',box_stk(5)
            write(*,*)'nz_hi =',box_stk(6)
         end if

c I still don't understand "i.ne.1" part!

         i = nx_lo
 11      if (sigx(i).eq.0.and.(nx_hi-nx_lo).gt.minwidth) then
            if (mod(i,blk_fac).eq.0.and.i.ne.1) then
               nx_lo = i + 1
            end if
            i = i + 1
            goto 11
         end if

         i = nx_hi
 16      if (sigx(i).eq.0.and.(nx_hi-nx_lo).gt.minwidth) then
            if (mod(i-2,blk_fac).eq.0) then
               nx_hi = i - 1
            end if
            i = i - 1
            goto 16
         end if

         j = ny_lo
  21     if (sigy(j).eq.0.and.(ny_hi-ny_lo).gt.minwidth) then
            if (mod(j,blk_fac).eq.0.and.j.ne.1) then
               ny_lo = j + 1
            end if
            j = j + 1
            goto 21
         end if

         j = ny_hi
  26     if (sigy(j).eq.0.and.(ny_hi-ny_lo).gt.minwidth) then
            if (mod(j-2,blk_fac).eq.0) then
               ny_hi = j - 1
            end if
            j = j - 1
            goto 26
         end if

         k = nz_lo
  31     if (sigz(k).eq.0.and.(nz_hi-nz_lo).gt.minwidth) then
            if (mod(k,blk_fac).eq.0.and.k.ne.1) then
               nz_lo = k + 1
            end if
            k = k + 1
            goto 31
         end if

         k = nz_hi
  36     if (sigz(k).eq.0.and.(nz_hi-nz_lo).gt.minwidth) then
            if (mod(k-2,blk_fac).eq.0) then
               nz_hi = k - 1
            end if
            k = k - 1
            goto 36
         end if

         box_stk(1) = nx_lo
         box_stk(2) = ny_lo
         box_stk(3) = nz_lo
         box_stk(4) = nx_hi
         box_stk(5) = ny_hi
         box_stk(6) = nz_hi

      return
      end


!-----------------------------------------------------------------------
!
!  This routine tries to split the inefficient box first by 
!   seeing if there are any zeros in the signatures (lines in the
!   box mesh with no flagged points) and if that does not
!   work then it computes the laplacian of the signatures and looks
!   for inflection points (i.e. where the laplacian goes to zero).
!
!-----------------------------------------------------------------------
      subroutine find_split(nx, ny, nz, no_box, stkid, minwidth,               
     &                     box_stk,                                     
     &                     sigx, sigy, sigz, lplx, lply, lplz,          
     &                     flag, box_max, blk_fac)

      implicit none

         integer Find_Holes, Find_Inflx
         integer no_box, nx, ny, nz, stkid, minwidth, blk_fac
         integer flag, box_max
         integer    sigx(nx), sigy(ny), sigz(nz)
         integer lplx(nx), lply(ny), lplz(nz)
         integer box_stk(6,box_max)
         logical trace
         parameter (trace=.false.)

 
!       Look for Holes
!       Search for holes and split box and store in box_stk.
!       flag returns 1 if holes were found. 0 if not.
!       Routine has to add to no_box if splitting is done.
 
         flag = find_holes(no_box,stkid,minwidth,sigx,sigy,sigz,          
     &                     box_stk,box_max,blk_fac)

         if (trace) write(*,*) 'flag after Find_holes =',flag

 
!       If no holes are found then look for inflextion points.
!           Search for inflextion points and split if possible.
!         If splitting is possible add to stack. Return flag = 1
!         if splitting is carried out. else return 1
!       Routine has to add to no_box if splitting is done.
 
         if ( flag .eq. 0 )then
            flag = find_inflx(nx,ny,nz,no_box,stkid,minwidth,sigx,       
     &                  sigy,sigz,lplx,lply,lplz,box_stk,box_max,
     &                  blk_fac)
         end if

         if (trace) write(*,*) 'flag after Find_Inflx =',flag

      return
      end

!-----------------------------------------------------------------------
!
! Integer function that attempts to find a hole (zero in the signature
!  array) in the box mesh and if it does then it splits the
!  box into two at that hole
!
!-----------------------------------------------------------------------
      integer function find_holes(no_box, stkid, minwidth,                  
     &                  sigx, sigy, sigz, box_stk, box_max, blk_fac)

      implicit none

         integer no_box, stkid, box_max, minwidth, blk_fac
         integer sigx(*), sigy(*), sigz(*), box_stk(6,*)
         integer i, j, k, nx_lo, nx_hi, ny_lo, ny_hi, nz_lo, nz_hi

         logical     trace
         parameter ( trace = .false. )

         find_holes = 0
         nx_lo = box_stk(1,stkid)
         ny_lo = box_stk(2,stkid)
         nz_lo = box_stk(3,stkid)
         nx_hi = box_stk(4,stkid)
         ny_hi = box_stk(5,stkid)
         nz_hi = box_stk(6,stkid)

         if (trace) then
            i = stkid
            write(*,*)'Find_Holes before'
            write(*,*)'stkid =',i
            write(*,*)'nx_lo =',box_stk(1,i)
            write(*,*)'ny_lo =',box_stk(2,i)
            write(*,*)'nz_lo =',box_stk(3,i)
            write(*,*)'nx_hi =',box_stk(4,i)
            write(*,*)'ny_hi =',box_stk(5,i)
            write(*,*)'nz_hi =',box_stk(6,i)
            do j = box_stk(2,i),box_stk(5,i)
               write(*,*)'sigy(',j,')=',sigy(j)
            end do
         end if

c first see if there is any box that can be reduced
         i = nx_lo
 10      if ( sigx(i) .eq. 0 .and. (nx_hi-nx_lo) .gt. minwidth) then
            if ( mod(i,blk_fac) .eq. 0 .and. i .ne. 1 ) then
               nx_lo = i + 1
               find_holes = 1
            end if
            i = i + 1
            goto 10
         end if


         i = nx_hi
 15      if ( sigx(i) .eq. 0 .and. (nx_hi-nx_lo) .gt. minwidth) then
            if ( mod(i-2,blk_fac) .eq. 0 ) then
               nx_hi = i - 1
               find_holes = 1
            end if
            i = i - 1
            goto 15
         end if

         j = ny_lo
  20     if ( sigy(j) .eq. 0 .and. (ny_hi-ny_lo) .gt. minwidth) then
            if ( mod(j,blk_fac) .eq. 0 .and. j .ne. 1 ) then
               ny_lo = j + 1
               find_holes = 1
            end if
            j = j + 1
            goto 20
         end if

         j = ny_hi
  25     if ( sigy(j) .eq. 0 .and. (ny_hi-ny_lo) .gt. minwidth) then
            if ( mod(j-2,blk_fac) .eq. 0 ) then
               ny_hi = j - 1
               find_holes = 1
            end if
            j = j - 1
            goto 25
         end if

         k = nz_lo
  30     if ( sigz(k) .eq. 0 .and. (nz_hi-nz_lo) .gt. minwidth) then
            if ( mod(k,blk_fac) .eq. 0 .and. k .ne. 1 ) then
               nz_lo = k + 1
               find_holes = 1
            end if
            k = k + 1
            goto 30
         end if

         k = nz_hi
  35     if ( sigz(k) .eq. 0 .and. (nz_hi-nz_lo) .gt. minwidth ) then
            if ( mod(k-2,blk_fac) .eq. 0 ) then
               nz_hi = k - 1
               find_holes = 1
            end if
            k = k - 1
            goto 35
         end if

         box_stk(1,stkid) = nx_lo
         box_stk(2,stkid) = ny_lo
         box_stk(3,stkid) = nz_lo
         box_stk(4,stkid) = nx_hi
         box_stk(5,stkid) = ny_hi
         box_stk(6,stkid) = nz_hi


c secondly see if the current box can be divided by finding holes

         do i = nx_lo+minwidth, nx_hi-minwidth
            if ( sigx(i) .eq. 0 .and. mod(i-2,blk_fac) .eq. 0 ) then
c           if (sigx(i).eq.0) then

               if (trace) then
                  write(*,*)'stkid =',stkid
                  write(*,*)'nx_lo =',nx_lo
                  write(*,*)'ny_lo =',ny_lo
                  write(*,*)'nz_lo =',nz_lo
                  write(*,*)'i-1 =',i-1
                  write(*,*)'ny_hi =',ny_hi
                  write(*,*)'i+1 =',i+1
                  write(*,*)'nx_hi =',nx_hi
                  write(*,*)'nz_hi =',nz_hi
                  write(*,*)'box_stk(1) =',box_stk(1,stkid)
                  write(*,*)'box_stk(2) =',box_stk(2,stkid)
                  write(*,*)'box_stk(3) =',box_stk(3,stkid)
                  write(*,*)'box_stk(4) =',box_stk(4,stkid)
                  write(*,*)'box_stk(5) =',box_stk(5,stkid)
                  write(*,*)'box_stk(5) =',box_stk(5,stkid)
               end if

               call boxStack(stkid, nx_lo, ny_lo, nz_lo, i-1, ny_hi,    
     &                       nz_hi, i+1, ny_lo, nz_lo, nx_hi, ny_hi,    
     &                       nz_hi, box_stk, no_box, box_max)

               if (trace) then
                  write(*,*)'box_stk(1,id) =',box_stk(1,stkid)
                  write(*,*)'box_stk(2,id) =',box_stk(2,stkid)
                  write(*,*)'box_stk(3,id) =',box_stk(3,stkid)
                  write(*,*)'box_stk(4,id) =',box_stk(4,stkid)
                  write(*,*)'box_stk(5,id) =',box_stk(5,stkid)
                  write(*,*)'box_stk(6,id) =',box_stk(6,stkid)
                  write(*,*)'box_stk(1) =',box_stk(1,no_box)
                  write(*,*)'box_stk(2) =',box_stk(2,no_box)
                  write(*,*)'box_stk(3) =',box_stk(3,no_box)
                  write(*,*)'box_stk(4) =',box_stk(4,no_box)
                  write(*,*)'box_stk(5) =',box_stk(5,no_box)
                  write(*,*)'box_stk(6) =',box_stk(6,no_box)
               end if

               find_holes = 1
               goto 200
            end if
         end do

         do j = ny_lo+minwidth, ny_hi-minwidth
            if ( sigy(j) .eq. 0 .and. mod(j-2,blk_fac) .eq. 0 ) then
c           if (sigy(j).eq.0) then

               if (trace) then
                write(*,*)'Find_Holes intermediate'
                write(*,*)'stkid =',stkid
                write(*,*)'nx_lo =',box_stk(1,stkid)
                write(*,*)'ny_lo =',box_stk(2,stkid)
                write(*,*)'nz_lo =',box_stk(3,stkid)
                write(*,*)'nx_hi =',box_stk(4,stkid)
                write(*,*)'ny_hi =',box_stk(5,stkid)
                write(*,*)'nz_hi =',box_stk(6,stkid)
                  do i = box_stk(2,stkid),box_stk(5,stkid)
                     write(*,*)'sigy(',i,')=',sigy(i)
                  end do
               end if

               call boxStack(stkid, nx_lo, ny_lo, nz_lo, nx_hi, j-1,  
     &                       nz_hi, nx_lo, j+1, nz_lo, nx_hi, ny_hi,     
     &                       nz_hi, box_stk, no_box, box_max)
               find_holes = 1

               if (trace) then
                write(*,*)'Find_Holes after'
                write(*,*)'stkid =',stkid
                write(*,*)'nx_lo =',box_stk(1,stkid)
                write(*,*)'ny_lo =',box_stk(2,stkid)
                write(*,*)'nz_lo =',box_stk(3,stkid)
                write(*,*)'nx_hi =',box_stk(4,stkid)
                write(*,*)'ny_hi =',box_stk(5,stkid)
                write(*,*)'nz_hi =',box_stk(6,stkid)
                  do i = box_stk(2,stkid),box_stk(5,stkid)
                     write(*,*)'sigy(',i,')=',sigy(i)
                  end do
               end if

               goto 200
            end if
         end do

         do k = nz_lo+minwidth, nz_hi-minwidth
            if ( sigz(k) .eq. 0 .and. mod(k-2,blk_fac) .eq. 0 ) then
c           if (sigz(k).eq.0) then
               call boxStack(stkid, nx_lo, ny_lo, nz_lo, nx_hi, ny_hi, 
     &                       k-1, nx_lo, ny_lo, k+1, nx_hi, ny_hi,      
     &                       nz_hi, box_stk, no_box, box_max)
               find_holes = 1
               goto 200
            end if
         end do

200   continue

      return
      end


          
!-----------------------------------------------------------------------
!
!  Routine: boxStack 
!   Internal Routine to Add and Delete boxs from the 
!    box stack.
!   
!  Input:
!       stkid := Current stack number. The one to delete.
!       stlo1x/y/z := First box begining coordinates. 
!       stlo2x/y/z := Second box begining coordinates. 
!       sthi1x/y/z := First box extensions. 
!       sthi2x/y/z := Second box extensions. 
!       box_stk := box Stack.
!       NO_box  := Number of boxs thus far. Will add 1 to this.
!       box_max     := Maximum number of boxs allowed.
!  
!  Output:
!       Routine will update the value of NO_box and add in an
!       additional box and replace the current one in box_stk.
!
!-----------------------------------------------------------------------
      subroutine boxStack(stkid, stlo1x, stlo1y, stlo1z,sthi1x, sthi1y,   
     &           sthi1z,stlo2x, stlo2y, stlo2z, sthi2x, sthi2y, sthi2z,   
     &           box_stk, NO_box, box_max)

      implicit none

         integer NO_box, box_max
         integer stkid
         integer stlo1x, stlo1y, stlo1z, sthi1x, sthi1y, sthi1z
         integer stlo2x, stlo2y, stlo2z, sthi2x, sthi2y, sthi2z
         integer box_stk(6,box_max)
         integer i,j
        
         logical     ltrace
         parameter ( ltrace = .false. )

!     Check if adding one more box will exceed the maximum stack size. 

         if( (NO_box + 1) .gt. box_max )then
             write(*,*)'Error in boxStack:'
             write(*,*)'     Number of boxs will ',                       
     &                 'exceed maximum  stack size'
             stop
         end if

!     Begin by replacing the current stack with the first box.

         box_stk( 1, stkid) = stlo1x
         box_stk( 2, stkid) = stlo1y
         box_stk( 3, stkid) = stlo1z
         box_stk( 4, stkid) = sthi1x
         box_stk( 5, stkid) = sthi1y
         box_stk( 6, stkid) = sthi1z

       if ( ltrace ) then
            write(*,*) 'current stack'
            do i=1,6
               write(*,*) 'box_stk(i,stkid)= ',box_stk(i,stkid)
            enddo 
       endif

!     Next add one more stack at the end.

         box_stk( 1, NO_box+1) = stlo2x
         box_stk( 2, NO_box+1) = stlo2y
         box_stk( 3, NO_box+1) = stlo2z
         box_stk( 4, NO_box+1) = sthi2x
         box_stk( 5, NO_box+1) = sthi2y
         box_stk( 6, NO_box+1) = sthi2z

      if ( ltrace ) then
           write(*,*) 'stack just added'
           do i=1,6
              write(*,*) 'box_stk(i,NO_box+1)= ',box_stk(i,NO_box+1)
           enddo

           write(*,*) 'current stack status'
           do j=1,NO_box
              write(*,*) 'current box id# is ', j
              do i=1,6
                 write(*,*) box_stk(i,j)
              enddo
           enddo
      endif

!     Add one to the number of boxs
         NO_box = NO_box + 1


         return
       end

!-----------------------------------------------------------------------
! Routine: InitStack
!
! Simple Routine that initializes the box stack.
!
! Input: 
!      box_stk   := box Stack array.
!      stbgx/y/z := box begining coordinates.
!      stexx/y/z := box extensions.
!-----------------------------------------------------------------------
      subroutine InitStack(box_stk, stbgx, stbgy, stbgz, stexx, stexy,    
     &                     stexz, NO_box, box_max)

         implicit none

         integer box_max, NO_box,  box_stk(6,box_max),
     &           stbgx, stbgy, stbgz, stexx, stexy, stexz


!     Initialize NO_box
         NO_box = 1

!     Set up first stack element.
         box_stk( 1, 1) = stbgx
         box_stk( 2, 1) = stbgy
         box_stk( 3, 1) = stbgz
         box_stk( 4, 1) = stexx
         box_stk( 5, 1) = stexy
         box_stk( 6, 1) = stexz

         return
      end

!-----------------------------------------------------------------------
!
!   Routine: Find_Inflx
!   Integer function that calculates the laplacian given the signature
!   arrays and locates inflextion points or zeros in the laplacian.
!   With these inflextion points a choice is made as to how to split the
!   box.
!
!   Laplacian array begins at lox+1 and ends at hix-1. 
!
!-----------------------------------------------------------------------
      integer function find_inflx(nx,ny,nz, no_box, stkid, minwidth,
     &  sigx, sigy, sigz, lplx, lply, lplz, box_stk, box_max, blk_fac)

      implicit none

         integer no_box,stkid,box_max,nx,ny,nz, minwidth, blk_fac
         integer box_stk(6,box_max)
         integer sigx(nx), sigy(ny), sigz(nz)
         integer lplx(nx), lply(ny), lplz(nz)

         integer nx_lo, ny_lo, nz_lo, nx_hi, ny_hi, nz_hi
         integer nx_lo1, ny_lo1, nz_lo1,nx_hi1, ny_hi1,nz_hi1
         integer nx_lo2, ny_lo2, nz_lo2, nx_hi2, ny_hi2, nz_hi2

         integer i,j, k, hi_lplx, infptx, hi_lply, infpty, tmp
         integer hi_lplz, infptz

         logical trace
         parameter (trace = .false.)

         nx_lo = box_stk(1,stkid)
         ny_lo = box_stk(2,stkid)
         nz_lo = box_stk(3,stkid)
         nx_hi = box_stk(4,stkid)
         ny_hi = box_stk(5,stkid)
         nz_hi = box_stk(6,stkid)

 
! looking for two possible cases:
!   two succesive laplacians have opposite signs
         hi_lplx = 0
         infptx = 0
         do i = nx_lo+minwidth, nx_hi-minwidth
            if( (lplx(i).gt.0.and.lplx(i+1).lt.0) .or.           
     &          (lplx(i).lt.0.and.lplx(i+1).gt.0)      ) then
               if ( mod(i-1,blk_fac) .eq. 0 ) then
                  tmp = abs(lplx(i)-lplx(i+1))
                  if ( tmp .gt. hi_lplx ) then
                     hi_lplx = tmp
                     infptx  = i
                  end if
               end if
            end if
         end do

         hi_lply = 0
         infpty = 0
         do j = ny_lo+minwidth, ny_hi-minwidth
            if ((lply(j).gt.0.and.lply(j+1).lt.0).or.            
     &          (lply(j).lt.0.and.lply(j+1).gt.0)) then
               if (mod(j-1,blk_fac).eq.0) then
                  tmp = abs(lply(j)-lply(j+1))
                  if (tmp.gt.hi_lply) then
                     hi_lply = tmp
                     infpty  = j
                  end if
               end if
            end if
         end do

         hi_lplz = 0
         infptz = 0
         do k = nz_lo+minwidth, nz_hi-minwidth
            if ((lplz(k).gt.0.and.lplz(k+1).lt.0).or.           
     &          (lplz(k).lt.0.and.lplz(k+1).gt.0)) then
               if (mod(k-1,blk_fac).eq.0) then
                  tmp = abs(lplz(k)-lplz(k+1))
                  if (tmp.gt.hi_lplz) then
                     hi_lplz = tmp
                     infptz  = k
                  end if
               end if
            end if
         end do

         if (infpty.ne.0.or.infptx.ne.0.or.infptz.ne.0) then
            if (hi_lplx.ge.hi_lply.and.hi_lplx.ge.hi_lplz) then
               nx_lo1 = nx_lo
               ny_lo1 = ny_lo
               nz_lo1 = nz_lo
               nx_hi1 = infptx
               ny_hi1 = ny_hi
               nz_hi1 = nz_hi

c                             nx_lo2 = infptx+1
               nx_lo2 = infptx
               ny_lo2 = ny_lo
               nz_lo2 = nz_lo
               nx_hi2 = nx_hi
               ny_hi2 = ny_hi
               nz_hi2 = nz_hi
            else if (hi_lply.ge.hi_lplx.and.hi_lply.ge.hi_lplz) then
               nx_lo1 = nx_lo
               ny_lo1 = ny_lo
               nz_lo1 = nz_lo
               nx_hi1 = nx_hi
               ny_hi1 = infpty
               nz_hi1 = nz_hi

               nx_lo2 = nx_lo
c                             ny_lo2 = infpty+1
               ny_lo2 = infpty
               nz_lo2 = nz_lo
               nx_hi2 = nx_hi
               ny_hi2 = ny_hi
               nz_hi2 = nz_hi
            else
               nx_lo1 = nx_lo
               ny_lo1 = ny_lo
               nz_lo1 = nz_lo
               nx_hi1 = nx_hi
               ny_hi1 = ny_hi
               nz_hi1 = infptz

               nx_lo2 = nx_lo
               ny_lo2 = ny_lo
c                             nz_lo2 = infptz+1
               nz_lo2 = infptz
               nx_hi2 = nx_hi
               ny_hi2 = ny_hi
               nz_hi2 = nz_hi
            end if
               
           call boxStack(stkid, nx_lo1, ny_lo1, nz_lo1, nx_hi1, ny_hi1,  
     &          nz_hi1, nx_lo2, ny_lo2, nz_lo2, nx_hi2, ny_hi2, nz_hi2,  
     &          box_stk, NO_box, box_max)
           find_inflx = 1
         else
           find_inflx = 0
         end if

      return
 10   format(a,i3,a,i4)
      end

c(8/25/96) I never used out_clstr
!------------------------------------------------------------------------
!
! Routine that outputs what the clustering algorithm has done to the
!  data given initially
!
!   box_stk(1-3,i) contains nx_lo, ny_lo, nz_lo - of the ith box
!   box_stk(4-6,i) contains nx_hi, ny_hi, nz_hi - of the ith box
!
!   fnamef - contains the flagged points.
!   fnameb - contains the box info.
!
!-----------------------------------------------------------------------
      subroutine out_clstr(nx,ny,nz,no_boxes,big_box,box_stk,level,
     &                     cx,cy,cz,fnamef,fnameb)

      implicit none

         integer ind, indlnb
         integer nx,ny,nz,no_boxes, i,j, level
         character*(*) fnamef, fnameb

         real*8 xmin,xmax,ymin,ymax,zmin,zmax
         integer bnx,bny,bnz

         integer box_stk(6,no_boxes)
!
!  hdf variables needed
!
         integer    gf3_rc, gf3_write_hdf_f77
         integer    dshape(3),       cshape(3)
         real*8     big_box(nx,ny,nz),cx(nx),cy(ny),cz(nz)

         if (nx.gt.100.or.ny.gt.100.or.nz.gt.100) then
            call error('Out_clstr: box size too large. > 100.')
         end if

         open(unit=7,file=fnameb(1:indlnb(fnameb)),form='formatted',   
     &           err=900)
         write(7,10)0.0d0,0.0d0,0.0d0, 0.0d0,0.0d0,0.0d0, 0, 0, 0, 0
         xmin = 1.0d0
         ymin = 1.0d0
         zmin = 1.0d0
         xmax = 1.0d0*nx
         ymax = 1.0d0*ny
         zmax = 1.0d0*nz
         write(7,10)xmin,xmax,ymin,ymax,zmin,zmax,nx,ny,nz,0
         do i = 1,no_boxes
            xmin = box_stk(1,i)*1.0d0
            xmax = box_stk(4,i)*1.0d0
            ymin = box_stk(2,i)*1.0d0
            ymax = box_stk(5,i)*1.0d0
            zmin = box_stk(3,i)*1.0d0
            zmax = box_stk(6,i)*1.0d0
            bnx  = box_stk(4,i)-box_stk(1,i) + 1
            bny  = box_stk(5,i)-box_stk(2,i) + 1
            bnz  = box_stk(6,i)-box_stk(3,i) + 1
            write(7,10)xmin,xmax,ymin,ymax,zmin,zmax,bnx,bny,bnz,level
         end do
         close(7)

! flag file
         cshape(1) = nx
         cshape(2) = ny
         cshape(3) = nz

         dshape(1) = nx
         dshape(2) = ny
         dshape(3) = nz
! FP --- disabled hdf output

c         gf3_rc = gf3_write_hdf_f77(fnamef(1:indlnb(fnamef)),        
c     &            'flagged_pts',cshape, cx, cy, cz, dshape, big_box)
c         if( gf3_rc .ne. 0 ) then
c            call error('Out_clstr: Error writing flag file')
c         end if

      return
  10  format(1p,6e13.5,4i4)
 900  continue
      write(*,*)'Out_clstr: error opening ',fnameb(1:indlnb(fnameb))
      stop
      end

 
!---------------------------------------------------------------------
!
!     Load vector with scalar.
!
!---------------------------------------------------------------------
      SUBROUTINE L_IVLS(V1,S1,N)

         INTEGER     V1(1), S1, I, N

         DO I = 1 , N
            V1(I) = S1
         ENDDO

         RETURN
      END

!---------------------------------------------------------------------
! FP --- simple error routine
!---------------------------------------------------------------------
        subroutine error(msg)
        implicit none
        character*(*) msg

        write(*,*) msg
        return
        end

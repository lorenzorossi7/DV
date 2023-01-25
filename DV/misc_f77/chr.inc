c----------------------------------------------------------------------
c     Constants characterizing grid points
c
c     NOTE:   These must agree with definitions in graxi_rnpl file.
c----------------------------------------------------------------------
      real*8   CHR_interior,
     *         CHR_excised,
     *         CHR_amr_bdy,
     *         CHR_rhomax_bdy,
     *         CHR_rhomin_bdy,
     *         CHR_zmax_bdy,
     *         CHR_zmin_bdy,
     *         CHR_zminrhomax_bdy,
     *         CHR_zmaxrhomax_bdy,
     *         CHR_zminrhomin_bdy,
     *         CHR_zmaxrhomin_bdy
c======================================================================
        real*8 CHR_ex_bdy 
        real*8 CHR_repopulate
c======================================================================
      parameter (
     *         CHR_interior          =  0.d0,
     *         CHR_excised           =  1.d0,
     *         CHR_amr_bdy           =  2.d0,
     *         CHR_rhomax_bdy        =  3.d0,
     *         CHR_rhomin_bdy        =  4.d0,
     *         CHR_zmax_bdy          =  5.d0,
     *         CHR_zmin_bdy          =  6.d0,
     *         CHR_zminrhomax_bdy    =  7.d0,
     *         CHR_zmaxrhomax_bdy    =  8.d0,
     *         CHR_zminrhomin_bdy    =  9.d0,
     *         CHR_zmaxrhomin_bdy    = 10.d0    )
c======================================================================
        parameter ( CHR_ex_bdy = 11.0d0 )
        parameter ( CHR_repopulate = 12.0d0 )
c======================================================================


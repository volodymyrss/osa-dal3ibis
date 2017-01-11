!*****************************************************************************/
!*                                                                           */
!*                       INTEGRAL SCIENCE DATA CENTRE                        */
!*                                                                           */
!*                           DAL3  IBIS  LIBRARY                             */
!*                                                                           */
!*                          F90 SAMPLE_ICA PROGRAM                           */
!*                                                                           */
!*  Authors: Stéphane Paltani, Laurent Lerusse                               */
!*  Date:    21 May 2001                                                     */
!*  Version: 3.5.0                                                           */
!*                                                                           */
!*  Revision history                                                         */
!*                                                                           */
!*  21.05.2001 V 3.5.0                                                       */
!*  ==================                                                       */
!*  16.05: SPR0455: Fixed test data, which were not compatible anymore       */
!*                  Added "TESTING" script                                   */
!*                                                                           */
!*  02.05.2001 V 3.4.2                                                       */
!*  ==================                                                       */
!*  01.05: SPR0389: Removed multiple definition of a variable                */
!*                                                                           */
!*  08.04.2001 V 3.4.0                                                       */
!*  ==================                                                       */
!*  08.04: SCR0180: The ICA APIs have been completely revised. This part of  */
!*                  DAL3IBIS is now isolated at the file level to ease the   */
!*                  concurrent development.                                  */
!*                                                                           */
!*  01.09.1999 V 2.2.0                                                       */
!*  ==================                                                       */
!*  1. New DAL3IBIS APIs for ICA are included (written by L.Lerusse)         */
!*  3. New "samplec_ica.c" and samplef90_ica.f90" executables and test data  */
!*                                                                           */
!*  18.06.1999 V 2.0.1                                                       */
!*  27.05.1999 V 2.0.0                                                       */
!*                                                                           */
!*  10.03.1999 V 1.0.0                                                       */
!*                                                                           */
!*****************************************************************************/
 
PROGRAM SAMPLEF90_ICA

!* Only the following includes are necessary.                                */

!* ISDC standard libraries                                                   */
USE ISDC

!* <ISDC> could have been replaced by <DAL_F90_API>, as this samplef90       */
!* program does not use other ISDC libraries; However a real ISDC software   */
!* must always include <ISDC>                                                */

!* DAL3GEN library must be included                                          */

USE DAL3GEN_F90_API

!* DAL3IBIS library                                                          */

USE DAL3IBIS_F90_API

IMPLICIT NONE

 integer :: status
 integer :: i

!  /*  New Map */
 integer(kind=8), dimension(:), allocatable :: block_time
 integer(kind=1), dimension(:), allocatable :: mce_id
 integer(kind=2), dimension(:), allocatable :: period_on
 integer(kind=1), dimension(:), allocatable :: on_range
 integer(kind=1), dimension(:,:), allocatable :: block_status
 integer :: numNoisMap

!  /* Previous List */
 integer(kind=1), dimension(:), allocatable :: YSw
 integer(kind=1), dimension(:), allocatable :: ZSw
 integer(kind=8), dimension(:), allocatable :: timeDe
 integer(kind=8), dimension(:), allocatable :: timeSw
 integer(kind=1), dimension(:), allocatable :: flagSw
 integer :: nSw
!! integer(kind=8) :: mceTime(8)

!  /* previous Map */
!! integer(kind=8), dimension(:), allocatable :: blockTime 
!! integer(kind=1), dimension(:), allocatable :: mceId 
!! integer(kind=2), dimension(:), allocatable :: periodOn 
!! integer(kind=1), dimension(:), allocatable :: onRange 
!! integer(kind=1), dimension(:,:), allocatable :: blockStatus 
!! integer :: numMap = 0

!  /* Context */
 integer(kind=1) :: lowThres(128,128)
 integer(kind=8) :: thresTime
 integer(kind=1) :: ctxtStatus(128,128)
 integer(kind=8) :: ctxtTime(8)
 integer(kind=1) :: ctxtMceId(8)

 integer(kind=8) :: lowLimitTime, upLimitTime
 integer :: y, z, l, m
 integer(kind=8) :: first 
 integer(kind=8) :: last
 
 integer         :: num_chg
 integer(kind=8), dimension(:), allocatable  :: gti_start
 integer(kind=8), dimension(:), allocatable  :: gti_stop
 real(kind=8)    :: pixpercofftime(128,128)
 integer         :: numchgmce(9)
 integer(kind=8), dimension(:,:), allocatable :: efftime
 integer        , dimension(:,:), allocatable :: numpixoff

 integer  ::  idxCtxtPtr 
 integer  ::  idxNoisMapPtr
 integer  ::  idxSwListPtr
 integer  ::  ctxtPtr 
 integer  ::  noisMapPtr 
 integer  ::  swListPtr 
 integer  :: slctCtxtPtr  

 character*(41),parameter :: idxCtxtDOL    = "./test_data/idx/idx_isgri_context.fits[1]"
 character*(43),parameter :: idxNoisMapDOL = "./test_data/idx/idx_isgri_prp_noise.fits[1]"
 character*(45),parameter :: idxSwListDOL  = "./test_data/idx/idx_isgri_switch_list.fits[1]"

 character*(53),parameter :: ctxtDOL       = "./test_data/ctxt/isgri_context_20010312105300.fits[1]"
 character*(60),parameter :: noisMapDOL    = "./test_data/nois/isgri_prp_noise_2001-02-22T15:18:52.fits[1]"
 character*(62),parameter :: swListDOL     = "./test_data/list/isgri_switch_list_2001-02-22T15:18:52.fits[1]"


!! integer :: mode
!! integer :: Y(8), Z(8) 

 status = ISDC_OK
 first = 26986004086784_8
 last =  26988151570432_8
 allocate(gti_start(1:1))
 allocate(gti_stop(1:1))
 gti_start(1) = first
 gti_stop(1) = last
 status = dal_object_open(ctxtDOL, ctxtPtr, status)
 if(status == ISDC_OK) then
    print*,ctxtDOL," open!"
 else
    write (6,'(A,A,i5)')ctxtDOL," NOT open! status = ", status
 endif
 status = dal_object_open(noisMapDOL, noisMapPtr, status)
 if(status == ISDC_OK) then
    print*,noisMapDOL," open!"
 else
    write (6,'(A,A,i5)')noisMapDOL," NOT open! status = ", status
 endif
 status = dal_object_open(swListDOL, swListPtr, status)
 if(status == ISDC_OK) then
    print*,swListDOL," open!"
 else
    write (6,'(A,A,i5)')swListDOL," NOT open! status = ", status
 endif
 print*
 status = dal_object_open(idxCtxtDOL, idxCtxtPtr, status)
 if(status == ISDC_OK) then
    print*,idxCtxtDOL," open!"
 else
    write (6,'(A,A,i5)')idxCtxtDOL," NOT open! status = ", status
 endif
 status = dal_object_open(idxNoisMapDOL, idxNoisMapPtr, status)
 if(status == ISDC_OK) then
    print*,idxNoisMapDOL," open!"
 else
    write (6,'(A,A,i5)')idxNoisMapDOL," NOT open! status = ", status
 endif
 status = dal_object_open(idxSwListDOL, idxSwListPtr, status)
 if(status == ISDC_OK) then
    print*,idxSwListDOL," open!"
 else
    write (6,'(A,A,i5)')idxSwListDOL," NOT open! status = ", status
 endif

 if (status /= ISDC_OK) then
    print*, 'Can not Open the files! '
   stop  
 end if

 write (6,'(A,i5)') 'Status : ', status
 print*

!  /* 
!   *  ISGRI CONTEXT - Read 
!   */
  print*
  print*
  print*, 'ISGRI CONTEXT - Read '
 thresTime = 26775921865184_8;
 status = dal3ibis_select_ctxt(idxCtxtPtr, thresTime, ISGRI_CTXT, slctCtxtPtr, status)
 if (status /= ISDC_OK) then
    slctCtxtPtr =  ctxtPtr
    status = ISDC_OK
    print*,'Can not select the ISGRI lowThresh! Use the ctxt DS instead! ',  status
 end if     
 status = dal3ibis_ctxt_get_ima_par(slctCtxtPtr, thresTime, ISGRI_PIX_LTHR, addrof(lowThres(1,1)), status)
 status = dal3ibis_ctxt_get_tbl_par(slctCtxtPtr, thresTime, ISGRI_MODP, "MCE_ID", DAL_BYTE, addrof(ctxtMceId(1)), status)
 status = dal3ibis_ctxt_get_tbl_par(slctCtxtPtr, thresTime, ISGRI_MODP, "OB_TIME", DAL_BYTE, addrof(ctxtTime(1)), status)
 if (status /= ISDC_OK) then
    write (6,'(A,i5)')'Can not read the ISGRI lowThresh! ',  status
    stop
 end if
    
 print*,"Part of the Low threshold array :"

 print*,"Z :   -- 0 ---- 1 ---- 2 ---- 3 ---- 4 ---- 5 ---- 6 ---- 7 ---- 8 ---- 9 --"
 print*

 write(6,'(10(A,I5))')"Y 5:    ",lowThres(1,6),'   ',lowThres(2,6),'    ',&
      lowThres(3,6),'   ',lowThres(4,6),'    ',lowThres(5,6),&
      '    ',lowThres(6,6),'   ',lowThres(7,6),'   ',lowThres(8,6),'    ',&
      lowThres(9,6),'    ',lowThres(10,6)
 write(6,'(10(A,I5))')"Y 4:    ",lowThres(1,5),'    ',lowThres(2,5),'    ',&
      lowThres(3,5),'   ',lowThres(4,5),'    ',lowThres(5,5),&
      '    ',lowThres(6,5),'   ',lowThres(7,5),'   ',lowThres(8,5),'   ',&
      lowThres(9,5),'   ',lowThres(10,5)
 write(6,'(10(A,I5))')"Y 3:    ",lowThres(1,4),'    ',lowThres(2,4),'    ',&
      lowThres(3,4),'   ',lowThres(4,4),'    ',lowThres(5,4),&
      '    ',lowThres(6,4),'   ',lowThres(7,4),'   ',lowThres(8,4),'   ',&
      lowThres(9,4),'   ',lowThres(10,4)
 write(6,'(10(A,I5))')"Y 2:    ",lowThres(1,3),'    ',lowThres(2,3),'    ',&
      lowThres(3,3),'   ',lowThres(4,3),'    ',lowThres(5,3),&
      '    ',lowThres(6,3),'   ',lowThres(7,3),'   ',lowThres(8,3),'   ',&
      lowThres(9,3),'   ',lowThres(10,3)
 write(6,'(10(A,I5))')"Y 1:    ",lowThres(1,2),'    ',lowThres(2,2),'    ',&
      lowThres(3,2),'   ',lowThres(4,2),'    ',lowThres(5,2),&
      '    ',lowThres(6,2),'   ',lowThres(7,2),'   ',lowThres(8,2),'   ',&
      lowThres(9,2),'   ',lowThres(10,2)
 write(6,'(10(A,I5))')"Y 0:    ",lowThres(1,1),'    ',lowThres(2,1),'    ',&
      lowThres(3,1),'   ',lowThres(4,1),'    ',lowThres(5,1),&
      '    ',lowThres(6,1),'   ',lowThres(7,1),'   ',lowThres(8,1),'   ',&
      lowThres(9,1),'   ',lowThres(10,1)


 write (6,'(A,i5)') 'Status : ', status
 print*

!  /* 
!   *  SINGLE ISGRI NOISY PIXELS MAPS - Read 
!   */
 print*
 print*, 'ISGRI NOISY PIXELS MAPS - Read one noisy map '
 status = dal3ibis_get_size_noisy_maps( noisMapPtr, 0_8, 0_8, numNoisMap, status)
 allocate(block_time(0:numNoisMap-1))
 allocate(mce_id(0:numNoisMap-1))
 allocate(period_on(0:numNoisMap-1))
 allocate(on_range(0:numNoisMap-1))
 allocate(block_status(0:256-1,0:numNoisMap))
! status = dal3ibis_get_noisy_maps( noisMapPtr, 0_8, 0_8, block_time, mce_id, period_on, on_range, block_status(0,0),&
!      numNoisMap, status)
! if (status /= ISDC_OK) then
!       print*,'Can not read the new ISGRI noisy pixel map! ', status
!  stop
! end if     

! print*, '  Number of noisy Maps :', numNoisMap
! print*, '  Time start', block_time(0),'; time end', block_time(numNoisMap-1)
! print*, '  The data are difficult to display.'
! print*, '     Block  0  -> mce', mce_id(0) 
! print*, '     Block', numNoisMap/3, ' -> mce', mce_id(numNoisMap/3) 
! print*, '     Block', 2*numNoisMap/3, ' -> mce', mce_id(2*numNoisMap/3) 
! print*, '     Block', numNoisMap-1, ' -> mce', mce_id(numNoisMap-1)
! print*

! print*, 'Status : ', status
! print*

!  /* 
!   *  MANY ISGRI NOISY PIXELS MAPS - Read 
!   */
 deallocate(block_time)
 deallocate(mce_id)
 deallocate(period_on)
 deallocate(on_range)
 deallocate(block_status)

 print*
 print*, 'ISGRI NOISY PIXELS MAPS - Read several noisy maps '
 status = dal3ibis_get_size_noisy_maps( idxNoisMapPtr, first, last, numNoisMap, status)
 allocate(block_time(0:numNoisMap-1))
 allocate(mce_id(0:numNoisMap-1))
 allocate(period_on(0:numNoisMap-1))
 allocate(on_range(0:numNoisMap-1))
 allocate(block_status(0:256-1,0:numNoisMap))
! status = dal3ibis_get_noisy_maps( idxNoisMapPtr, first, last, block_time, mce_id, period_on, on_range, block_status(0,0),&
!      numNoisMap, status)
! if (status /= ISDC_OK) then
!       print*,'Can not read the new ISGRI noisy pixel map! ', status
!  stop
! end if     

! print*, '  Number of noisy Maps : ', numNoisMap
! print*, '  Time start ', block_time(0),' time end', block_time(numNoisMap-1)
! print*, '  The data are difficult to display.'
! print*, '     Block   0  -> mce', mce_id(0) 
! print*, '     Block ', numNoisMap/3, ' -> mce', mce_id(numNoisMap/3) 
! print*, '     Block', 2*numNoisMap/3, ' -> mce', mce_id(2*numNoisMap/3) 
! print*, '     Block', numNoisMap-1, ' -> mce', mce_id(numNoisMap-1)
! print*

! print*, 'Status : ', status
! print*

!  /* 
!   *  ISGRI PIXEL SWITCHES LIST - Read 
!   */
 print*,'ISGRI PIXEL SWITCHES LIST - Read '
 status = dal3ibis_get_size_switch_list( idxSwListPtr, first, last, nSw, status)
 print*, '   '
 allocate(YSw(0:nSw-1))
 allocate(ZSw(0:nSw-1))
 allocate(timeDe(0:nSw-1))
 allocate(timeSw(0:nSw-1))
 allocate(flagSw(0:nSw-1))
 status = dal3ibis_get_switch_list( idxSwListPtr, first, last, YSw, ZSw, timeDe, timeSw, flagSw, nSw, status)
 if (status /= ISDC_OK) then
  print*,'    Can not read the previous ISGRI pixel switches list! ', status
stop
 end if    
 write (6,'(A,i5)')'  Number of switches :', nSw
 write (6,'(A,I20,A,I20)')'  Time start', timeDe(0),'; time end',timeSw(nSw-1)
 print*,'  Five typical switches :'
 i = 00
 write (6,'(A,I5,A,I5,A,I5,A,I5)')'          i: ',i,',  Y: ', YSw(i),',  Z: ', ZSw(i),', flag: ',flagSw(i) 
 i = 10
 write (6,'(A,I5,A,I5,A,I5,A,I5)')'          i: ',i,',  Y: ', YSw(i),',  Z: ', ZSw(i),', flag: ',flagSw(i) 
 i = 20
 write (6,'(A,I5,A,I5,A,I5,A,I5)')'          i: ',i,',  Y: ', YSw(i),',  Z: ', ZSw(i),', flag: ',flagSw(i) 
 i = 30
 write (6,'(A,I5,A,I5,A,I5,A,I5)')'          i: ',i,',  Y: ', YSw(i),',  Z: ', ZSw(i),', flag: ',flagSw(i) 
 i = 40
 write (6,'(A,I5,A,I5,A,I5,A,I5)')'          i: ',i,',  Y: ', YSw(i),',  Z: ', ZSw(i),', flag: ',flagSw(i) 
 
 write (6,'(A,i5)') 'Status : ', status
 print*
 
!* /*
!*  *  Get the efficiency of the pixels for this science window.
!*  */
 print*
 print*,'ISGRI PIXEL EFFICIENCY - calculation'
 status = dal3ibis_get_size_switch_list( idxSwListPtr, first, last, num_chg, status)
 write(6,'(A,I20,I20)')'   Test DAL3IBISicaIsgriNoisEff with the limits', &
      first, last
 write (6,'(A,i5)')'   Number of changes :',   num_chg

 allocate(efftime(1:9,1:num_chg))
 allocate(numpixoff(1:9,1:num_chg))

 status = dal3ibis_ica_isgri_nois_eff(idxSwListPtr, 1, gti_start, gti_stop, &
      pixpercofftime, numchgmce, efftime, numpixoff, status)

 print*
 print*,'   The number of level in the timeline for the modules are :'
 write(6,'(9(A,I5))')'     ',numchgmce(1),'   ',numchgmce(2),'   ',&
      numchgmce(3),'   ',numchgmce(4),'   ',numchgmce(5),'   ',&
      numchgmce(6),'   ',&
      numchgmce(7),'   ',numchgmce(8),'   ',numchgmce(9)
 i = 1
 write(6,'(I5,9(A,I5))')i-1,')  ', numpixoff(1,i),'   ', numpixoff(2,i), &
      '   ', numpixoff(3,i), '   ', numpixoff(4,i),'   ', numpixoff(5,i),&
      '   ', & 
      numpixoff(6,i),'   ', numpixoff(7,i), '   ', numpixoff(8,i), &
      '   ', numpixoff(9,i)
 i = 6
 write(6,'(I5,9(A,I5))')i-1,')  ', numpixoff(1,i),'   ', numpixoff(2,i), &
      '   ', numpixoff(3,i), '   ', numpixoff(4,i),'   ', numpixoff(5,i),&
      '   ', & 
      numpixoff(6,i),'   ', numpixoff(7,i), '   ', numpixoff(8,i), '   ',&
      numpixoff(9,i)
 i = 11
 write(6,'(I5,9(A,I5))')i-1,')  ', numpixoff(1,i),'   ', numpixoff(2,i), &
      '   ', numpixoff(3,i), '   ', numpixoff(4,i),'   ', numpixoff(5,i),&
      '   ', & 
      numpixoff(6,i),'   ', numpixoff(7,i), '   ', numpixoff(8,i), '   ',&
      numpixoff(9,i)
 i = 16
 write(6,'(I5,9(A,I5))')i-1,')  ', numpixoff(1,i),'   ', numpixoff(2,i), &
      '   ', numpixoff(3,i), '   ', numpixoff(4,i),'   ', numpixoff(5,i),&
      '   ', & 
      numpixoff(6,i),'   ', numpixoff(7,i), '   ', numpixoff(8,i), '   ',&
      numpixoff(9,i)
 i = 21
 write(6,'(I5,9(A,I5))')i-1,')  ', numpixoff(1,i),'   ', numpixoff(2,i), &
      '   ', numpixoff(3,i), '   ', numpixoff(4,i),'   ', numpixoff(5,i),&
      '   ', & 
      numpixoff(6,i),'   ', numpixoff(7,i), '   ', numpixoff(8,i), '   ',&
      numpixoff(9,i)
 i = 26
 write(6,'(I5,9(A,I5))')i-1,')  ', numpixoff(1,i),'   ', numpixoff(2,i), &
      '   ', numpixoff(3,i), '   ', numpixoff(4,i),'   ', numpixoff(5,i),&
      '   ', & 
      numpixoff(6,i),'   ', numpixoff(7,i), '   ', numpixoff(8,i), '   ', &
      numpixoff(9,i)
 i = 31
 write(6,'(I5,9(A,I5))')i-1,')  ', numpixoff(1,i),'   ', numpixoff(2,i), &
      '   ', numpixoff(3,i), '   ', numpixoff(4,i),'   ', numpixoff(5,i),&
      '   ', & 
      numpixoff(6,i),'   ', numpixoff(7,i), '   ', numpixoff(8,i), '   ',&
      numpixoff(9,i)
 i = 36
 write(6,'(I5,9(A,I5))')i-1,')  ', numpixoff(1,i),'   ', numpixoff(2,i),&
      '   ', numpixoff(3,i), '   ', numpixoff(4,i),'   ', numpixoff(5,i),&
      '   ', & 
      numpixoff(6,i),'   ', numpixoff(7,i), '   ', numpixoff(8,i), '   ',&
      numpixoff(9,i)
 i = 41
 write(6,'(I5,9(A,I5))')i-1,')  ', numpixoff(1,i),'   ', numpixoff(2,i), &
      '   ', numpixoff(3,i), '   ', numpixoff(4,i),'   ', numpixoff(5,i),&
      '   ', & 
      numpixoff(6,i),'   ', numpixoff(7,i), '   ', numpixoff(8,i), '   ',&
      numpixoff(9,i)
 print*
 print*,'   The negative values are due to bad test data.'
 print*
 print*,'   The %% OFF time of the pixels: '
 l = 5;
 m = 78;
 write(6,'(A,I5,A,I5,A,F20.10,A)')'       pixel[',m,'][',l,']: ', &
      pixPercOffTime(l+1,m+1),'% OFF'
 l = 7;
 m = 3;
 write(6,'(A,I5,A,I5,A,F20.10,A)')'       pixel[',m,'][',l,']: ', &
      pixPercOffTime(l+1,m+1),'% OFF'
 l = 18;
 m = 42;
 write(6,'(A,I5,A,I5,A,F20.10,A)')'       pixel[',m,'][',l,']: ', &
      pixPercOffTime(l+1,m+1),'% OFF'
 l = 56;
 m = 124;
 write(6,'(A,I5,A,I5,A,F20.10,A)')'       pixel[',m,'][',l,']: ', &
      pixPercOffTime(l+1,m+1),'% OFF'
 l = 83;
 m = 42;
 write(6,'(A,I5,A,I5,A,F20.10,A)')'       pixel[',m,'][',l,']: ', &
      pixPercOffTime(l+1,m+1),'% OFF'
 l = 120;
 m = 124;
 write(6,'(A,I5,A,I5,A,F20.10,A)')'       pixel[',m,'][',l,']: ', &
      pixPercOffTime(l+1,m+1),'% OFF'
 print*


 write (6,'(A,i5)') 'Status : ', status
 print*

!  /*
!   *   CLOSE AND EXIT. 
!   */
 print*
 print*,"That's all, close the data structures"
 print*

! It's not clear what's broken, but, for now, don't close the
! files
! status = dal_object_close( idxCtxtPtr,    DAL_SAVE, status); 
! status = dal_object_close( idxNoisMapPtr, DAL_SAVE, status); 
! status = dal_object_close( idxSwListPtr,  DAL_SAVE, status); 
! status = dal_object_close( ctxtPtr,    DAL_SAVE, status); 
! status = dal_object_close( noisMapPtr, DAL_SAVE, status);
! status = dal_object_close( swListPtr,  DAL_SAVE, status);


 write (6,'(A,i5)') 'Status : ', status
 print*

END PROGRAM


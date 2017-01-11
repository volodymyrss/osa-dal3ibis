!*****************************************************************************/
!*                                                                           */
!*                       INTEGRAL SCIENCE DATA CENTRE                        */
!*                                                                           */
!*                           DAL3  IBIS  LIBRARY                             */
!*                                                                           */
!*                                  ICA                                      */
!*                                                                           */
!*                              F90 PROTOTYPE                                */
!*                                                                           */
!*  Authors: Stéphane Paltani, Laurent Lerusse                               */
!*  Date:    8 April 2001                                                    */
!*  Version: 3.4.0                                                           */
!*                                                                           */
!*  Revision history                                                         */
!*                                                                           */
!*  08.04.2001 V 3.4.0 First version                                         */
!*  ==================                                                       */
!*  08.04: SCR0180: The ICA APIs have been completely revised. This part of  */
!*                  DAL3IBIS is now isolated at the file level to ease the   */
!*                  concurrent development.                                  */
!*                                                                           */
!*****************************************************************************/

MODULE DAL3IBIS_ICA_F90_API

USE    DAL_F90_API
USE    DAL3GEN_F90_API

INTEGER, PARAMETER :: DAL3IBIS_ICA_ERROR_CODE_BASE     = -26000

INTEGER, PARAMETER :: DAL3IBIS_NOT_VALID_DS            = DAL3IBIS_ICA_ERROR_CODE_BASE-302
INTEGER, PARAMETER :: DAL3IBIS_NOT_VALID_INDEX         = DAL3IBIS_ICA_ERROR_CODE_BASE-303
INTEGER, PARAMETER :: DAL3IBIS_INDEX_EMPTY             = DAL3IBIS_ICA_ERROR_CODE_BASE-304
INTEGER, PARAMETER :: DAL3IBIS_ICA_PIX_POS_ERR         = DAL3IBIS_ICA_ERROR_CODE_BASE-305
INTEGER, PARAMETER :: DAL3IBIS_BAD_CTXT_STRUCT         = DAL3IBIS_ICA_ERROR_CODE_BASE-306
INTEGER, PARAMETER :: DAL3IBIS_BAD_CTXT_PARAMETER      = DAL3IBIS_ICA_ERROR_CODE_BASE-307
INTEGER, PARAMETER :: DAL3IBIS_SW_LIST_NOT_EMPTY       = DAL3IBIS_ICA_ERROR_CODE_BASE-308

INTEGER, PARAMETER :: IBIS_NUM_BLOCK     =   8
INTEGER, PARAMETER :: ISGRI_SIZE         = 128
INTEGER, PARAMETER :: IBIS_IBLOCK_LENGTH = 256
INTEGER, PARAMETER :: PICSIT_SIZE        =  64

INTEGER, PARAMETER :: ISGRI_CTXT      = 10
INTEGER, PARAMETER :: ISGRI_PIX_GAIN  = 11 
INTEGER, PARAMETER :: ISGRI_PIX_LTHR  = 12 
INTEGER, PARAMETER :: ISGRI_PIX_STA   = 13
INTEGER, PARAMETER :: ISGRI_PIX_TEST  = 14
INTEGER, PARAMETER :: ISGRI_ASIC_GAIN = 15
INTEGER, PARAMETER :: ISGRI_ASIC_HTHR = 16
INTEGER, PARAMETER :: ISGRI_MODP      = 18 
INTEGER, PARAMETER :: PICSIT_CTXT     = 20 
INTEGER, PARAMETER :: PICSIT_PIX_STA  = 21
INTEGER, PARAMETER :: PICSIT_MODP     = 28
INTEGER, PARAMETER :: PICSIT_DETP     = 29 

INTEGER, PARAMETER :: DAL3IBIS_NOT_NOISY_EVTS      =  1
INTEGER, PARAMETER :: DAL3IBIS_NOISY_EVTS          =  0
INTEGER, PARAMETER :: DAL3IBIS_ICA_INCLUDE_EXT_MAP =  1
INTEGER, PARAMETER :: DAL3IBIS_ICA_NO_EXT_MAP      = -1


! The following functions are FORTRAN 90 interfaces for DAL3IBIS functions
! written by Laurent Lerusse
! ####################################################################

INTERFACE DAL3IBIS_ICA_TRANSF_COORD

   FUNCTION DAL3IBIS_ICA_TRANSF_COORD(MMODULE, OFFSETBYTES, BITSNUM, Y, Z, STATUS)

     INTEGER         :: MMODULE
     INTEGER         :: OFFSETBYTES
     INTEGER         :: BITSNUM
     INTEGER         :: Y
     INTEGER         :: Z
     INTEGER         :: STATUS
     INTEGER         :: DAL3IBIS_ICA_TRANSF_COORD

   END FUNCTION DAL3IBIS_ICA_TRANSF_COORD

END INTERFACE

INTERFACE DAL3IBIS_GET_SIZE_NOISY_MAPS

   FUNCTION DAL3IBIS_GET_SIZE_NOISY_MAPS(MAPPTR, OBTSTART, OBTEND, NOISYLENGTH, STATUS)

     INTEGER         :: MAPPTR
     INTEGER(KIND=8) :: OBTSTART
     INTEGER(KIND=8) :: OBTEND
     INTEGER         :: NOISYLENGTH
     INTEGER         :: STATUS
     INTEGER         :: DAL3IBIS_GET_SIZE_NOISY_MAPS

   END FUNCTION DAL3IBIS_GET_SIZE_NOISY_MAPS

END INTERFACE

!INTERFACE DAL3IBIS_GET_NOISY_MAPS
! comment out as per screw 841

!   FUNCTION DAL3IBIS_GET_NOISY_MAPS(MAPPTR, OBTSTART, OBTEND,  BLOCK_TIME, MCE_ID, &
!                                   PERIOD_ON, ON_RANGE, BLOCK_STATUS, NOISYLENGTH, STATUS)

!     INTEGER         :: GRPPTR
!     INTEGER(KIND=8) :: OBTSTART
!     INTEGER(KIND=8) :: OBTEND
!     INTEGER(KIND=8) :: BLOCK_TIME(*)
!     INTEGER(KIND=1) :: MCE_ID(*)
!     INTEGER(KIND=2) :: PERIOD_ON(*)
!     INTEGER(KIND=1) :: ON_RANGE(*)
!     INTEGER(KIND=1) :: BLOCK_STATUS
!     INTEGER         :: NOISYLENGTH
!     INTEGER         :: STATUS
!     INTEGER         :: DAL3IBIS_GET_NOISY_MAPS

!   END FUNCTION DAL3IBIS_GET_NOISY_MAPS

!END INTERFACE

INTERFACE DAL3IBIS_GET_SIZE_SWITCH_LIST

   FUNCTION DAL3IBIS_GET_SIZE_SWITCH_LIST(LISTPTR, OBTSTART, OBTEND, LISTLENGTH, STATUS)

     INTEGER         :: LISTPTR
     INTEGER(KIND=8) :: OBTSTART
     INTEGER(KIND=8) :: OBTEND
     INTEGER         :: LISTLENGTH
     INTEGER         :: STATUS
     INTEGER         :: DAL3IBIS_GET_SIZE_SWITCH_LIST

   END FUNCTION DAL3IBIS_GET_SIZE_SWITCH_LIST

END INTERFACE

INTERFACE DAL3IBIS_GET_SWITCH_LIST

   FUNCTION DAL3IBIS_GET_SWITCH_LIST(LISTPTR, OBTSTART, OBTEND, Y_SWITCH, Z_SWITCH, &
                                     TIMEDETECT, TIMESWITCH, FLAG_SWITCH, LISTLENGTH, STATUS)

     INTEGER         :: LISTPTR
     INTEGER(KIND=8) :: OBTSTART
     INTEGER(KIND=8) :: OBTEND
     INTEGER(KIND=1) :: Y_SWITCH(*)
     INTEGER(KIND=1) :: Z_SWITCH(*)
     INTEGER(KIND=8) :: TIMEDETECT(*)
     INTEGER(KIND=8) :: TIMESWITCH(*)
     INTEGER(KIND=1) :: FLAG_SWITCH(*)
     INTEGER         :: LISTLENGTH
     INTEGER         :: STATUS
     INTEGER         :: DAL3IBIS_GET_SWITCH_LIST

   END FUNCTION DAL3IBIS_GET_SWITCH_LIST

END INTERFACE

INTERFACE DAL3IBIS_SELECT_CTXT

   FUNCTION DAL3IBIS_SELECT_CTXT(CTXTPTR, LIMTIME, DATA_KIND, SLCTCTXTPTR, STATUS)

     INTEGER         :: CTXTPTR
     INTEGER(KIND=8) :: LIMTIME
     INTEGER         :: DATA_KIND
     INTEGER         :: SLCTCTXTPTR
     INTEGER         :: STATUS
     INTEGER         :: DAL3IBIS_SELECT_CTXT

   END FUNCTION DAL3IBIS_SELECT_CTXT

END INTERFACE


INTERFACE DAL3IBIS_CTXT_GET_IMA_PAR

   FUNCTION DAL3IBIS_CTXT_GET_IMA_PAR(CTXTPTR, LIMTIME, DATA_KIND, DATABUFF, STATUS)

     INTEGER         :: CTXTPTR
     INTEGER(KIND=8) :: LIMTIME
     INTEGER         :: DATA_KIND
     INTEGER         :: DATABUFF
     INTEGER         :: STATUS
     INTEGER         :: DAL3IBIS_CTXT_GET_IMA_PAR

   END FUNCTION  DAL3IBIS_CTXT_GET_IMA_PAR

END INTERFACE


INTERFACE DAL3IBIS_CTXT_GET_TBL_PAR

   FUNCTION DAL3IBIS_CTXT_GET_TBL_PAR(CTXTPTR, LIMTIME, DATA_KIND, COLNAME, DATATYPE, DATABUFF, STATUS)

     INTEGER         :: CTXTPTR
     INTEGER(KIND=8) :: LIMTIME
     INTEGER         :: DATA_KIND
     CHARACTER*(*)   :: COLNAME
     INTEGER         :: DATATYPE
     INTEGER         :: DATABUFF
     INTEGER         :: STATUS
     INTEGER         :: DAL3IBIS_CTXT_GET_TBL_PAR

   END FUNCTION  DAL3IBIS_CTXT_GET_TBL_PAR

END INTERFACE
INTERFACE DAL3IBIS_ICA_ISGRI_NOIS_EFF

   FUNCTION DAL3IBIS_ICA_ISGRI_NOIS_EFF(IDXLISTPTR, NUM_INTERVALS, GTI_START, GTI_STOP, &
                                        PIXPERCOFFTIME, NUMCHG, EFFTIME, NUMPIXOFF, STATUS)

     INTEGER         :: IDXLISTPTR
     INTEGER         :: NUM_INTERVALS
     INTEGER(KIND=8) :: GTI_START(*)
     INTEGER(KIND=8) :: GTI_STOP(*)
     REAL(KIND=8)    :: PIXPERCOFFTIME(128,128)
     INTEGER         :: NUMCHG(9)
     INTEGER(KIND=8) :: EFFTIME(9,*)
     INTEGER         :: NUMPIXOFF(9,*)
     INTEGER         :: STATUS
     INTEGER         :: DAL3IBIS_ICA_ISGRI_NOIS_EFF

   END FUNCTION DAL3IBIS_ICA_ISGRI_NOIS_EFF

END INTERFACE

END MODULE DAL3IBIS_ICA_F90_API

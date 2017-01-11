!*****************************************************************************/
!*                                                                           */
!*                       INTEGRAL SCIENCE DATA CENTRE                        */
!*                                                                           */
!*                           DAL3  IBIS  LIBRARY                             */
!*                                                                           */
!*                             F90 PROTOTYPE                                 */
!*                                                                           */
!*  Authors: Stéphane Paltani, Laurent Lerusse                               */
!*  Date:    28 September 2001                                               */
!*  Version: 3.5.5                                                           */
!*                                                                           */
!*  Revision history                                                         */
!*                                                                           */
!*  28.09.2001 V 3.5.5                                                       */
!*  ==================                                                       */
!*  28.09: SPR0475: Moved NULL "&" string treatment into F90                 */
!*                                                                           */
!*  11.05.2000 V 3.0.0                                                       */
!*  ==================                                                       */
!*  1. Removed the event treatment from DAL3IBIS and put it in DAL3GEN       */
!*                                                                           */
!*  21.02.2000 V 2.2.5                                                       */
!*  ==================                                                       */
!* 1. The library uses now Makefile-2.0.1                                    */
!* 2. The library requires now DAL3GEN 2.4                                   */
!* 3. The library now installs "dal3ibis_f90_api_local.mod" for some non-SUN */
!*    F90 compilers                                                          */
!* 8. Added a F90 interface to DAL3IBISgetPixStatusMap                       */
!*                                                                           */
!*  29.10.1999 V 2.2.1                                                       */
!*  ==================                                                       */
!* 1. Fixed a bug that prevented the reading of some COMPTON event           */
!*    parameters.                                                            */
!*                                                                           */
!*  01.09.1999 V 2.2.0                                                       */
!*  ==================                                                       */
!* 1. New DAL3IBIS APIs for ICA are included (written by L.Lerusse)          */
!*                                                                           */
!*  22.08.1999 V 2.1.0                                                       */
!*  ==================                                                       */
!* 1. The library now uses DAL 1.3 and Makefile-1.3.1                        */
!* 2. Changed variable and parameter names in accordance with the CTS.       */
!*    Defined new symbols for parameters.                                    */
!*                                                                           */
!*  18.06.1999 V 2.0.1                                                       */
!*  27.05.1999 V 2.0.0                                                       */
!*                                                                           */
!*  10.03.1999 V 1.0.0                                                       */
!*                                                                           */
!*****************************************************************************/

MODULE DAL3IBIS_F90_API_LOCAL

USE    DAL_F90_API
USE    DAL3GEN_F90_API


INTEGER, PARAMETER :: DAL3IBIS_ERROR_CODE_BASE     = -26000

INTEGER, PARAMETER :: DAL3IBIS_NO_IBIS_EVENTS      = DAL3IBIS_ERROR_CODE_BASE-1

INTEGER, PARAMETER :: DAL3IBIS_EVPROP_INEXISTENT   = DAL3IBIS_ERROR_CODE_BASE-2

INTEGER, PARAMETER :: DAL3IBIS_EVPROP_INCOMPATIBLE = DAL3IBIS_ERROR_CODE_BASE-3


INTEGER, PARAMETER :: DAL3IBIS_INVALID_GROUP       = DAL3IBIS_ERROR_CODE_BASE-10


INTEGER, PARAMETER :: DAL3IBIS_NOT_A_SWG           = DAL3IBIS_ERROR_CODE_BASE-101

INTEGER, PARAMETER :: DAL3IBIS_GAPS_NO_OBT         = DAL3IBIS_ERROR_CODE_BASE-201


INTEGER, PARAMETER :: ISGRI_EVTS   =  0
INTEGER, PARAMETER :: PICSIT_SGLE  =  1
INTEGER, PARAMETER :: PICSIT_MULE  =  2
INTEGER, PARAMETER :: COMPTON_SGLE =  3
INTEGER, PARAMETER :: COMPTON_MULE =  4

INTEGER, PARAMETER :: COMP_TIME                    = TIME_TAG

INTEGER            :: DAL3_NULL(1)

!*                            DAL3  IBIS  LIBRARY                            */
!*                            EVENT MANIPULATIONS                            */

INTERFACE DAL3IBIS_SHOW_ALL_EVENTS

   FUNCTION DAL3IBIS_SHOW_ALL_EVENTS(DAL_DS, IBIS_NUM, CURRENT_STATUS)

     INTEGER         :: DAL_DS
     INTEGER         :: IBIS_NUM(*)
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_SHOW_ALL_EVENTS

   END FUNCTION DAL3IBIS_SHOW_ALL_EVENTS

END INTERFACE

INTERFACE DAL3IBIS_SELECT_EVENTS_C

   FUNCTION DAL3IBIS_SELECT_EVENTS_C(DAL_DS, IBIS_NUM, COLSEL, OBTNUMS, OBTFIRST, OBTLAST, ROWFILTER, CURRENT_STATUS)

     INTEGER         :: DAL_DS
     INTEGER         :: IBIS_NUM
     INTEGER         :: COLSEL
     INTEGER         :: OBTNUMS
     INTEGER(KIND=8) :: OBTFIRST(*)
     INTEGER(KIND=8) :: OBTLAST(*)
     INTEGER         :: ROWFILTER
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_SELECT_EVENTS_C

   END FUNCTION DAL3IBIS_SELECT_EVENTS_C

END INTERFACE


INTERFACE DAL3IBIS_GET_NUM_EVENTS

   FUNCTION DAL3IBIS_GET_NUM_EVENTS(NUM_EVENTS, CURRENT_STATUS)

     INTEGER         :: NUM_EVENTS
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_GET_NUM_EVENTS

   END FUNCTION DAL3IBIS_GET_NUM_EVENTS

END INTERFACE


INTERFACE DAL3IBIS_GET_EVENTS

   FUNCTION DAL3IBIS_GET_EVENTS(COLID, COLTYPE, COLBUFF, CURRENT_STATUS)

     INTEGER         :: COLID
     INTEGER         :: COLTYPE
     INTEGER         :: COLBUFF
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_GET_EVENTS

   END FUNCTION DAL3IBIS_GET_EVENTS

END INTERFACE

INTERFACE DAL3IBIS_GET_EVENTS_BINS

   FUNCTION DAL3IBIS_GET_EVENTS_BINS(COLID, COLTYPE, STARTBIN, ENDBIN, COLBUFF, CURRENT_STATUS)

     INTEGER         :: COLID
     INTEGER         :: COLTYPE
     INTEGER         :: STARTBIN
     INTEGER         :: ENDBIN
     INTEGER         :: COLBUFF
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_GET_EVENTS_BINS

   END FUNCTION DAL3IBIS_GET_EVENTS_BINS

END INTERFACE


INTERFACE DAL3IBIS_CLOSE_EVENTS

   FUNCTION DAL3IBIS_CLOSE_EVENTS(CURRENT_STATUS)

     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_CLOSE_EVENTS

   END FUNCTION DAL3IBIS_CLOSE_EVENTS

END INTERFACE


INTERFACE DAL3IBIS_GET_NUM_EVENT_PACKETS

   FUNCTION DAL3IBIS_GET_NUM_EVENT_PACKETS(DAL_DS, EVTYPE, NUM_PACKETS, CURRENT_STATUS)

     INTEGER         :: DAL_DS
     INTEGER         :: EVTYPE
     INTEGER         :: NUM_PACKETS
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_GET_NUM_EVENT_PACKETS

   END FUNCTION DAL3IBIS_GET_NUM_EVENT_PACKETS

END INTERFACE


INTERFACE DAL3IBIS_GET_EVENT_PACKETS_F90

   FUNCTION DAL3IBIS_GET_EVENT_PACKETS_F90(DAL_DS, EVTYPE, SSC, PART_LOBT, ROW_BEGIN, ROW_NUM,&
                                       FIRST_NUM, SECOND_NUM, COVFLW_NUM, DAL3_NULL, CURRENT_STATUS)

     INTEGER         :: DAL_DS
     INTEGER         :: EVTYPE
     INTEGER         :: SSC
     INTEGER         :: PART_LOBT
     INTEGER         :: ROW_BEGIN
     INTEGER         :: ROW_NUM
     INTEGER         :: FIRST_NUM
     INTEGER         :: SECOND_NUM
     INTEGER         :: COVFLW_NUM
     INTEGER         :: DAL3_NULL
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_GET_EVENT_PACKETS_F90

   END FUNCTION DAL3IBIS_GET_EVENT_PACKETS_F90

END INTERFACE


INTERFACE DAL3IBIS_GET_NUM_DUMMY

   FUNCTION DAL3IBIS_GET_NUM_DUMMY(DAL_DS, EVTYPE, NUM_DUMMY, CURRENT_STATUS)

     INTEGER         :: DAL_DS
     INTEGER         :: EVTYPE
     INTEGER         :: NUM_DUMMY
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_GET_NUM_DUMMY

   END FUNCTION DAL3IBIS_GET_NUM_DUMMY

END INTERFACE

INTERFACE DAL3IBIS_GET_DUMMY_F90

  FUNCTION DAL3IBIS_GET_DUMMY_F90(DAL_DS, EVTYPE, SSC, PART_LOBT, POSITION, &
       FIRST_DUMMY, SECOND_DUMMY, COUNTER_OVFLW, DAL3_NULL, CURRENT_STATUS)

    INTEGER         :: DAL_DS
    INTEGER         :: EVTYPE
    INTEGER         :: SSC
    INTEGER         :: PART_LOBT
    INTEGER         :: POSITION
    INTEGER         :: FIRST_DUMMY
    INTEGER         :: SECOND_DUMMY
    INTEGER         :: COUNTER_OVFLW
    INTEGER         :: DAL3_NULL
    INTEGER         :: CURRENT_STATUS
    INTEGER         :: DAL3IBIS_GET_DUMMY_F90

  END FUNCTION DAL3IBIS_GET_DUMMY_F90

END INTERFACE

INTERFACE DAL3IBIS_CALCULATE_EVENT_GAPS

   FUNCTION DAL3IBIS_CALCULATE_EVENT_GAPS(DAL_DS, EVTYPE, NUM_MISS_PACK, NUM_CONTINUOUS, CURRENT_STATUS)

     INTEGER         :: DAL_DS
     INTEGER         :: EVTYPE
     INTEGER         :: NUM_MISS_PACK
     INTEGER         :: NUM_CONTINUOUS
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_CALCULATE_EVENT_GAPS

   END FUNCTION DAL3IBIS_CALCULATE_EVENT_GAPS

END INTERFACE

INTERFACE DAL3IBIS_FIND_GAPS_OBT

   FUNCTION DAL3IBIS_FIND_GAPS_OBT(DAL_DS, EVT_TYPE, OBTFIRST_EVENT, OBTLAST_EVENT, CURRENT_STATUS)

     INTEGER         :: DAL_DS
     INTEGER         :: EVT_TYPE
     INTEGER         :: OBTFIRST_EVENT
     INTEGER         :: OBTLAST_EVENT
     INTEGER         :: CURRENT_STATUS
     INTEGER         :: DAL3IBIS_FIND_GAPS_OBT

   END FUNCTION DAL3IBIS_FIND_GAPS_OBT

END INTERFACE

! The following functions cannot call directly the C functions, because of the
! DAL3_DUMMY vector. Some F90 code is necessary 

CONTAINS

  FUNCTION DAL3IBIS_GET_EVENT_PACKETS(DAL_DS, EVTYPE, SSC, PART_LOBT, ROW_BEGIN, ROW_NUM,&
       FIRST_NUM, SECOND_NUM, COVFLW_NUM, CURRENT_STATUS)

    IMPLICIT NONE

    INTEGER         :: DAL_DS
    INTEGER         :: EVTYPE
    INTEGER         :: SSC(*)
    INTEGER         :: PART_LOBT(*)
    INTEGER         :: ROW_BEGIN(*)
    INTEGER         :: ROW_NUM(*)
    INTEGER         :: FIRST_NUM(*)
    INTEGER         :: SECOND_NUM(*)
    INTEGER         :: COVFLW_NUM(*)
    INTEGER         :: CURRENT_STATUS
    INTEGER         :: DAL3IBIS_GET_EVENT_PACKETS

    DAL3IBIS_GET_EVENT_PACKETS = DAL3IBIS_GET_EVENT_PACKETS_F90(DAL_DS, EVTYPE, SSC(1), PART_LOBT(1), ROW_BEGIN(1),&
         ROW_NUM(1), FIRST_NUM(1), SECOND_NUM(1), COVFLW_NUM(1), DAL3_NULL(1), CURRENT_STATUS)

  END FUNCTION DAL3IBIS_GET_EVENT_PACKETS



  FUNCTION DAL3IBIS_FIND_EVENT_GAPS(DAL_DS, EVT_TYPE, NUM_MISS_PACK, NUM_CONTINUOUS, &
                                    OBTFIRST_EVENT, OBTLAST_EVENT, CURRENT_STATUS)

    IMPLICIT NONE

    INTEGER         :: DAL_DS
    INTEGER         :: EVT_TYPE
    INTEGER         :: NUM_MISS_PACK
    INTEGER         :: NUM_CONTINUOUS
    INTEGER(KIND=8),pointer,dimension(:) :: OBTFIRST_EVENT
    INTEGER(KIND=8),pointer,dimension(:) :: OBTLAST_EVENT
    INTEGER         :: CURRENT_STATUS
    INTEGER         :: DAL3IBIS_FIND_EVENT_GAPS

    INTEGER         :: status

    status=ISDC_OK

    if ( current_status/=ISDC_OK ) then
       DAL3IBIS_FIND_EVENT_GAPS=current_status
       return
    end if

    do
       !* Call to the function that actually does the job                    */
       status=DAL3IBIS_calculate_Event_Gaps(DAL_DS,EVT_TYPE,num_miss_pack,num_continuous,status)

       allocate(OBTfirst_event(1:num_continuous))
       allocate(OBTlast_event (1:num_continuous))
          
       !* Call to the function that actually does the job                    */
       status=DAL3IBIS_find_Gaps_OBT(DAL_DS,EVT_TYPE,ADDROF(OBTfirst_event(1)),ADDROF(OBTlast_event(1)),status);

       exit
    end do

    DAL3IBIS_FIND_EVENT_GAPS=status


  END FUNCTION DAL3IBIS_FIND_EVENT_GAPS


  !
  ! Functions below must treat the NULL "&" string 
  ! 

  FUNCTION DAL3IBIS_SELECT_EVENTS(DAL_DS, IBIS_NUM, COLSEL, OBTNUMS, OBTFIRST, OBTLAST, ROWFILTER, CURRENT_STATUS)

    INTEGER         :: DAL_DS
    INTEGER         :: IBIS_NUM
    INTEGER         :: COLSEL
    INTEGER         :: OBTNUMS
    INTEGER(KIND=8) :: OBTFIRST(*)
    INTEGER(KIND=8) :: OBTLAST(*)
    CHARACTER*(*)   :: ROWFILTER
    INTEGER         :: CURRENT_STATUS
    INTEGER         :: DAL3IBIS_SELECT_EVENTS

    INTEGER         :: CROWFILTER, R, R2

    R = CURRENT_STATUS

    IF (ISDC_OK == R) THEN

       R = DALMAKECSTRING(ROWFILTER, CROWFILTER, R)

       IF (ISDC_OK == R) THEN

          R = DAL3IBIS_SELECT_EVENTS_C(DAL_DS, IBIS_NUM, COLSEL, OBTNUMS, OBTFIRST, OBTLAST, CROWFILTER, R)

       ENDIF

       ! use following construction to avoid warnings on Linux
       ! about R2 being assigned to but not subsequently used
       R2 = R
       R = DALSAFREE1STR(CROWFILTER)
       R = R2

    ENDIF

    DAL3IBIS_SELECT_EVENTS = R

   END FUNCTION DAL3IBIS_SELECT_EVENTS


END MODULE DAL3IBIS_F90_API_LOCAL


MODULE DAL3IBIS_F90_API

USE DAL3IBIS_F90_API_LOCAL, ISDC_OK_3IBIS => ISDC_OK
USE DAL3IBIS_ICA_F90_API  , ISDC_OK_3IBISICA => ISDC_OK

INTEGER, PARAMETER :: DAL3IBIS_F90_API_HAPPY_COMPILER = 0

END MODULE DAL3IBIS_F90_API

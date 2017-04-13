/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                              C PROTOTYPE                                  */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse, Nicolas Produit              */
/*  Date:    13 August 2003                                                  */
/*  Version: 4.3.0                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  13.08.2003 V 4.3.0                                                       */
/*  ==================                                                       */
/*  add routine DAL3IBIScalculateAllEventGaps                                */
/*  add routine DAL3IBISfindAllEventGaps                                     */
/*                                                                           */
/*  06.11.2001 V 3.5.6                                                       */
/*  ==================                                                       */
/*  05.11: SPR0725: Fixed a bug making "DAL3IBISfindEventGaps" read bad rows */
/*                                                                           */
/*  28.09.2001 V 3.5.5                                                       */
/*  ==================                                                       */
/*  28.09: SPR0475: Moved NULL "&" string treatment into F90                 */
/*                                                                           */
/*  09.08.2001 V 3.5.1                                                       */
/*  ==================                                                       */
/*  23.07: SPR0524: Fixed wrong F90 interface to "DAL3IBISfindGapsOBT"       */
/*                                                                           */
/*  21.05.2001 V 3.5.0                                                       */
/*  ==================                                                       */
/*  17.05: SPR0444: Fix access problems to isolated data structures          */
/*  16.05: SPR0455: Fixed test data, which were not compatible anymore       */
/*                  Added "TESTING" script                                   */
/*                                                                           */
/*  02.05.2001 V 3.4.2                                                       */
/*  ==================                                                       */
/*  01.05: SPR0389: Removed multiple definition of a variable                */
/*  01.05: SPR0392: Implemented a multipart SPR: Fixed wrong extension name, */
/*                  missing symbols, treatment of NO_OBT, missing "break" in */
/*                  switch, insufficient alloc size, wrong selection, bad    */
/*                  pixel position                                           */
/*                                                                           */
/*  09.04.2001 V 3.4.1                                                       */
/*  ==================                                                       */
/*  09.04: SPR0371: Fixed a typo in the "DAL3IBISicaGetSize" function        */
/*                                                                           */
/*  08.04.2001 V 3.4.0                                                       */
/*  ==================                                                       */
/*  08.04: SCR0180: The ICA APIs have been completely revised. This part of  */
/*                  DAL3IBIS is now isolated at the file level to ease the   */
/*                  concurrent development.                                  */
/*  20.03: SPR0219: "DAL3IBISfindEventGaps" returns "GAPS_NO_OBT" if no PRP  */
/*                  data can be found.                                       */
/*  28.02: SPR0235: Fixed a bug in the determination of valid levels         */
/*  27.03: SPR0337: Obsolete "OBTIME" symbols replaced by "OBTime"           */
/*                                                                           */
/*  23.12.2000 V 3.3.0                                                       */
/*  ==================                                                       */
/*  23.12: SCR0126: The "SELECT_FLAG" property is now part of COR data       */
/*  18.12: SPR0136: "DAL3IBISfindEventGaps" now returns "NO_EVENTS" when     */
/*                  called without event data                                */
/*                                                                           */
/*  26.09.2000 V 3.2.0                                                       */
/*  ==================                                                       */
/*  1. Implement the OBT selection optimization from DAL3GEN 3.2             */
/*  2. Fixed a bug in "DAL3IBISfindGapsOBT"                                  */
/*  3. Catches internal DAL3GEN error codes for events                       */
/*                                                                           */
/*  30.05.2000 V 3.0.1                                                       */
/*  ==================                                                       */
/*  1. Fixed wrong backward-compatibility symbol                             */
/*                                                                           */
/*  11.05.2000 V 3.0.0                                                       */
/*  ==================                                                       */
/*  1. Removed the event treatment from DAL3IBIS and put it in DAL3GEN       */
/*  2. Made the header file isdcroot-friendly                                */
/*  3. For consistency, the "IBIS_evpar" enumaration is now called           */
/*     "IBIS_params"                                                         */
/*  4. Uses now "DAL3 Lists" to speed up processing                          */
/*  5. Fixed "bugsinos" in the bindings                                      */
/*  6. Removed DAL3 subsets from the sample programs                         */
/*  7. Added a parameter to the C version of "DAL3IBISgetEventPackets"       */
/*                                                                           */
/*  21.02.2000 V 2.2.5                                                       */
/*  ==================                                                       */
/* 1. The library uses now Makefile-2.0.1                                    */
/* 2. The library requires now DAL3GEN 2.4                                   */
/* 3. The library now installs "dal3ibis_f90_api_local.mod" for some non-SUN */
/*    F90 compilers                                                          */
/* 4. The library now installs the User's Manual in the help area            */
/* 5. Implements the new ISDCLevel concept from TN020                        */
/* 6. The ICA functions have been modified to handle the new template for    */
/*    the noisy pixels map                                                   */
/* 7. The different switch lists are now stored in two separate Data         */
/*    Structure                                                              */
/* 8. Added a F90 interface to DAL3IBISgetPixStatusMap                       */
/*                                                                           */
/*  29.10.1999 V 2.2.1                                                       */
/*  ==================                                                       */
/* 1. Fixed a bug that prevented the reading of some COMPTON event           */
/*    parameters.                                                            */
/*                                                                           */
/*  01.09.1999 V 2.2.0                                                       */
/*  ==================                                                       */
/*  1. New DAL3IBIS APIs for ICA are included (written by L.Lerusse)         */
/*  2. Correct a small bug that prevents the correct reading of COMPTON mode */
/*     Delta-Times.                                                          */
/*  3. New "samplec_ica.c" and samplef90_ica.f90" executables and test data  */
/*                                                                           */
/*  22.08.1999 V 2.1.0                                                       */
/*  ==================                                                       */
/* 1. The library now uses DAL 1.3 and Makefile-1.3.1                        */
/* 2. Changed variable and parameter names in accordance with the CTS.       */
/*    Defined new symbols for parameters.                                    */
/* 3. The APIs do not create automatically an IBIS_SUBSET anymore.           */
/* 4. Event table descriptions are now static global variables, and they     */
/*    are more widely used.                                                  */
/* 5. Better internal error handling. Now intermediate subsets are properly  */
/*    deleted in case of error.                                              */
/* 6. The event access APIs now work with a single data structure.           */
/* 7. Fixed a bug in "DAL3IBIScloseEvents" API, which tried to close non-    */
/*    existing data structures.                                              */
/* 8. Some bug fixes related to allocation of internal buffers.              */
/* 9. Changed "samplec" and "samplef90" so that the user can really "play"   */
/*    with it.                                                               */
/*                                                                           */
/*  18.06.1999 V 2.0.1                                                       */
/*  27.05.1999 V 2.0.0                                                       */
/*                                                                           */
/*  10.03.1999 V 1.0.0                                                       */
/*                                                                           */
/*****************************************************************************/

#include <dal3gen.h>
#include <dal3ibis_version.h>

#ifndef DAL3IBIS_INCLUDE
#define DAL3IBIS_INCLUDE



	/* DAL3IBIS ERROR                                                    */

#define DAL3IBIS_MAX_SSC             16383 /* SSCs roll over at 16384 */
#define DAL3IBIS_ERROR_CODE_BASE      -26000

	/* -26001 : OBJECT does not contain any IBIS events                  */
#define DAL3IBIS_NO_IBIS_EVENTS        (DAL3IBIS_ERROR_CODE_BASE-1)
        /* -26002 : Parameter inexistent in this object                      */
#define DAL3IBIS_INEXISTENT_PARAMETER    (DAL3IBIS_ERROR_CODE_BASE-2)
        /* -26003 : Parameter cannot exist for this event type               */
#define DAL3IBIS_INCOMPATIBLE_PARAMETER  (DAL3IBIS_ERROR_CODE_BASE-3)
        /* -26004 : Code will not work if more then one children */
#define DAL3IBIS_TOO_MANY_CHILDREN  (DAL3IBIS_ERROR_CODE_BASE-4)

        /* -26010 : The group has a wrong structure                          */
#define DAL3IBIS_INVALID_GROUP         (DAL3IBIS_ERROR_CODE_BASE-10)

        /* -26101 : The input DS must be a SWG                               */
#define DAL3IBIS_NOT_A_SWG             (DAL3IBIS_ERROR_CODE_BASE-101)

        /* -26201 : Cannot find the DS containing the OBTs of the gaps       */
#define DAL3IBIS_GAPS_NO_OBT           (DAL3IBIS_ERROR_CODE_BASE-201)
#define DAL3IBIS_MISMATCH_SRW          (DAL3IBIS_ERROR_CODE_BASE-301)
#define DAL3IBIS_MISMATCH_CHILDREN      (DAL3IBIS_ERROR_CODE_BASE-302)

#define DAL3IBIS_ERR_MEMORY      (DAL3IBIS_ERROR_CODE_BASE-400)
#define DAL3IBIS_NO_VALID_HK      (DAL3IBIS_ERROR_CODE_BASE-401)


#define SEC_DELTA_MIN       25


typedef enum {
  ISGRI_EVTS= 0, PICSIT_SGLE= 1, PICSIT_MULE = 2, COMPTON_SGLE= 3, COMPTON_MULE= 4
} IBIS_type;

typedef enum {
  PICSIT_SISH     = 0,  PICSIT_SIMH   = 1, PICSIT_POLH =  2
} IBIS_HistType;


#ifdef F90_PROTOTYPING

#define IBIS_TYPE              int
#define IBIS_evpar             int

#else
#ifdef __CINT__

typedef IBIS_type              IBIS_TYPE;
typedef Event_params           IBIS_evpar;

#else

#define IBIS_TYPE              IBIS_type
#define IBIS_evpar             Event_params

#endif

#endif

#ifdef __CINT__
typedef IBIS_evpar             IBIS_evprop;
#else
#define IBIS_evprop             IBIS_evpar
#endif

#define COMP_TIME                  TIME_TAG

#define ISGRI_MAX_DIFF_OBT         1
#define PICSIT_MAX_DIFF_OBT        1
#define COMPTON_MAX_DIFF_OBT       1


/*****************************************************************************/
/*                                                                           */
/* Prototype for      dal3ibis_events.c                                      */
/*                                                                           */
/*****************************************************************************/

#ifndef __CINT__
#ifdef __cplusplus
extern "C" {
#endif
#endif

int DAL3IBISshowAllEvents(DAL_ELEMENTP InputDS,
			  long         IBISnum[5],
			  int          current_status);
  /*
    Given a DAL data structure, "DAL3IBISshowAllEvents" finds the number of all
    events of all types
  */

int DAL3IBISselectEvents(DAL_ELEMENTP           InputDS,
			 IBIS_TYPE              EvType,
			 ISDC_LEVEL             ColSel,
			 int                    NumIntervals,
			 OBTime                *gtiStart,
			 OBTime                *gtiStop,
			 const DAL_STRINGHANDLE RowFilter,
			 int                    current_status);
  /*
    Given a DAL data structure, "DAL3IBISselectEvents" extracts
    selected events
  */

int DAL3IBISgetNumEvents(long *NumEvents,
			 int   current_status);
  /*
    Given a DAL data structure, "DAL3IBISgetNumEvents" retrieves the
    number of selected events
  */

int DAL3IBISgetEvents(IBIS_evpar    ColID,
		      dal_dataType *ColType,
		      DAL_VOIDP     ColBuff,
		      int           current_status);
  /*
    Given a DAL data structure, "DAL3IBISgetEvents" retrieves the
    content of the column "colID" for the selected events
  */

int DAL3IBISgetEventsBins(IBIS_evpar    ColID,
			  dal_dataType *ColType,
			  long          StartBin,
			  long          EndBin,
			  DAL_VOIDP     ColBuff,
			  int           current_status);
  /*
    Given a DAL data structure, "DAL3IBISgetEvents" retrieves the
    content of the column "colID" for the selected events from rows
    "startBin" to "endBin"
  */

int DAL3IBIScloseEvents(int current_status);
  /*
    "DAL3IBIScloseEvents" closes the IBIS related data structures
    and frees the associated memory
  */

int DAL3IBISgetNumEventPackets(DAL_ELEMENTP InputDS,
			       IBIS_type    EvType,
			       int         *NumPackets,
			       int          current_status);
  /*
    "DAL3IBISgetNumEventPackets" returns the number of packets containing
    events found in the given Science Window.
  */

int DAL3IBISgetEventPackets(DAL_ELEMENTP InputDS,
			    IBIS_type    EvType,
			    int         *SSC,
			    int         *PartLOBT,
			    OBTime      *LOCAL_OBT,
			    int         *RowBegin,
			    int         *RowNum,
			    int         *FirstNum,
			    int         *SecondNum,
			    int         *CovflwNum,
			    int          current_status);
  /*
    "DAL3IBISgetEventPackets" returns the properties of the packets found
    in the given Science Window.
  */

int DAL3IBISgetNumDummy(DAL_ELEMENTP InputDS,
			IBIS_type    EvType,
			int         *NumDummy,
			int          current_status);
  /*
    "DAL3IBISgetNumDummy" returns the number of dummy events of all kinds
    found in the given Science Window.
  */

int DAL3IBISgetDummy(DAL_ELEMENTP InputDS,
		     IBIS_type    EvType,
		     int         *SSC,
		     int         *PartLOBT,
		     int         *Position,
		     int         *FirstDummy,
		     unsigned long *SecondDummy,
		     int         *CounterOvflw,
		     int          current_status);
  /*
    "DAL3IBISgetDummy" returns the number of dummy events of all kinds
    found in the given Science Window.
  */

int DAL3IBISfindEventGaps(DAL_ELEMENTP DAL_DS,
			  IBIS_type    evType,
			  int         *num_miss_pack,
			  int         *num_continuous,
			  OBTime     **OBTfirst_event,
			  OBTime     **OBTlast_event,
			  int          current_status);
  /*
    "DAL3IBISfindEventGaps" finds the possible gaps in the telemetry
    and returns the OBT of the last event before the gaps and of the first event
    after the gap
  */

int DAL3IBISfindAllEventGaps(DAL_ELEMENTP DAL_DS,
			     IBIS_type    evType,
			     int         *num_miss_pack,
			     int         *num_continuous,
			     OBTime     **OBTfirst_event,
			     OBTime     **OBTlast_event,
			     int          current_status);
  /*
    "DAL3IBISfindEventGaps" finds the possible gaps in the telemetry or in the 
    data itself
    and returns the OBT of the last event before the gaps and of the first event
    after the gap
  */

/******************************************************************************/
/*          The following functions must not be called directly               */
/*          They are used by the Fortran 90 interface or by C internally      */
/******************************************************************************/

int DAL3IBISgetEventPacketsF90(DAL_ELEMENTP InputDS,
			       IBIS_type    EvType,
			       int         *SSC,
			       int         *PartLOBT,
			       int         *RowBegin,
			       int         *RowNum,
			       int         *FirstNum,
			       int         *SecondNum,
			       int         *CovflwNum,
			       DAL_VOIDP    DAL3_NULL,
			       int          current_status);
  /*
    "DAL3IBISgetEventPacketsF90" is an intermediate functions that is not
    supposed to be used directly, but from the DAL3IBIS_GET_EVENT_PACKETS
    F90 function
  */


int DAL3IBIScalculateEventGaps(DAL_ELEMENTP InputDS,
			       IBIS_type    EvType,
			       int         *NumChildren,
			       int         *NumMissPack,
			       int         *NumIntervals,
			       int          current_status);
  /*
    "DAL3IBIScalculateEventGaps" finds the possible gaps in the telemetry 
    and returns the location of the events at the boundaries
  */

int DAL3IBIScalculateAllEventGaps(DAL_ELEMENTP InputDS,
			       IBIS_type    EvType,
			       int         *NumChildren,
			       int         *NumMissPack,
			       int         *NumIntervals,
			       int          current_status);
  /*
    "DAL3IBIScalculateEventGaps" finds the possible gaps in the telemetry 
    and returns the location of the events at the boundaries
  */

int DAL3IBISfindGapsOBT(DAL_ELEMENTP InputDS,
			IBIS_type    EvType,
			int          NumChildren,
			OBTime      *OBTstart,
			OBTime      *OBTstop,
			int          current_status);
  /*
    "DAL3IBISfindGapsOBT" calculates the OBTs corresponding to the beginnings
    and ends of the continuous "plages"
  */

#define DAL_GC_MAX_ALLOCATIONS 100
#define DAL_GC_RESOURCE_KIND char
#define DAL_GC_MEMORY_RESOURCE 0
#define DAL_GC_DAL_OBJECT_RESOURCE 1

#ifndef __CINT__
#ifdef __cplusplus
}
#endif
#endif

/* ICA functions of DAL3IBIS have a separate header file                      */
#include <dal3ibis_ica.h>

#endif

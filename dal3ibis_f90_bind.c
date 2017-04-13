/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                              F90 BINDINGS                                 */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse                               */
/*  Date:    28 September 2001                                               */
/*  Version: 3.5.5                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  28.09.2001 V 3.5.5                                                       */
/*  ==================                                                       */
/*  28.09: SPR0475: Moved NULL "&" string treatment into F90                 */
/*                                                                           */
/*  09.08.2001 V 3.5.1                                                       */
/*  ==================                                                       */
/*  23.07: SPR0524: Fixed wrong F90 interface to "DAL3IBISfindGapsOBT"       */
/*                                                                           */
/*  11.05.2000 V 3.0.0                                                       */
/*  ==================                                                       */
/*  5. Fixed "bugsinos" in the bindings                                      */
/*                                                                           */
/*  01.09.1999 V 2.2.0                                                       */
/*  ==================                                                       */
/*  1. New DAL3IBIS APIs for ICA are included (written by L.Lerusse)         */
/*                                                                           */
/*  22.08.1999 V 2.1.0                                                       */
/*  ==================                                                       */
/* 1. The library now uses DAL 1.3 and Makefile-1.3.1                        */
/* 2. Changed variable and parameter names in accordance with the CTS.       */
/*    Defined new symbols for parameters.                                    */
/*                                                                           */
/*  18.06.1999 V 2.0.1                                                       */
/*  27.05.1999 V 2.0.0                                                       */
/*                                                                           */
/*  10.03.1999 V 1.0.0                                                       */
/*                                                                           */
/*****************************************************************************/

#define F90_PROTOTYPING
#define DAL_F90_PROTOTYPING

#include <dal3ibis.h>

#include <cfortran_isdc.h>


/*                            DAL3  IBIS  LIBRARY                            */
/*                            EVENT MANIPULATIONS                            */

FCALLSCFUN3(INT, DAL3IBISshowAllEvents, DAL3IBIS_SHOW_ALL_EVENTS, dal3ibis_show_all_events,
	    INT, LONGV, INT)

FCALLSCFUN8(INT, DAL3IBISselectEvents, DAL3IBIS_SELECT_EVENTS_C, dal3ibis_select_events_c,
	    INT, INT, INT, INT, LONGV, LONGV, INT, INT)

FCALLSCFUN2(INT, DAL3IBISgetNumEvents, DAL3IBIS_GET_NUM_EVENTS, dal3ibis_get_num_events,
	    LONGV, INT)

FCALLSCFUN4(INT, DAL3IBISgetEvents, DAL3IBIS_GET_EVENTS, dal3ibis_get_events,
	    INT, INTV, INT, INT)

FCALLSCFUN6(INT, DAL3IBISgetEventsBins, DAL3IBIS_GET_EVENTS_BINS, dal3ibis_get_events_bins,
	    INT, INTV, INT, INT, INT, INT)

FCALLSCFUN1(INT, DAL3IBIScloseEvents, DAL3IBIS_CLOSE_EVENTS, dal3ibis_close_events,
	    INT)

FCALLSCFUN4(INT, DAL3IBISgetNumEventPackets, DAL3IBIS_GET_NUM_EVENT_PACKETS, dal3ibis_get_num_event_packets,
	    INT, INT, INTV, INT)

FCALLSCFUN11(INT, DAL3IBISgetEventPacketsF90, DAL3IBIS_GET_EVENT_PACKETS_F90, dal3ibis_get_event_packets_f90,
	     INT, INT, INTV, INTV, INTV, INTV, INTV, INTV, INTV, INT, INT)

FCALLSCFUN4(INT, DAL3IBISgetNumDummy, DAL3IBIS_GET_NUM_DUMMY, dal3ibis_get_num_dummy,
	    INT, INT, INTV, INT)

     /*
obsolete
FCALLSCFUN10(INT, DAL3IBISgetDummyF90, DAL3IBIS_GET_DUMMY_F90, dal3ibis_get_dummy_f90,
	     INT, INT, INTV, INTV, INTV, INTV, INTV, INTV, INTV, INT)
     */
FCALLSCFUN6(INT, DAL3IBIScalculateEventGaps, DAL3IBIS_CALCULATE_EVENT_GAPS, dal3ibis_calculate_event_gaps,
	    INT, INT, INTV, INTV, INTV, INT)

FCALLSCFUN6(INT, DAL3IBISfindGapsOBT, DAL3IBIS_FIND_GAPS_OBT, dal3ibis_find_gaps_obt,
	    INT, INT, INT, LONGV, LONGV, INT)

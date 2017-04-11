/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                              F90 BINDINGS                                 */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse                               */
/*  Date:    8 April 2001                                                    */
/*  Version: 3.4.0                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  08.04.2001 V 3.4.0 First version                                         */
/*  ==================                                                       */
/*  08.04: SCR0180: The ICA APIs have been completely revised. This part of  */
/*                  DAL3IBIS is now isolated at the file level to ease the   */
/*                  concurrent development.                                  */
/*                                                                           */
/*****************************************************************************/

#define F90_PROTOTYPING
#define DAL_F90_PROTOTYPING

#include <dal3ibis.h>

#include <cfortran_isdc.h>


/* The following functions are FORTRAN 90 interfaces for DAL3IBIS functions  */
/* written by Laurent Lerusse                                                */

FCALLSCFUN6(INT, DAL3IBISicaTransfCoord, DAL3IBIS_ICA_TRANSF_COORD, dal3ibis_ica_transf_coord,
	    INT, INT, INT, INTV, INTV, INT)

FCALLSCFUN5(INT, DAL3IBISgetSizeNoisyMaps, DAL3IBIS_GET_SIZE_NOISY_MAPS, dal3ibis_get_size_noisy_maps,
	    INT, LONGLONG, LONGLONG, LONGV, INT)
 
     /* Comment out as per screw 841 */
     /* FCALLSCFUN10(INT, DAL3IBISgetNoisyMaps, DAL3IBIS_GET_NOISY_MAPS, dal3ibis_get_noisy_maps,
	INT, LONGLONG, LONGLONG,  LONGLONGV, BYTEV, SHORTV, BYTEV, BYTEVV, LONGV, INT)
     */
FCALLSCFUN5(INT,DAL3IBISgetSizeSwitchList, DAL3IBIS_GET_SIZE_SWITCH_LIST, dal3ibis_get_size_switch_list,
	     INT, LONGLONG, LONGLONG, LONGV, INT) 			

FCALLSCFUN10(INT, DAL3IBISgetSwitchList, DAL3IBIS_GET_SWITCH_LIST, dal3ibis_get_switch_list,  
	     INT, LONGLONG, LONGLONG, BYTEV, BYTEV, LONGLONGV, LONGLONGV, BYTEVV, LONGV, INT)

FCALLSCFUN5(INT, DAL3IBISselectCtxt, DAL3IBIS_SELECT_CTXT, dal3ibis_select_ctxt,
	    INT, LONGLONGV, INT, INTV, INT)
// suspicipusly passing only LONGLONGV even if not?

FCALLSCFUN5(INT, DAL3IBISctxtGetImaPar, DAL3IBIS_CTXT_GET_IMA_PAR, dal3ibis_ctxt_get_ima_par,
	    INT, LONGLONGV, INT, INT, INT)

FCALLSCFUN7(INT, DAL3IBISctxtGetTblPar, DAL3IBIS_CTXT_GET_TBL_PAR,dal3ibis_ctxt_get_tbl_par,
	    INT, LONGLONGV, INT, STRING, INT, INT, INT)

FCALLSCFUN9(INT, DAL3IBISicaIsgriNoisEff, DAL3IBIS_ICA_ISGRI_NOIS_EFF, dal3ibis_ica_isgri_nois_eff,
            INT, INT, LONGLONGV, LONGLONGV, DOUBLEVV, LONGV, LONGLONGV, INTVV, INT)

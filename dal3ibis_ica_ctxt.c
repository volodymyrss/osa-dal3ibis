/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                               ICA CONTEXT                                 */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse                               */
/*  Date:    2 May 2001                                                      */
/*  Version: 3.4.2                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  02.05.2001 V 3.4.2                                                       */
/*  ==================                                                       */
/*  01.05: SPR0392: Implemented a multipart SPR: Fixed wrong extension name, */
/*                  missing symbols, treatment of NO_OBT, missing "break" in */
/*                  switch, insufficient alloc size, wrong selection, bad    */
/*                  pixel position                                           */
/*                                                                           */
/*  08.04.2001 V 3.4.0 First version                                         */
/*  ==================                                                       */
/*  08.04: SCR0180: The ICA APIs have been completely revised. This part of  */
/*                  DAL3IBIS is now isolated at the file level to ease the   */
/*                  concurrent development.                                  */
/*                                                                           */
/*****************************************************************************/

#include "dal3ibis.h"

/****************************************************************************

NOTE : The 'dataKind' parameter define the part of the IBIS context you want 
       to retrieve. It defines if you expect to select the ISGRI context or 
       the PICsIT one. It defines if you want to get the pixels status or 
       the gain of the ASICs. 

       It is declared as an enumeration in dal3ibis.h

       The possible values are :

		ISGRI_CTXT      ->	ISGRI context
		ISGRI_PIX_GAIN  -> 	pixels gains, 
		ISGRI_PIX_LTHR  ->	pixels low thresholds, 
		ISGRI_PIX_STA   ->	pixels status,
		ISGRI_PIX_TEST  ->	pixels test status,
		ISGRI_ASIC_GAIN ->	ASICs gains,
		ISGRI_ASIC_HTHR -> 	ASICs high thresholds,
		ISGRI_MODP      -> 	ISGRI modules parameters, 

		PICSIT_CTXT     ->	PICsIT context, 
		PICSIT_PIX_STA  ->	pixels status,
		PICSIT_MODP     ->	PICsIT modules parameters,
		PICSIT_DETP     ->	PICsIT detector parameters

****************************************************************************/

/****************************************************************************
 Function : 	ica_readCtxtImaPar                                        
                                                                          
 Description :	
 	This static function read the pixels or array parameter from the 
	image extension which hold them. No selection of any kind are 
	applied here. 'dataKind' define which part of the image has to be 
	read. 

 Parameter argument :                                                      
                                                                          
   name           type           description
   --------------|--------------|----------------------------------------                              
   ctxtPtr	  dal_element*	 Pointer to the ISGFRI context index or 
   				 to the top group of the context or to the 
				 low threshold table.
   dataKind	  IBISDS_Type	 Define which part of the image have to be 
   			         read.
   dataBuff 	  DAL3_Byte *    Data buffer. It has to be previously 
   				 allocated. with at least 16384 or 4 
   status         int            Error code.
                                                                          
***************************************************************************/
static int DAL3IBISicaReadCtxtImaPar( dal_element *parPtr, 
				      IBISDS_Type  dataKind, 
				      DAL3_Byte   *dataBuff, 
				      int          status){
  long startValue[3] = {1,1,1};
  long endValue[3] = {ISGRI_SIZE,ISGRI_SIZE,1}; 
  dal_dataType dataType = DAL_BYTE;
  long numValues;

  if(status != ISDC_OK) return(status);
  switch (dataKind) {
    case ISGRI_PIX_GAIN	:
    	startValue[2] = 1;
	endValue[2]   = 1;
    	break;
    case ISGRI_PIX_LTHR	:
    	startValue[2] = 2;
	endValue[2]   = 2;
    	break;
    case ISGRI_PIX_STA  :
     	startValue[2] = 3;
	endValue[2]   = 3;
     	break;
    case ISGRI_PIX_TEST	:
    	startValue[2] = 4;
	endValue[2]   = 4;
    	break;
    case ISGRI_ASIC_GAIN:
    	startValue[2] = 1;
	endValue[2]   = 1;
	endValue[0]   = ISGRI_SIZE/2;
	endValue[1]   = ISGRI_SIZE/2;
    	break;
    case ISGRI_ASIC_HTHR:
    	startValue[2] = 2;
	endValue[2]   = 2;
	endValue[0]   = ISGRI_SIZE/2;
	endValue[1]   = ISGRI_SIZE/2;
    	break;
    case PICSIT_PIX_STA	:
    	startValue[2] = 1;
 	endValue[2]   = 1;
	endValue[0]   = PICSIT_SIZE;
	endValue[1]   = PICSIT_SIZE;
    	break;
    default:
    	status = DAL3IBIS_BAD_CTXT_PARAMETER;
    	break;
  }

  status = DALarrayGetSection( parPtr, 3, startValue, endValue, &dataType, &numValues, dataBuff, status);

  if( status==ISDC_OK &&
      ( dataType != DAL_BYTE || (numValues != 16384  && numValues != 4096)))
    status = DAL3IBIS_BAD_CTXT_STRUCT;
  return(status);
}
     

/****************************************************************************
 Function :  DAL3IBIselectCtxt                                         
                                                                          
 Description : This function select the context of ISGRI or PICsIT which is 
 	       just before the limit Time. 
	       The context time is returned in the limTime parameter.
	       dataKind allow to make a selection on the type of dal_element 
	       is passed. The input can be any CTXT DS_Type. 

 Parameter argument :                                                      
                                                                          
   name           type           description
   --------------|--------------|----------------------------------------                              
   ctxtPtr	  dal_element*	 Pointer to the ISGFRI context index or 
   				 to the top group of the context or to the 
				 low threshold table.
   limTime 	  OBTime *	 I/O Time of the context, in input is the 
   				 requested time, in output it is the real 
				 time.
   dataKind	  IBISDS_Type	 Define which context it has to be. 
   				 Can be ISGRI_CTXT or PICSIT_CTXT. The other
				 part of these two context table can also be 
				 passed. 
   lowThres 	  DAL3_Byte **   Low threshold table (DAL3_Byte[128][128]) 
   status         int            Error code.
                                                                          
***************************************************************************/

int DAL3IBISselectCtxt(dal_element  *ctxtPtr, 
		       OBTime       *limTime,
		       IBISDS_Type   dataKind, 
		       dal_element **slctCtxtPtr,
		       int           status){

  char *obt_key = ICA_OBT_CTXT_REF_KEY; 
  char **columns;
  char slctStr[SELECT_STRING_LENGTH];
  char obtStr[DAL_MED_STRING];

  int numSelected = 0;
  IBISDS_Type input;

  dal_element *subIdxPtr;

  /* 
   *   Start of the function.
   */
  if(status != ISDC_OK) return(status);

  columns = malloc(2*sizeof(char*)); 
  columns[0] = obt_key;

  /*  
   *  Determine the input type. It can be the pixel switches list or an index of it. 
   */
  switch (dataKind) {
    case ISGRI_CTXT	: 
    case ISGRI_PIX_GAIN	:
    case ISGRI_PIX_LTHR	:
    case ISGRI_PIX_STA  :
    case ISGRI_PIX_TEST	:
    case ISGRI_ASIC_GAIN:
    case ISGRI_ASIC_HTHR: 
    case ISGRI_MODP	:
	  status = DAL3IBISindexOrMember(ctxtPtr, ICA_EXTNAME_ISGR_CTXT_GRP, &input, status);
	  break;
    case PICSIT_CTXT	: 
    case PICSIT_PIX_STA :
    case PICSIT_DETP	:
    case PICSIT_MODP	:
	  status = DAL3IBISindexOrMember(ctxtPtr, ICA_EXTNAME_PICS_CTXT_GRP, &input, status);
	  break;
    default:
    	status = DAL3IBIS_BAD_CTXT_PARAMETER;
    	break;
  }
  if(status  != ISDC_OK) return(status);

  /*
   *  Get the context. 
   */
  switch (input) {
  case IBIS_DS	: 
    *slctCtxtPtr = ctxtPtr;
    break;
  case IBIS_INDEX	:
    if (limTime == NULL) return(DAL3GEN_INVALID_PARAMETER);
    if (*limTime != DAL3_NO_OBTIME) {
      status = DAL3GENformatOBT( *limTime, obtStr, status);
      sprintf( slctStr, "%s <= '%s'", columns[0], obtStr);        
    } else {   
      status = DAL3GENformatOBT( (OBTime)0, obtStr, status);
      sprintf( slctStr, "%s >= '%s'", columns[0], obtStr);        
    }
    status = DAL3GENindexFindMember(ctxtPtr,NULL,slctStr,&numSelected,&subIdxPtr, status);
    if(status != ISDC_OK) return(status);
    if( numSelected  <= 0 ){ 
      status = DAL3IBIS_INDEX_EMPTY;
      *slctCtxtPtr = NULL;
      return(status);
    } else if( numSelected  == 1 ){
      /* get the element */
      *slctCtxtPtr = subIdxPtr;
    } else if(numSelected > 1){	 
      /* sort the table */
      status = DALtableSortRows(subIdxPtr, columns, 1, DAL_SORT_DESCENDING, DAL_L2R, status); 
      /* get the element */
      status = DAL3GENindexGetMember(subIdxPtr, NULL, 1, slctCtxtPtr, status);
      status = DALobjectClose(subIdxPtr, DAL_DELETE, status);
    }
    break;
  default: /* SPR 830 */
    status = DAL3IBIS_BAD_CTXT_PARAMETER;
    break;
    
  }
  status = DAL3GENattributeGetOBT( *slctCtxtPtr, obt_key, limTime, NULL, status); 
  return(status);
}


int DAL3IBISctxtGetImaPar( dal_element *ctxtPtr, 
			   OBTime      *limTime,
			   IBISDS_Type  dataKind, 
			   void        *dataBuff, 
			   int          status){
  dal_element *ctxtDsPtr;
  dal_element *parPtr;
  char extname[DAL_MED_STRING];
  /* 
   *   Start of the function.
   */
  if(status != ISDC_OK) return(status);
  if(ctxtPtr == NULL) return(DAL3GEN_INVALID_PARAMETER);
  switch (dataKind) {
  case ISGRI_PIX_GAIN	:
  case ISGRI_PIX_LTHR	:
  case ISGRI_PIX_STA  :
  case ISGRI_PIX_TEST	:
    strcpy(extname,ICA_EXTNAME_ISGR_CTXT_PXLP );
    break;
  case ISGRI_ASIC_GAIN:
  case ISGRI_ASIC_HTHR:
    strcpy(extname,ICA_EXTNAME_ISGR_CTXT_ASIP );
    break;
  case PICSIT_PIX_STA	:
    strcpy(extname, ICA_EXTNAME_PICS_CTXT_PXLP );
    break;
  default:
    status = DAL3IBIS_BAD_CTXT_PARAMETER;
    break;
  }
  status = DAL3IBISselectCtxt( ctxtPtr, limTime, dataKind, &ctxtDsPtr, status);
  if(status != ISDC_OK) return(status);
  status = DALobjectFindElement(ctxtPtr, extname, &parPtr, status);
  if(status != ISDC_OK) return(status);
  status = DAL3IBISicaReadCtxtImaPar( parPtr, dataKind, dataBuff, status);  
  return(status); 
}

/* SCREW 1612, replace the function */


int DAL3IBISGetlowthresholdKev( dal_element *ctxtPtr, 
				OBTime      limTime, 
				float       *dataBuff, 
				int         status)
{
  int  z,y;
  float a,b,
    g_4C1[4]= { 1.0265, 1.216,  0.9666, 1.1277 },
    o_4C1[4]= { 1.4435, 1.6641, 1.1983, 1.581  },
    g_4C2[4]= { 1.1834, 1.4482, 1.0929, 1.2867 },
    o_4C2[4]= { 1.6877, 1.69,   1.619,  1.816  };
  DAL3_Byte 
    lowThres[ISGRI_SIZE][ISGRI_SIZE],
    pixGain[ISGRI_SIZE][ISGRI_SIZE];
  dal_element 
    *slctCtxtPtr = NULL;

  do{
    if (status!=ISDC_OK) break;
    status = DAL3IBISselectCtxt(ctxtPtr, &limTime, ISGRI_CTXT, &slctCtxtPtr, 
				status);
    if (status!=ISDC_OK) break;
    /* Read Low Tresholds Map (STEP) */
    status = DAL3IBISctxtGetImaPar(slctCtxtPtr, &limTime,ISGRI_PIX_LTHR, 
				   lowThres, status);
    /* Read Pixel Gain Map */
    status = DAL3IBISctxtGetImaPar(slctCtxtPtr, &limTime,ISGRI_PIX_GAIN, 
				   pixGain, status);
    for (z=0;z<ISGRI_SIZE;z++)
      for (y=0;y<ISGRI_SIZE;y++)
	{
	  /* Dead pixel or NaN value for Pixel Gain (never happened until now) */
	  if( lowThres[z][y]>62 || pixGain[z][y]<0 || pixGain[z][y]>3 )
	    {
	      a=0;
	      b=0;
	    }
	  else
	    { 
	      /* Apply experimental SACLAY laws for ASCICs "4C1" =  MDU4 */
	      if(z>95 && y<64)
		{
		  a= g_4C1[pixGain[z][y]];
		  b= o_4C1[pixGain[z][y]];
		}
	      /* Apply experimental SACLAY laws for ASCICs "4C2" = All MDUs except MDU4 */
	      else
		{
		  a= g_4C2[pixGain[z][y]];
		  b= o_4C2[pixGain[z][y]];
		}
	    }      
	  dataBuff[z*ISGRI_SIZE+y]= a*lowThres[z][y]+b;
	}
  }while (0);

  return status;
}

int DAL3IBISctxtGetTblPar( dal_element *ctxtPtr, 
			   OBTime      *limTime, 
			   IBISDS_Type  dataKind, 
			   const char  *colName, 
			   dal_dataType dataType, 
			   void        *dataBuff, 
			   int          status){
  long numValues;
  dal_element *ctxtDsPtr;
  dal_element *tblPtr;
  char extname[DAL_MED_STRING];

  /* 
   *   Start of the function.
   */
  if(status != ISDC_OK) return(status);
  if(ctxtPtr == NULL) return(DAL3GEN_INVALID_PARAMETER);
  switch (dataKind) {
    case ISGRI_MODP	:
	strcpy(extname,ICA_EXTNAME_ISGR_CTXT_MODP );
    	break;
    case PICSIT_MODP	:
	strcpy(extname,ICA_EXTNAME_PICS_CTXT_MODP );
    	break;
    case PICSIT_DETP	:
	strcpy(extname, ICA_EXTNAME_PICS_CTXT_DETP );
	break;/* SPR 2052 */
    default:
    	status = DAL3IBIS_BAD_CTXT_PARAMETER;
    	break;
  }
  status = DAL3IBISselectCtxt( ctxtPtr, limTime, dataKind, &ctxtDsPtr, status);
  status = DALobjectFindElement(ctxtDsPtr, extname, &tblPtr, status);
  status=DALtableGetNumRows(tblPtr, &numValues, status);
  if(status != ISDC_OK) { return(status); }
  if(numValues > IBIS_NUM_BLOCK || numValues <= 0){ 
  	status =  DAL3IBIS_BAD_CTXT_STRUCT;
	return(status);
  }
  if(  strcmp(colName, "OB_TIME") == 0) {
	status = DAL3GENtableGetOBT( tblPtr, "OB_TIME", 0, &numValues, dataBuff, status);
  } else {
  	status = DALtableGetCol( tblPtr, colName, 0, &dataType, &numValues, dataBuff, status);
  }
  return(status); 
}

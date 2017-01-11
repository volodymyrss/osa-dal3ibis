/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                                ICA NOISY                                  */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse                               */
/*  Date:    2 May 2001                                                      */
/*  Version: 3.4.2                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  02.05.2002 V 4.0.0                                                       */
/*  ==================                                                       */
/*  02.05: SPR0733 - Free memory                                             */
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

/*****************************************************************************

 Static Function :  ica_readSwitchList                                          

 Description : 	
 	Read the ISGRI noisy pixel map data structure. 

 Parameter argument :                                                      

    name           type           description
   --------------|--------------|----------------------------------------                              
   dsPtr	  dal_element*	 Pointer to the ISGRI switch list.
   Y_switch	  DAL3_Byte *    Y position of the pixel.
   Z_switch	  DAL3_Byte *	 Z position of the pixel.
   timeDetect	  OBTime *	 Detection time of the switch.
   timeSwitch	  OBTime *	 Confirmation time of the switch.
   flag_switch	  DAL3_Byte *	 Type of switch.
   *numRows	  long*		 Number of switch to allocate.
   status         int            Error code.
                                                                           
******************************************************************************/	
static int DAL3IBISicaReadNoisyMap(dal_element *dsPtr,
				   OBTime      *blockTime, 
				   DAL3_Byte   *mceId, 
				   DAL3_Word   *periodOn, 
				   DAL3_Byte   *onRange, 
				   DAL3_Byte  (*blockStatus)[IBIS_IBLOCK_LENGTH], 
				   long        *numRows,
				   int          status){
  int i;
  dal_dataType byteType = DAL_BYTE;
  dal_dataType wordType = DAL_USHORT;
  long numValues = IBIS_IBLOCK_LENGTH;
  if(status!=ISDC_OK){ return status; }
  /*  Get the contents of the columns   */
  status = DAL3GENtableGetOBT(dsPtr, NOISY_MAP_OBT_COLNAME, 0,  numRows, blockTime, status);
  status = DALtableGetCol(dsPtr, NOISY_MAP_MCEID_COLNAME, 0, &byteType, numRows, mceId, status);
  status = DALtableGetCol(dsPtr, NOISY_MAP_PERIOD_ON_COLNAME, 0, &wordType, numRows, periodOn, status);
  status = DALtableGetCol(dsPtr, NOISY_MAP_ON_RANGE_COLNAME, 0, &byteType, numRows, onRange, status);
  for(i=1; i <= *numRows; i++){
  	status = DALtableGetColBins(dsPtr, NOISY_MAP_BLOCKS_COLNAME, 0, &byteType, i, i, &numValues,
				    blockStatus[i-1], status);
  }
  return(status);
}

/****************************************************************************

 Function :   DAL3IBISgetSizeNoisyMaps                                         

 Description : Get the maximum size to read the noisy pixel maps 	

 Parameter argument :                                                      

     name           type          I/O  description
   --------------|--------------|---|----------------------------------------                              
   mapPtr	  dal_element*	  I  Pointer to the ISGRI noisy map index or 
   				     to the noisy map DS itself.
   obtStart	  OBTime	  I  Start time of the considered interval
   obtEnd	  OBTime	  I  End time of the considered interval
   noisylength	  long*		  O  Number of switch to allocate.
   status         int            I/O Error code.
                                                                           
***************************************************************************/
int DAL3IBISgetSizeNoisyMaps(dal_element *mapPtr, 
			      OBTime      obtStart, 
			      OBTime      obtEnd, 
			      long       *noisylength, 
			      int         status){
  char *first_key = ICA_OBT_FIRST_KEY;
  char *last_key  = ICA_OBT_LAST_KEY;
  IBISDS_Type input;  
  /* Start of the function. */
  if(status != ISDC_OK) return(status);
  if(mapPtr == NULL) return(DAL3GEN_INVALID_PARAMETER);
  *noisylength = 0;
  /*  Determine the input type. It can be the noisy pixel map or an index of it. */
  status = DAL3IBISindexOrMember(mapPtr, ICA_EXTNAME_ISGR_NOISY_MAP, &input, status);
  /* Get the length of the tables. */
  status = DAL3IBISicaGetSize(mapPtr, input, first_key, last_key, obtStart, obtEnd, noisylength, status);
  return(status);
}

/****************************************************************************

 Function :  DAL3IBISallocateNoisyMap                                          

 Description : 	
 	The function allocate the necessary space for the pixel switches 
	list buffer.

 Parameter argument :                                                      
    name           type           description
   --------------|--------------|----------------------------------------                              
   noisylength 	  long		 Number of switch to allocate.
   status         int            Error code.
                                                                           
***************************************************************************/
int DAL3IBISallocateNoisyMaps(OBTime     **blockTime, 
			      DAL3_Byte  **mceId, 
			      DAL3_Word  **periodOn, 
			      DAL3_Byte  **onRange, 
 			      DAL3_Byte (**blockStatus)[IBIS_IBLOCK_LENGTH], 
			      long         noisylength,
			      int          status){

  if(status != ISDC_OK) return(status);
  if((*blockTime   = realloc(*blockTime  ,noisylength * sizeof(OBTime)))     == NULL ||
     (*mceId       = realloc(*mceId      ,noisylength * sizeof(DAL3_Byte)))  == NULL ||
     (*periodOn    = realloc(*periodOn   ,noisylength * sizeof(DAL3_Word)))  == NULL ||
     (*onRange     = realloc(*onRange    ,noisylength * sizeof(DAL3_Byte)))  == NULL ||
     (*blockStatus = realloc(*blockStatus,IBIS_IBLOCK_LENGTH* noisylength * sizeof(DAL3_Byte*))) == NULL)
    status = DAL_MALLOC_ERROR;
  
  return(status);
}

/****************************************************************************

 Function :  DAL3IBISfreeNoisyMap                                          

 Description : 	
 	The function deallocates the pixel switches list buffer.

 Parameter argument :                                                      
    name           type           description
   --------------|--------------|----------------------------------------                              
   noisylength 	  long		 Number of switch to allocate.
   status         int            Error code.
                                                                           
***************************************************************************/
int DAL3IBISfreeNoisyMaps(OBTime     *blockTime, 
			  DAL3_Byte  *mceId, 
			  DAL3_Word  *periodOn, 
			  DAL3_Byte  *onRange, 
			  DAL3_Byte (*blockStatus)[IBIS_IBLOCK_LENGTH], 
			  int         status){

  if(status != ISDC_OK) return(status);
  free(blockTime);
  free(mceId);
  free(periodOn);
  free(onRange);
  free(blockStatus);

  return(status);
}

/****************************************************************************

 Function :  DAL3IBISallocateListBuffer                                          

 Description : 	
 	The function allocate the necessary space for the pixel switches 
	list buffer.

 Parameter argument :                                                      
    name           type           description
   --------------|--------------|----------------------------------------                              
   noisylength 	  long		 Number of switch to allocate.
   status         int            Error code.
                                                                           
***************************************************************************/
int DAL3IBISgetNoisyMaps(dal_element *mapPtr, 
			 OBTime       obtStart, 
			 OBTime       obtEnd, 
			 OBTime      *blockTime, 
			 DAL3_Byte   *mceId, 
			 DAL3_Word   *periodOn, 
			 DAL3_Byte   *onRange, 
			 DAL3_Byte  (*blockStatus)[IBIS_IBLOCK_LENGTH], 
			 long        *noisylength, 
			 int          status){
  char *first_key = ICA_OBT_FIRST_KEY;
  char *last_key  = ICA_OBT_LAST_KEY;
  char startStr[DAL_MED_STRING];
  char endStr[DAL_MED_STRING];
  char slctStr[SELECT_STRING_LENGTH];
  char ** columns;
  int i, j, k;
  int numSelected = 0;
  long numRows = 0;
  long ksw = 0;
  IBISDS_Type input;
  /* temporary map buffer */
  OBTime *blockTime_tmp = NULL; 
  DAL3_Byte *mceId_tmp = NULL;
  DAL3_Word *periodOn_tmp = NULL;
  DAL3_Byte *onRange_tmp = NULL;
  DAL3_Byte (*blockStatus_tmp)[IBIS_IBLOCK_LENGTH] = NULL; 
  /* Variables to  pass from one list to another. */
  dal_element *subIdxPtr = NULL;
  dal_element *elePtr = NULL;
  /* 
   *   Start of the function.
   */
  if(status != ISDC_OK) return(status);
  if(mapPtr == NULL) return(DAL3GEN_INVALID_PARAMETER);
  columns = malloc(2*sizeof(char*)); 
  columns[0] = first_key;
  columns[1] = last_key;
  *noisylength = 0;
  /*  
   *  Determine the input type. It can be the pixel switches list or an index of it. 
   */
  status = DAL3IBISindexOrMember(mapPtr, ICA_EXTNAME_ISGR_NOISY_MAP, &input, status);
  if(status != ISDC_OK) {
    free(columns);
    return(status);
  }
  
  /*
   *  Get the pixel switch list. 
   */
  switch (input) {
  case IBIS_DS	:
    /* If the input is a single noisy pixel map, it is very easy.    */
    /* NB: The buffer must have been allocated before this function. */ 
    status = DAL3IBISicaReadNoisyMap(mapPtr, blockTime,  mceId, periodOn, onRange, blockStatus,
				     noisylength, status);
    break;
  case IBIS_INDEX	:
    /* If the input is an index, several operation have to be performed. */
    /* select the list which are in the limits */
    status = DAL3GENformatOBT( obtStart, startStr, status);
    status = DAL3GENformatOBT( obtEnd, endStr, status);
    /* (S <= KS && KS <= E) || (S <= KE && KE <= E) ||  ( KS <= S && E <= KE) */
    sprintf(slctStr, "('%s' <= %s && %s <= '%s' )||( '%s' <= %s && %s <= '%s' )||( %s <= '%s' && '%s' <= %s )", 
            startStr,  first_key, first_key, endStr, 
            startStr,  last_key,  last_key,  endStr, 
            first_key, startStr,  endStr,    last_key );
    status = DAL3GENindexFindMember(mapPtr,NULL,slctStr,&numSelected,&subIdxPtr, status);
    if(status != ISDC_OK) {
      free (columns);
      return(status);
    }
    
    if( numSelected  < 1 ){ 
      status = DAL3IBIS_INDEX_EMPTY;
    } else if( numSelected  == 1 ){
      status = DAL3IBISicaReadNoisyMap(subIdxPtr, blockTime,  mceId, periodOn, onRange, blockStatus,
				       noisylength, status);
    } else if( numSelected  > 1 ){
      /* sort the subIndex*/
      status = DALtableSortRows(subIdxPtr, columns, 2, DAL_SORT_DESCENDING, DAL_L2R, status); 
      if(status != ISDC_OK) {
	free(columns);
	return(status);
      }
      
      /* LOOP on all selected lists */
      ksw = 0;
      for(i=1; i<= numSelected && status == ISDC_OK; i++){ 
	/*  Get the list and allocate data buffer */
	status = DAL3GENindexGetMember(subIdxPtr, NULL, i, &elePtr, status);
	status = DALtableGetNumRows(elePtr, &numRows, status);
	status =  DAL3IBISallocateNoisyMaps( &blockTime_tmp,  &mceId_tmp, &periodOn_tmp,
					     &onRange_tmp, &blockStatus_tmp, numRows, status);
	/* Read the list of pixel switches */  
	status = DAL3IBISicaReadNoisyMap(elePtr, blockTime_tmp,  mceId_tmp, periodOn_tmp, onRange_tmp,
					 blockStatus_tmp, &numRows, status);
	/* LOOP on all statement */
	for(j = 0; j < numRows && status == ISDC_OK; j++){
	  blockTime[ksw] = blockTime_tmp[j];
	  mceId[ksw] = mceId_tmp[j];
	  periodOn[ksw] = periodOn_tmp[j];
	  onRange[ksw] = onRange_tmp[j];
	  for(k=0; k < IBIS_IBLOCK_LENGTH; k++)
	    blockStatus[ksw][k] = blockStatus_tmp[j][k];
	  ksw++;
	} /* END LOOP on all switches */
	status =  DAL3IBISfreeNoisyMaps( blockTime_tmp,  mceId_tmp, periodOn_tmp,
					 onRange_tmp, blockStatus_tmp, status);
	blockTime_tmp=NULL;
	mceId_tmp=NULL;
	periodOn_tmp=NULL;
	onRange_tmp=NULL;
	blockStatus_tmp=NULL;
      } /* END LOOP on all selected lists */
      *noisylength = ksw;
      status = DALobjectClose(subIdxPtr, DAL_DELETE, status);
    } /* END if more than one list in the index */ 
    break;
  default: /* SPR 830 */
    break;
  } /* END of switch(input) */
  free(columns);
  return(status);
}

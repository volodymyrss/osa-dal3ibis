/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                            ICA MISCELLANEOUS                              */
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
/*  09.04.2001 V 3.4.1                                                       */
/*  ==================                                                       */
/*  09.04: SPR0371: Fixed a typo in the "DAL3IBISicaGetSize" function        */
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

 Function : DAL3IBISindexOrMember                                          

 Description :
 	Determine the input type. It can be the data structure or 
	the index of it 

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------                              
   dsPtr	  dal_element*	  I   Pointer to the table index or to the 
   				      table itself.
   extnameRef	  char * 	  I   Extname of the expected data structure
   input	  IBISDS_Type*	  O   Input type
   status         int            I/O  Error code.

*****************************************************************************/
int DAL3IBISindexOrMember(dal_element *dsPtr,
			  char        *extnameRef,
			  IBISDS_Type *input,
			  int          status){

  char mbrname[DAL_MED_STRING];
  char extname[DAL_MED_STRING];  
  /* Start of the function. */
  if(status != ISDC_OK) return(status);
  status = DALattributeGetChar(dsPtr, "IDXMEMBR",  mbrname, NULL, NULL, status);
  /* if the keyword IDXMEMBR does not exist that means it is not an index */
  if (status == (DAL_ERROR_CODE_BASE - KEY_NO_EXIST)) {
    status = ISDC_OK;
    status = DALelementGetName(dsPtr, extname, status);
    /* Two extname are valid for the pixel switches list */
    if(strcmp(extname, extnameRef ) == 0){
      *input = IBIS_DS;
    } else {
      status = DAL3IBIS_NOT_VALID_DS;
    }
    /* Two member name are valid for the pixel switches list indices */
  } else if(status == ISDC_OK && 
  	    (strcmp( mbrname, extnameRef ) == 0)){
    *input = IBIS_INDEX;
  } else if(status == ISDC_OK){
    status = DAL3IBIS_NOT_VALID_INDEX;
  }
  return(status);
}
/*****************************************************************************

 Function : DAL3IBISicaGetSize                                           

 Description :
 	Determine the maximum size needed to read tables. 
	The input can be an index of those tables. 

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------                              
   dsPtr	  dal_element*	  I   Pointer to the table index or to the 
   				      table itself.
   input	  IBISDS_Type 	  I   Input type
   startDs_key	  char*		  I   keyword onto the selection will be made
   endDs_key	  char*		  I   keyword onto the selection will be made
   limStart	  OBTime	  I   limit OBT for the selection
   limEnd	  OBTime	  I   limit OBT for the selection
   size		  long *	  O   Number of rows find in the selected 
   				      tables.
   status         int            I/O  Error code.

*****************************************************************************/
int DAL3IBISicaGetSize(dal_element *dsPtr,
  		       IBISDS_Type  input,
		       char 	   *startDs_key,
		       char 	   *endDs_key,
  		       OBTime       limStart, 
		       OBTime       limEnd, 
		       long        *size, 
		       int          status){
  int i;
  int numSelected = 0;
  long numRows = 0;
  char limStartStr[DAL_MED_STRING];
  char limEndStr[DAL_MED_STRING];
  char slctStr[SELECT_STRING_LENGTH];
  dal_element *subIdxPtr = NULL;
  dal_element *elePtr = NULL;
  /* Start of the function. */
  if(status != ISDC_OK){ return(status); }
  *size = 0;	    
  switch (input) {
  case IBIS_DS		:
    status = DALtableGetNumRows(dsPtr, size, status);
    break;
  case IBIS_INDEX	:
    /* select the elements */
    status = DAL3GENformatOBT( limStart, limStartStr, status);
    status = DAL3GENformatOBT( limEnd, limEndStr, status);
    /* (S <= KS && KS <= E) || (S <= KE && KE <= E) ||  ( KS <= S && E <= KE) */
    sprintf(slctStr, "('%s' <= %s && %s <= '%s' )||( '%s' <= %s && %s <= '%s' )||( %s <= '%s' && '%s' <= %s )", 
            limStartStr, startDs_key, startDs_key, limEndStr, 
            limStartStr, endDs_key,   endDs_key,   limEndStr, 
            startDs_key, limStartStr, limEndStr,   endDs_key);
    status = DAL3GENindexFindMember(dsPtr,NULL,slctStr,&numSelected,&subIdxPtr, status);
   if(status != ISDC_OK) return(status);
    if( numSelected  <= 0 ){ 
      status = DAL3IBIS_INDEX_EMPTY;
    } else if  ( numSelected  == 1 ){ 
      status = DALtableGetNumRows(subIdxPtr, &numRows, status);
      *size += numRows;
   } else {
      /* LOOP on the selected data structures. */
      for(i=1; i<= numSelected; i++){ 
	status = DAL3GENindexGetMember(subIdxPtr, NULL, i, &elePtr, status);
	status = DALelementReopen(elePtr, status);
	status = DALtableGetNumRows(elePtr, &numRows, status);
	*size += numRows;
      }
      status = DALobjectClose(subIdxPtr, DAL_DELETE, status);
    }
    break;
  default: /* SPR 830 */
    break;
  }
  return(status); 			      
}		 

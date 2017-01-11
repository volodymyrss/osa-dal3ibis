/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                                ICA LIST                                   */
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
/*  08.04.2001 V 3.4.0                                                       */
/*  ==================                                                       */
/*  08.04: SCR0180: The ICA APIs have been completely revised. This part of  */
/*                  DAL3IBIS is now isolated at the file level to ease the   */
/*                  concurrent development.                                  */
/*                                                                           */
/*  17.02.2000 V 2.2.2                                                       */
/*  ==================                                                       */
/* 1. The ica functions have been modified to handle the new template for    */
/*    the noisy pixels map (ISGR-NOIS-CRW). One data structure contains now  */
/*    N minutes of Telemetry and not one complete set of blocks.             */
/* 2. The different switch lists (different creator and precision) are now   */
/*    stored in Data Structure with different extname but with the same      */
/*    structure (ISGR-NMAP-STA and ISGR-NEVT-STA).                           */
/*    Before It was the same extname with keyword value which make the       */
/*    difference.                                                            */
/* 3. The modification of the Observation Group and of the related DAL3GEN   */
/*    functions have been implemented (reference Platform 1.4.)              */
/*                                                                           */
/*  29.10.1999 V 2.2.1                                                       */
/*                                                                           */
/*  01.09.1999 V 2.2.0                                                       */
/*  ==================                                                       */
/*  1. New DAL3IBIS APIs for ICA are included (written by L.Lerusse)         */
/*                                                                           */
/*****************************************************************************/

# include "dal3ibis.h" 

/*****************************************************************************

 Static Function :  ica_readSwitchList                                          

 Description :  
  Read the ISGRI pixels switch list data structure. The input must be 
  one table of the good type. No more testing done on the input. 

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   dsPtr    dal_element*    I  Pointer to the ISGRI switch list.
   Y_switch   DAL3_Byte *     O  Y position of the pixel.
   Z_switch   DAL3_Byte *   O  Z position of the pixel.
   timeDetect   OBTime *    O  Detection time of the switch.
   timeSwitch   OBTime *    O  Confirmation time of the switch.
   flag_switch    DAL3_Byte *   O  Type of switch.
   *numRows   long*    I/O Number of switch to allocate.
   status         int            I/O Error code.
                                                                           
*****************************************************************************/
static int DAL3IBISicaReadSwitchList(dal_element *dsPtr, 
             DAL3_Byte   *Y_switch, 
             DAL3_Byte   *Z_switch, 
             OBTime      *timeD, 
             OBTime      *timeS, 
             DAL3_Byte   *flag, 
             long        *numRows, 
             int          status) {
  dal_dataType    yType = SWITCH_Y_COLTYPE;
  dal_dataType    zType = SWITCH_Z_COLTYPE;
  dal_dataType    flagType = SWITCH_FLAG_COLTYPE;

  if (status != ISDC_OK) return status;

  /* Get the contents of the columns  */
  status = DALtableGetCol(dsPtr, SWITCH_Y_COLNAME, 0, &yType, numRows, 
        Y_switch, status);
  status = DALtableGetCol(dsPtr, SWITCH_Z_COLNAME, 0, &zType, numRows, 
        Z_switch, status);
  status = DAL3GENtableGetOBT(dsPtr, SWITCH_OBT1_COLNAME, 0, numRows, timeD, 
            status);
  status = DAL3GENtableGetOBT(dsPtr, SWITCH_OBT2_COLNAME, 0, numRows, timeS, 
            status);
  status = DALtableGetCol(dsPtr, SWITCH_FLAG_COLNAME, 0, &flagType, numRows, 
        flag, status);
  return (status);
}

/*****************************************************************************

 Static Function :  ica_readSwitchListMceTime                                           

 Description :  

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   listPtr    dal_element*    I  Pointer to the ISGRI switch list.
   status         int            I/O Error code.
                                                                           
*****************************************************************************/
static int DAL3IBISicaReadSwitchListMceTime( dal_element *listPtr, 
               OBTime      *mceTime,
               int          status){
  if(status != ISDC_OK) return(status);
  
  status = DAL3GENattributeGetOBT(listPtr, SWITCH_MCE_END_TIME_0, &mceTime[0], NULL, status);
  status = DAL3GENattributeGetOBT(listPtr, SWITCH_MCE_END_TIME_1, &mceTime[1], NULL, status);
  status = DAL3GENattributeGetOBT(listPtr, SWITCH_MCE_END_TIME_2, &mceTime[2], NULL, status);
  status = DAL3GENattributeGetOBT(listPtr, SWITCH_MCE_END_TIME_3, &mceTime[3], NULL, status);
  status = DAL3GENattributeGetOBT(listPtr, SWITCH_MCE_END_TIME_4, &mceTime[4], NULL, status);
  status = DAL3GENattributeGetOBT(listPtr, SWITCH_MCE_END_TIME_5, &mceTime[5], NULL, status);
  status = DAL3GENattributeGetOBT(listPtr, SWITCH_MCE_END_TIME_6, &mceTime[6], NULL, status);
  status = DAL3GENattributeGetOBT(listPtr, SWITCH_MCE_END_TIME_7, &mceTime[7], NULL, status);

  return(status);
}


/****************************************************************************

 Function :    DAL3IBISgetSizeSwitchList                                         

 Description :  
  Get the size of the pixel switches list between 
  OBTstart and obtEnd.

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   listPtr    dal_element*    I  Pointer to the ISGRI switch list index or 
               to the list itself.
   obtStart   OBTime    I  Start time of the considered interval
   obtEnd   OBTime    I  End time of the considered interval
   listlength   long*     O  Number of switch to allocate.
   status         int            I/O Error code.
                                                                           
***************************************************************************/
int DAL3IBISgetSizeSwitchList(dal_element *listPtr, 
            OBTime       obtStart, 
            OBTime       obtEnd, 
            long        *listlength, 
            int          status) {  
  char       *first_key = SWITCH_START_TIME;
  char       *last_key = SWITCH_END_TIME;
  IBISDS_Type     input;

  if (status != ISDC_OK) return (status);
  if (listPtr == NULL) return(DAL3GEN_INVALID_PARAMETER);
  *listlength = 0;
  /*  Determine the input type. It can be the pixel switches list or an index of it. */
  status = DAL3IBISindexOrMember(listPtr, ICA_EXTNAME_ISGR_SW_LIST, &input, status);
  /* Get the low threshold data structure. */
  status = DAL3IBISicaGetSize(listPtr, input, first_key, last_key, obtStart, 
            obtEnd, listlength, status);
  return (status);
}

/****************************************************************************

 Function :  DAL3IBISallocateSwitchList                                          

 Description :  
  The function allocate the necessary space for the pixel switches 
  list buffer.

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   Y_switch   DAL3_Byte *    I/O Y position of the pixel.
   Z_switch   DAL3_Byte *  I/O Z position of the pixel.
   timeDetect   OBTime *   I/O Detection time of the switch.
   timeSwitch   OBTime *   I/O Confirmation time of the switch.
   flag_switch    DAL3_Byte *  I/O Yype of switch.
   listlength   long      I  Number of switch to allocate.
   status         int            I/O Error code.
                                                                           
***************************************************************************/

int DAL3IBISallocateSwitchList(DAL3_Byte **Y_switch, 
             DAL3_Byte **Z_switch, 
             OBTime    **timeDetect, 
             OBTime    **timeSwitch, 
             DAL3_Byte **flag_switch, 
             long        listlength, 
             int         status) {  
  
  if (status != ISDC_OK) return (status);
  listlength+=ISGRI_SIZE*ISGRI_SIZE;
  if((*Y_switch    = realloc(*Y_switch   ,listlength *sizeof(DAL3_Byte))) == NULL || 
     (*Z_switch    = realloc(*Z_switch   ,listlength *sizeof(DAL3_Byte))) == NULL || 
     (*flag_switch = realloc(*flag_switch,listlength *sizeof(DAL3_Byte))) == NULL || 
     (*timeDetect  = realloc(*timeDetect ,listlength *sizeof(OBTime)))    == NULL || 
     (*timeSwitch  = realloc(*timeSwitch ,listlength *sizeof(OBTime)))    == NULL) {
    status = DAL_MALLOC_ERROR;
  }
  return (status);
}
/****************************************************************************

 Function :  DAL3IBISfreeSwitchList                                          

 Description :  
  The function deallocates the pixel switches list buffer.

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   Y_switch   DAL3_Byte *    I/O Y position of the pixel.
   Z_switch   DAL3_Byte *  I/O Z position of the pixel.
   timeDetect   OBTime *   I/O Detection time of the switch.
   timeSwitch   OBTime *   I/O Confirmation time of the switch.
   flag_switch    DAL3_Byte *  I/O Yype of switch.
   status         int            I/O Error code.
                                                                           
***************************************************************************/

int DAL3IBISfreeSwitchList(DAL3_Byte *Y_switch, 
         DAL3_Byte *Z_switch, 
         OBTime    *timeDetect, 
         OBTime    *timeSwitch, 
         DAL3_Byte *flag_switch, 
         int        status) { 
  
  if (status != ISDC_OK) return (status);
  free(Y_switch);
  free(Z_switch);
  free(timeDetect);
  free(timeSwitch);
  free(flag_switch);
  return (status);
}

/****************************************************************************

 Function :   DAL3IBISgetSwitchList                                         

 Description :  

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   listPtr    dal_element*    I  Pointer to the ISGRI switch list index or 
               to the list itself.
   obtStart   OBTime    I  Start time of the considered interval
   obtEnd   OBTime    I  End time of the considered interval
   Y_switch   DAL3_Byte *     O  Y position of the pixel.
   Z_switch   DAL3_Byte *   O  Z position of the pixel.
   timeDetect   OBTime *    O  Detection time of the switch.
   timeSwitch   OBTime *    O  Confirmation time of the switch.
   flag_switch    DAL3_Byte *   O  Type of switch.
   listlength   long *   I/O Number of switchs allocated.
   status         int            I/O Error code.
                                                                           
***************************************************************************/
int DAL3IBISgetSwitchList(dal_element *listPtr, 
        OBTime       obtStart, 
        OBTime       obtEnd, 
        DAL3_Byte   *Y_switch, 
        DAL3_Byte   *Z_switch, 
        OBTime      *timeDetect, 
        OBTime      *timeSwitch, 
        DAL3_Byte   *flag_switch, 
        long        *listlength, 
        int          status) { 

  char       *first_key = SWITCH_START_TIME;
  char       *last_key = SWITCH_END_TIME;
  char       startStr[DAL_MED_STRING];
  char       endStr[DAL_MED_STRING];
  char       slctStr[SELECT_STRING_LENGTH];
  char       ** columns;
  int      i, j, iSw;
  unsigned char iy, iz;
  int      numSelected = 0;
  int      switchStatus = 0;
  long       numRows = 0;
  long       ksw = 0;
  IBISDS_Type     input;
  /* temporary list buffer */
  DAL3_Byte      *YSw = NULL;
  DAL3_Byte      *ZSw = NULL;
  OBTime       *tDe = NULL;
  OBTime       *tSw = NULL;
  DAL3_Byte      *fSw = NULL;
  /* Variables to  pass from one list to another. */
  DAL3_Byte      statusPix[ISGRI_SIZE][ISGRI_SIZE];
  OBTime       statusTime[ISGRI_SIZE][ISGRI_SIZE];
  dal_element     *subIdxPtr = NULL;
  dal_element     *elePtr = NULL;
  /* 
   *   Start of the function.
   */
  if (status != ISDC_OK) return (status);
  columns = malloc(2 *sizeof(char *));
  columns[0] = first_key;
  columns[1] = last_key;
  /* SPR 2202, don't zero anymore because it's an input parameter */
  /* *listlength = 0; */
  /*  
   *  Determine the input type. It can be the pixel switches list or an index of it. 
   */
  status = DAL3IBISindexOrMember(listPtr, ICA_EXTNAME_ISGR_SW_LIST, &input, status);
  if (status != ISDC_OK) {
    /* SPR 1927 */
    free (columns);
    return (status);
  }
  if (listPtr == NULL) {
    /* SPR 1927 */
    free(columns);
    return(DAL3GEN_INVALID_PARAMETER);
  }
  /*
   *  Get the pixel switch list. 
   */
  switch (input) {
  case IBIS_DS:
    /* If the input is a single pixel switch list, it is very easy.  */
    /* NB: The buffer must have been allocated before this function. */
    status = DAL3IBISicaReadSwitchList(listPtr, Y_switch, Z_switch, timeDetect, 
               timeSwitch, flag_switch, listlength, 
               status);
    break;
  case IBIS_INDEX:
    /* If the input is an index, several operation have to be performed. */
    /* select the list which are in the limits */
    status = DAL3GENformatOBT(obtStart, startStr, status);
    status = DAL3GENformatOBT(obtEnd, endStr, status);
    /* (S <= KS && KS <= E) || (S <= KE && KE <= E) ||  ( KS <= S && E <= KE) */
    sprintf(slctStr, "('%s' <= %s && %s <= '%s' )||( '%s' <= %s && %s <= '%s' )||( %s <= '%s' && '%s' <= %s )", 
            startStr,  first_key, first_key, endStr, 
            startStr,  last_key,  last_key,  endStr, 
            first_key, startStr,  endStr,    last_key );
    status = DAL3GENindexFindMember(listPtr,NULL,slctStr,&numSelected,&subIdxPtr, status);
    if (status != ISDC_OK) 
      {
  /* spr 1942 */
  free(columns);
  return (status);
      }
    if (numSelected < 1) {
      /* spr 1942 */
      free(columns);
      status = DAL3IBIS_INDEX_EMPTY;
      return (status);
    }
    else if (numSelected == 1) {
      status = DAL3IBISicaReadSwitchList(subIdxPtr, Y_switch, Z_switch, 
           timeDetect, timeSwitch, flag_switch, 
           listlength, status);      
    }
    else if (numSelected > 1) {
      /* sort the subIndex */
      status = DALtableSortRows(subIdxPtr, columns, 2, DAL_SORT_ASCENDING, DAL_L2R, status);
      if (status != ISDC_OK) {
  /* spr 1942 */
  free(columns);
  return (status);
      }
      ksw = 0;
      switchStatus = 0;
      for (iy = 0; iy < ISGRI_SIZE; iy++)
  for (iz = 0; iz < ISGRI_SIZE; iz++)
    statusPix[iy][iz] = DAL3IBIS_IPIX_UNKNOWN;

      /* LOOP on all selected lists */
      for (i = 1; i <= numSelected; i++) {
  /*  Get the list and allocate data buffer */
  status = DAL3GENindexGetMember(subIdxPtr, NULL, i, &elePtr, status);
  status = DALtableGetNumRows(elePtr, &numRows, status);
  status = DAL3IBISallocateSwitchList(&YSw, &ZSw, &tDe, &tSw, &fSw, numRows, status);
  /* Read the list of pixel switches */
  status = DAL3IBISicaReadSwitchList(elePtr, YSw, ZSw, tDe, tSw, fSw, &
             numRows, status);  
  iSw = 0;
  if(statusPix[0][0] ==  DAL3IBIS_IPIX_UNKNOWN){ 
    while( iSw <  numRows && 
          (fSw[iSw] == DAL3IBIS_IPIX_DUBIOUS || 
           fSw[iSw] == DAL3IBIS_IPIX_OFF)){ /* SPR 2755 */
      statusPix[YSw[iSw]][ZSw[iSw]] = fSw[iSw];
      statusTime[YSw[iSw]][ZSw[iSw]] = tSw[iSw];
      Y_switch[ksw] = YSw[iSw];
      Z_switch[ksw] = ZSw[iSw];
      timeDetect[ksw] = statusTime[YSw[iSw]][ZSw[iSw]];
      timeSwitch[ksw] = tSw[iSw];
      flag_switch[ksw] = fSw[iSw];
      ksw++;
      if (ksw > *listlength + ISGRI_SIZE*ISGRI_SIZE) {
        /* SPR 2202, opps, we ran out of allocated switches */
        return DAL3IBIS_NOT_ENOUGH_SW;
      }
      iSw++;
    }
    for (iy = 0; iy < ISGRI_SIZE; iy++){
      for (iz = 0; iz < ISGRI_SIZE; iz++){
        if(statusPix[iy][iz] == DAL3IBIS_IPIX_UNKNOWN){
    statusPix[iy][iz] = DAL3IBIS_IPIX_ON;
    statusTime[iy][iz] = tSw[0];    
        }
      }
    }
  }
  
  
    
                    
  /* LOOP on all statement */
  for (j = iSw; j < numRows; j++) {
    if(YSw[j] < ISGRI_SIZE &&
       ZSw[j] < ISGRI_SIZE ){
      switch (fSw[j]) {
      case DAL3IBIS_IPIX_DUBIOUS  :
      case DAL3IBIS_IPIX_OFF  :
        if( statusPix[YSw[j]][ZSw[j]] != DAL3IBIS_IPIX_DUBIOUS &&
      statusPix[YSw[j]][ZSw[j]] != DAL3IBIS_IPIX_OFF){
    Y_switch[ksw] = YSw[j];
    Z_switch[ksw] = ZSw[j];
    timeDetect[ksw] = statusTime[YSw[j]][ZSw[j]];
    timeSwitch[ksw] = tSw[j];
    flag_switch[ksw] = DAL3IBIS_SWITCH_OFF_ON;
    ksw++;
    if (ksw > *listlength + ISGRI_SIZE*ISGRI_SIZE) {
      /* SPR 2202, opps, we ran out of allocated switches */
      return DAL3IBIS_NOT_ENOUGH_SW;
    }
    statusPix[YSw[j]][ZSw[j]] =  DAL3IBIS_IPIX_ON;
    statusTime[YSw[j]][ZSw[j]] = tSw[j];   
        } else {
    statusTime[YSw[j]][ZSw[j]] = tSw[j];   
        }
      
        break;
      case DAL3IBIS_SWITCH_ON_OFF :
        if( statusPix[YSw[j]][ZSw[j]] != DAL3IBIS_IPIX_ON){
    Y_switch[ksw] = YSw[j];
    Z_switch[ksw] = ZSw[j];
    timeDetect[ksw] = statusTime[YSw[j]][ZSw[j]];
    timeSwitch[ksw] = tDe[j];
    flag_switch[ksw] = DAL3IBIS_SWITCH_OFF_ON;
    ksw++;
    if (ksw > *listlength + ISGRI_SIZE*ISGRI_SIZE) {
      /* SPR 2202, opps, we ran out of allocated switches */
      return DAL3IBIS_NOT_ENOUGH_SW;
    }
    statusPix[YSw[j]][ZSw[j]] =  DAL3IBIS_IPIX_ON;
    statusTime[YSw[iSw]][ZSw[iSw]] = tSw[j];    
        }
        Y_switch[ksw] = YSw[j];
        Z_switch[ksw] = ZSw[j];
        timeDetect[ksw] = tDe[j];
        timeSwitch[ksw] = tSw[j];
        flag_switch[ksw] = fSw[j];
        ksw++;
        if (ksw > *listlength + ISGRI_SIZE*ISGRI_SIZE) {
    /* SPR 2202, opps, we ran out of allocated switches */
    return DAL3IBIS_NOT_ENOUGH_SW;
        }
        statusPix[YSw[j]][ZSw[j]] =  DAL3IBIS_IPIX_DUBIOUS;
        statusTime[YSw[iSw]][ZSw[iSw]] = tSw[j];    
        break;
      case DAL3IBIS_SWITCH_OFF_ON :
        if( statusPix[YSw[j]][ZSw[j]] != DAL3IBIS_IPIX_DUBIOUS &&
      statusPix[YSw[j]][ZSw[j]] != DAL3IBIS_IPIX_OFF){
    Y_switch[ksw] = YSw[j];
    Z_switch[ksw] = ZSw[j];
    timeDetect[ksw] = statusTime[YSw[j]][ZSw[j]];
    timeSwitch[ksw] = tDe[j];
    flag_switch[ksw] = DAL3IBIS_SWITCH_OFF_ON;
    ksw++;
    if (ksw > *listlength + ISGRI_SIZE*ISGRI_SIZE) {
      /* SPR 2202, opps, we ran out of allocated switches */
      return DAL3IBIS_NOT_ENOUGH_SW;
    }
    statusPix[YSw[j]][ZSw[j]] =  DAL3IBIS_IPIX_ON;
    statusTime[YSw[iSw]][ZSw[iSw]] = tSw[j];    
        }
        Y_switch[ksw] = YSw[j];
        Z_switch[ksw] = ZSw[j];
        timeDetect[ksw] = tDe[j];
        timeSwitch[ksw] = tSw[j];
        flag_switch[ksw] = fSw[j];
        ksw++;
        if (ksw > *listlength + ISGRI_SIZE*ISGRI_SIZE) {
    /* SPR 2202, opps, we ran out of allocated switches */
    return DAL3IBIS_NOT_ENOUGH_SW;
        }
        statusPix[YSw[j]][ZSw[j]] =  DAL3IBIS_IPIX_ON;
        statusTime[YSw[iSw]][ZSw[iSw]] = tSw[j];    
        break;
      default:
        break;
      }
    }
  }
  status = DAL3IBISfreeSwitchList(YSw, ZSw, tDe, tSw,fSw,status);
  YSw = NULL; 
  ZSw= NULL;
  tDe= NULL;
  tSw= NULL;
  fSw= NULL;
  /* END LOOP on all switches */
      }
      /* END LOOP on all selected lists */
      /* Look at the status statement stored in the array */
      for (iy = 0; iy < ISGRI_SIZE; iy++) {
  for (iz = 0; iz < ISGRI_SIZE; iz++) {
    if (statusPix[iy][iz] != DAL3IBIS_IPIX_ON) {
      Y_switch[ksw] = iy;
      Z_switch[ksw] = iz;
      timeDetect[ksw] = statusTime[iy][iz];
      timeSwitch[ksw] = statusTime[iy][iz];
      flag_switch[ksw] = statusPix[iy][iz];
      ksw++;
      if (ksw > *listlength + ISGRI_SIZE*ISGRI_SIZE) {
        /* SPR 2202, opps, we ran out of allocated switches */
        return DAL3IBIS_NOT_ENOUGH_SW;
      }
    }
  }
      }
      *listlength = ksw;
      status = DALobjectClose(subIdxPtr, DAL_DELETE, status);
    }
    /* END if more than one list in the index */
    break;
  default: /* SPR 830 */
    status = DAL3IBIS_INCOMPATIBLE_PARAMETER;
    break;
    
  }
  /* END of switch(input) */
  /* spr 1942 */
  free(columns);
  return (status);
}

/****************************************************************************

 Function :   DAL3IBISgetSwitchListMceTime                                         

 Description :  

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   listPtr    dal_element*    I  Pointer to the ISGRI switch list index or 
               to the list itself.
   obtStart   OBTime    I  Start time of the considered interval
   obtEnd   OBTime    I  End time of the considered interval
   status         int            I/O Error code.
                                                                           
***************************************************************************/
int DAL3IBISgetSwitchListMceTime( dal_element *listPtr, 
          OBTime       obtStart, 
          OBTime       obtEnd, 
          OBTime      *mceTime, 
          int          status){
  char *first_key = SWITCH_START_TIME;
  char *last_key = SWITCH_END_TIME;
  char ** columns;
  int numSelected;

  IBISDS_Type     input;
 
  char       startStr[DAL_MED_STRING];
  char       endStr[DAL_MED_STRING];
  char       slctStr[SELECT_STRING_LENGTH];

  dal_element *subIdxPtr = NULL;
  dal_element *elePtr = NULL;

  dal_accessType aType;
  dal_baseType baseType;


  /* 
   *   Start of the function.
   */
  if (status != ISDC_OK) return(status);
  if (listPtr == NULL) return(DAL3GEN_INVALID_PARAMETER);
 
  columns = malloc(2 *sizeof(char *));
  columns[0] = first_key;
  columns[1] = last_key;
  /*  
   *  Determine the input type. It can be the pixel switches list or an index of it. 
   */
  status = DAL3IBISindexOrMember(listPtr, ICA_EXTNAME_ISGR_SW_LIST, &input, status);
  if (status != ISDC_OK) {
    /* spr 1942 */
    free(columns);
    return (status);
  }
  
  /*
   *  Get the pixel switch list. 
   */
  switch (input) {
  case IBIS_DS:
    /* If the input is a single pixel switch list, it is very easy.  */
    /* NB: The buffer must have been allocated before this function. */
    status = DAL3IBISicaReadSwitchListMceTime(listPtr, mceTime, status);
    break;
  case IBIS_INDEX:
    /* If the input is an index, several operation have to be performed. */
    /* select the list which are in the limits */
    status = DAL3GENformatOBT(obtStart, startStr, status);
    status = DAL3GENformatOBT(obtEnd, endStr, status);
    /* (S <= KS && KS <= E) || (S <= KE && KE <= E) ||  ( KS <= S && E <= KE) */
    sprintf(slctStr, "('%s' <= %s && %s <= '%s' )||( '%s' <= %s && %s <= '%s' )||( %s <= '%s' && '%s' <= %s )", 
            startStr,  first_key, first_key, endStr, 
            startStr,  last_key,  last_key,  endStr, 
            first_key, startStr,  endStr,    last_key );
    status = DALgroupSelectElements(listPtr, slctStr, DAL_MEM, NULL, &subIdxPtr, status);
    status = DALelementGetNumChildren(subIdxPtr, &numSelected, status);
    if (status != ISDC_OK) { 
      /* spr 1942 */
      free(columns);
      return (status);
    }    
    if (numSelected < 1) {
      status = DAL3IBIS_INDEX_EMPTY;
    }
    else if (numSelected == 1) {
      status = DALelementGetChild(subIdxPtr, 1, &elePtr, &aType, &baseType, status);
      status = DAL3IBISicaReadSwitchListMceTime(elePtr, mceTime, status);
    }
    else if (numSelected > 1) {
      /* sort the subIndex */
      status = DALtableSortRows(subIdxPtr, columns, 2, DAL_SORT_DESCENDING, DAL_L2R, status);
      status = DALelementSortChildren(subIdxPtr, NULL, DAL_SORT_DESCENDING, status);
      status = DALelementGetChild(subIdxPtr, 1, &elePtr, &aType, &baseType, status);
      status = DAL3IBISicaReadSwitchListMceTime(elePtr, mceTime, status);
    }
    break; /* SPR 2974, don't forget to break */
  default: /* SPR 830 */
    status = DAL3IBIS_INCOMPATIBLE_PARAMETER;
    break;  }        
  /* spr 1942 */
  free(columns);
  return (status);
}

/****************************************************************************

 Function :   DAL3IBISgetSwitchList                                         

 Description :  

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   listPtr    dal_element*    I  Pointer to the ISGRI switch list index or 
               to the list itself.
   obtStart   OBTime    I  Start time of the considered interval
   obtEnd   OBTime    I  End time of the considered interval
   status         int            I/O Error code.
                                                                           
***************************************************************************/
int DAL3IBISputSwitchList( dal_element *listPtr,
               DAL3_Byte   *Y_switch,
         DAL3_Byte   *Z_switch,
         OBTime      *timeDetect,
                 OBTime      *timeSwitch,
                 DAL3_Byte   *flag_switch,
                 long         numSwitch,
         OBTime      *mceTime,
         char        *origin,
               int          status){
  long numRows;
  char extname[DAL_MED_STRING];
  OBTime obtStart, obtEnd;
  long i;

  if(status != ISDC_OK) return(status);
  if (listPtr == NULL) return(DAL3GEN_INVALID_PARAMETER);
  if(numSwitch == 0) return(status);
  /* verify that the list is correct. */
  status = DALelementGetName(listPtr, extname, status);

  if(strcmp(extname, ICA_EXTNAME_ISGR_SW_LIST) != 0){
    status =  DAL3IBIS_NOT_VALID_DS;
    return(status);
  }   
  /* Verify that the table is empty. */
  status = DALtableGetNumRows( listPtr, &numRows, status);
  if(status != ISDC_OK) { return(status); }
  if(numRows != 0 && numRows != numSwitch){
    status = DAL3IBIS_SW_LIST_NOT_EMPTY;
    return(status);
  } 
  if(numRows != numSwitch){
    status = DALtableAddRows(listPtr, -1, numSwitch, status);
    if(status != ISDC_OK) { return(status); }
  }
  /* Columns */
  status = DALtablePutCol(listPtr, SWITCH_Y_COLNAME, 0, SWITCH_Y_COLTYPE, numSwitch, Y_switch, status);
  status = DALtablePutCol(listPtr, SWITCH_Z_COLNAME, 0, SWITCH_Z_COLTYPE, numSwitch, Z_switch, status);
  status = DAL3GENtablePutOBT(listPtr, SWITCH_OBT1_COLNAME, 0,  numSwitch, timeDetect, status);
  status = DAL3GENtablePutOBT(listPtr, SWITCH_OBT2_COLNAME, 0,  numSwitch, timeSwitch, status);
  status = DALtablePutCol(listPtr, SWITCH_FLAG_COLNAME, 0, SWITCH_FLAG_COLTYPE, numSwitch, flag_switch, status);
  /* Keywords */
  obtStart =  timeDetect[0];
  obtEnd = timeSwitch[0];
  for( i= 1; i < numSwitch; i++){
    if(timeDetect[i] < obtStart) obtStart = timeDetect[i];
    if(timeSwitch[i] > obtEnd  ) obtEnd   = timeSwitch[i];
  }
  status = DAL3GENattributePutOBT(listPtr, SWITCH_START_TIME, obtStart, NULL, status);
  status = DAL3GENattributePutOBT(listPtr, SWITCH_END_TIME, obtEnd, NULL, status);
  status = DAL3GENattributePutOBT(listPtr, SWITCH_MCE_END_TIME_0, mceTime[0], NULL, status);
  status = DAL3GENattributePutOBT(listPtr, SWITCH_MCE_END_TIME_1, mceTime[1], NULL, status);
  status = DAL3GENattributePutOBT(listPtr, SWITCH_MCE_END_TIME_2, mceTime[2], NULL, status);
  status = DAL3GENattributePutOBT(listPtr, SWITCH_MCE_END_TIME_3, mceTime[3], NULL, status);
  status = DAL3GENattributePutOBT(listPtr, SWITCH_MCE_END_TIME_4, mceTime[4], NULL, status);
  status = DAL3GENattributePutOBT(listPtr, SWITCH_MCE_END_TIME_5, mceTime[5], NULL, status);
  status = DAL3GENattributePutOBT(listPtr, SWITCH_MCE_END_TIME_6, mceTime[6], NULL, status);
  status = DAL3GENattributePutOBT(listPtr, SWITCH_MCE_END_TIME_7, mceTime[7], NULL, status);
  status = DALattributePutChar(listPtr, SWITCH_ORIGIN_KEYWORD, origin, NULL, NULL, status); 
  return(status);
}

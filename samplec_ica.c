/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                          C SAMPLE_ICA PROGRAM                             */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse                               */
/*  Date:    21 May 2001                                                     */
/*  Version: 3.5.0                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  21.05.2001 V 3.5.0                                                       */
/*  ==================                                                       */
/*  16.05: SPR0455: Fixed test data, which were not compatible anymore       */
/*                  Added "TESTING" script                                   */
/*                                                                           */
/*  08.04.2001 V 3.4.0                                                       */
/*  ==================                                                       */
/*  08.04: SCR0180: The ICA APIs have been completely revised. This part of  */
/*                  DAL3IBIS is now isolated at the file level to ease the   */
/*                  concurrent development.                                  */
/*                                                                           */
/*  01.09.1999 V 2.2.0                                                       */
/*  ==================                                                       */
/*  1. New DAL3IBIS APIs for ICA are included (written by L.Lerusse)         */
/*  3. New "samplec_ica.c" and samplef90_ica.f90" executables and test data  */
/*                                                                           */
/*  18.06.1999 V 2.0.1                                                       */
/*  27.05.1999 V 2.0.0                                                       */
/*                                                                           */
/*  10.03.1999 V 1.0.0                                                       */
/*                                                                           */
/*****************************************************************************/

#include "dal3ibis.h"

int main( int argc, char *argv[]){

  const char *idxCtxtDOL    = "./test_data/index/isgri_context_index.fits[1]";
  const char *idxNoisMapDOL = "./test_data/index/isgri_prp_noise_index.fits[1]";
  const char *idxSwListDOL  = "./test_data/index/isgri_pxlswtch_index.fits[1]"; 

  const char *ctxtDOL       = "./test_data/ctxt/isgri_context_20030805150711.fits[1]";
  const char *noisMapDOL    = "./test_data/nois/isgri_prp_noise_20030810213431.fits[1]";
  const char *swListDOL     = "./test_data/list/isgri_pxlswtch_20030810213431.fits[1]";
  
  int status = ISDC_OK;
    
   /*  New Map */        
  OBTime  *block_time = NULL;
  DAL3_Byte *mce_id = NULL;
  DAL3_Word *period_on = NULL;
  DAL3_Byte *on_range = NULL;
  DAL3_Byte (*block_status)[IBIS_IBLOCK_LENGTH] = NULL;
  long numNoisMap = 0;

  /* Previous List */
  DAL3_Byte *YSw = NULL;
  DAL3_Byte *ZSw = NULL;
  OBTime  *timeDe = NULL;
  OBTime  *timeSw = NULL;
  DAL3_Byte *flagSw = NULL;
  long nSw = 0;
  OBTime mceTime[IBIS_NUM_BLOCK];

  /* previous Map */
  OBTime  *blockTime = NULL;
  DAL3_Byte *mceId = NULL;
  DAL3_Word *periodOn = NULL;
  DAL3_Byte *onRange = NULL;
  DAL3_Byte (*blockStatus)[IBIS_IBLOCK_LENGTH] = NULL;
  long numMap = 0;
  
  /* Context */
  DAL3_Byte lowThres[ISGRI_SIZE][ISGRI_SIZE];
  OBTime thresTime;
  OBTime ctxtTime[IBIS_NUM_BLOCK];
  DAL3_Byte ctxtMceId[IBIS_NUM_BLOCK];
  dal_element *slctCtxtPtr;

  int y, z;
  int i, j, l, m;
  double   pixPercOffTime[ISGRI_SIZE][ISGRI_SIZE];
  long     numChg[IBIS_NUM_BLOCK+1];
  OBTime (*effTime)[IBIS_NUM_BLOCK+1];
  int    (*numPixOff)[IBIS_NUM_BLOCK+1];
  
  OBTime first = 26986004086784LL;
  OBTime last =  26988151570432LL;

  dal_element *idxCtxtPtr = NULL;
  dal_element *idxNoisMapPtr = NULL;
  dal_element *idxSwListPtr = NULL;
  dal_element *ctxtPtr = NULL;
  dal_element *noisMapPtr = NULL;
  dal_element *swListPtr = NULL;
  char errStr[1024];

  char *flagStr[5] = { "ON", "OFF", "DUBIOUS", "ON->OFF", "OFF->ON"};

  /*  
   *
   */

  status = DALobjectOpen( ctxtDOL,  &ctxtPtr,  status);
  printf(" %s", ctxtDOL);
  if(status == ISDC_OK) printf(" open!\n");
  else printf(" NOT open! status = %d \n", status);
  status = DALobjectOpen( noisMapDOL, &noisMapPtr, status);
  printf(" %s", noisMapDOL);
  if(status == ISDC_OK) printf(" open!\n");
  else printf(" NOT open! status = %d\n", status);
  status = DALobjectOpen( swListDOL,  &swListPtr, status);
  printf(" %s", swListDOL);
  if(status == ISDC_OK) printf(" open!\n");
  else printf(" NOT open! status = %d\n", status);

  printf("Status : %d\n\n",status);

  status = DALobjectOpen( idxCtxtDOL, &idxCtxtPtr, status);
  printf(" %s", idxCtxtDOL);
  if(status == ISDC_OK) printf(" open!\n");
  else printf(" NOT open! status = %d\n", status);
  status = DALobjectOpen( idxNoisMapDOL, &idxNoisMapPtr, status);
  printf(" %s", idxNoisMapDOL);
  if(status == ISDC_OK) printf(" open!\n");
  else printf(" NOT open! status = %d\n", status);
  status = DALobjectOpen( idxSwListDOL,  &idxSwListPtr, status);
  printf(" %s", idxSwListDOL);
  if(status == ISDC_OK) printf(" open!\n");
  else printf(" NOT open! status = %d\n", status);
  
  printf("Status : %d\n\n",status);

  if(status != ISDC_OK) {
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(status); 
  }

  /* 
   *  CONTEXT - Read 
   */
  printf("\n ISGRI CONTEXT - Read \n");
  thresTime = 26775921865184LL;
  status = DAL3IBISselectCtxt(idxCtxtPtr, &thresTime, ISGRI_CTXT, &slctCtxtPtr, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISselectCtxt - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
  }
  if(status != ISDC_OK){
    printf(" Use the ctxt data structure in place of the selected one \n\n");
    slctCtxtPtr = ctxtPtr;
  }
  printf("Status : %d\n\n",status);

  status = DAL3IBISctxtGetImaPar(slctCtxtPtr, &thresTime,ISGRI_PIX_LTHR, lowThres, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISctxtGetImaPar - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }
  printf("Status : %d\n\n",status);
  status = DAL3IBISctxtGetTblPar(slctCtxtPtr, &thresTime, ISGRI_MODP, "OB_TIME", DAL_BYTE, ctxtTime, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISctxtGetTblPar(OB_TIME) - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }
  printf("Status : %d\n\n",status);
  status = DAL3IBISctxtGetTblPar(slctCtxtPtr, &thresTime, ISGRI_MODP, "MCE_ID", DAL_BYTE, ctxtMceId, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISctxtGetTblPar(MCE_ID) - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }
  printf(" Part of the Low threshold array :\n");
  for(y=6; y >= 0; y --){
    if(y==6){
      printf("\n Z :\t");
    } else {
      printf("\n Y %d:\t", y );
    }
    for( z = 0; z < 10; z ++){
      if(y==6){
  printf("-- %d --", z);
      } else{  
  if(lowThres[y][z] > 9 ){
    printf("  %d   ", lowThres[y][z]);
  } else {
    printf("   %d   ", lowThres[y][z]);
  }
      }
    }
    if(y==6)  printf("\n \t");
  }
  printf("\n");
  printf("Status : %d\n\n",status);

  /* 
   *  ISGRI NOISY PIXELS MAPS - Read 
   */
  printf(" \n ISGRI NOISY PIXELS MAPS - Read one noisy map \n");
  status = DAL3IBISgetSizeNoisyMaps( noisMapPtr, 0, 0, &numNoisMap, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISgetSizeNoisyMaps - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }
  printf("Status : %d\n\n",status);
  status = DAL3IBISallocateNoisyMaps( &block_time, &mce_id, &period_on, &on_range, &block_status, numNoisMap, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISallocateNoisyMaps - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }
  status = DAL3IBISgetNoisyMaps( noisMapPtr, 0, 0, block_time, mce_id, period_on, on_range, block_status, &numNoisMap, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISgetNoisyMaps - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }
  printf("   Number of noisy Maps : %ld\n", numNoisMap);
  printf("   Time start %lld; time end %lld\n", block_time[0], block_time[numNoisMap-1]);
  printf("   The data are difficult to display.\n");
  printf("      Block  0  -> mce %d\n", mce_id[0]); 
  printf("      Block %ld  -> mce %d\n", numNoisMap/3, mce_id[numNoisMap/3]); 
  printf("      Block %ld  -> mce %d\n", 2*numNoisMap/3, mce_id[2*numNoisMap/3]); 
  printf("      Block %ld  -> mce %d\n", numNoisMap-1, mce_id[numNoisMap-1]); 
  printf("\n");
  status = DAL3IBISfreeNoisyMaps( block_time, mce_id, period_on, on_range, block_status, status);
  printf("Status : %d\n\n",status);

  printf("\n ISGRI NOISY PIXELS MAPS - Read several noisy maps \n");

  status = DAL3IBISgetSizeNoisyMaps( idxNoisMapPtr, first, last, &numMap, status);
  if(status != ISDC_OK){    
    printf("DAL3IBISgetSizeNoisyMaps - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }
  printf("Status : %d\n\n",status);
  if(numMap >= 0 && status == ISDC_OK){
    status = DAL3IBISallocateNoisyMaps( &blockTime, &mceId, &periodOn, &onRange, &blockStatus, numMap, status);
    if(status != ISDC_OK){ 
      printf("DAL3IBISallocateNoisyMaps - %d \n", status);
      status = ISDC_OK;
      while(status == ISDC_OK){
  status = DALerrorReadStack(errStr, status);
  printf(" - %s \n", errStr);
      } 
      printf(" \n ");
      return(-1); 
    }
    status = DAL3IBISgetNoisyMaps( idxNoisMapPtr, first, last, blockTime, mceId, periodOn, onRange, blockStatus, &numMap, status);
    if(status != ISDC_OK){ 
      printf("DAL3IBISgetNoisyMaps - %d \n", status);
      status = ISDC_OK;
      while(status == ISDC_OK){
  status = DALerrorReadStack(errStr, status);
  printf(" - %s \n", errStr);
      } 
      printf(" \n ");
      return(-1); 
    }
  } else if(status == ISDC_OK){
    printf(" No ISGRI noisy pixel map! %d \n", status);
  }
  printf("   Number of noisy Maps : %ld \n", numMap);
  printf("   Time start %lld; time end %lld \n", blockTime[0], blockTime[numMap-1]);
  printf("   The data are difficult to display.\n");
  printf("      Block   0  -> mce %d\n", mceId[0]); 
  printf("      Block  %ld  -> mce %d\n", numMap/3, mceId[numMap/3]); 
  printf("      Block %ld  -> mce %d\n", 2*numMap/3, mceId[2*numMap/3]); 
  printf("      Block %ld  -> mce %d\n", numMap-1, mceId[numMap-1]); 
  status = DAL3IBISfreeNoisyMaps( blockTime, mceId, periodOn, onRange, blockStatus, status);
  printf("Status : %d\n\n",status);
  
  /* 
   *  ISGRI PIXEL SWITCHES LIST - Read 
   */
  printf("\n ISGRI PIXEL SWITCHES LIST - Read \n");
  status = DAL3IBISgetSizeSwitchList( idxSwListPtr, first, last, &nSw, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISgetSizeSwitchList - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }
  printf("Status : %d\n\n",status);

  status = DAL3IBISallocateSwitchList( &YSw, &ZSw, &timeDe, &timeSw, &flagSw, nSw, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISallocateSwitchList - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }
  status = DAL3IBISgetSwitchList( idxSwListPtr , first, last, YSw, ZSw,
          timeDe, timeSw, flagSw, &nSw, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISgetSwitchList - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
  } 
    printf(" \n ");
    return(-1); 
  }
  status = DAL3IBISgetSwitchListMceTime( idxSwListPtr, first, last, (OBTime*)mceTime, status);
  if(status != ISDC_OK){ 
    printf("DAL3IBISgetSwitchListMceTime - %d \n", status);
    status = ISDC_OK;
    while(status == ISDC_OK){
      status = DALerrorReadStack(errStr, status);
      printf(" - %s \n", errStr);
    } 
    printf(" \n ");
    return(-1); 
  }

  printf("\n   Number of switches : %ld\n", nSw);
  printf("   Time start %lld; time end %lld \n", timeDe[0], timeSw[nSw-1]);
  printf("   Five typical switches :\n");
  printf("      i=00;  Y: %d; Z: %d; flag:%d %s \n", YSw[0],  ZSw[0],  flagSw[0],  flagStr[flagSw[0]]);
  printf("      i=10;  Y: %d; Z: %d; flag:%d %s \n", YSw[10], ZSw[10], flagSw[10], flagStr[flagSw[10]]);
  printf("      i=20;  Y: %d; Z: %d; flag:%d %s \n", YSw[20], ZSw[20], flagSw[20], flagStr[flagSw[20]]);
  printf("      i=30;  Y: %d; Z: %d; flag:%d %s \n", YSw[30], ZSw[30], flagSw[30], flagStr[flagSw[30]]);
  printf("      i=40;  Y: %d; Z: %d; flag:%d %s \n", YSw[40], ZSw[40], flagSw[40], flagStr[flagSw[40]]);
  printf("\n");
  status = DAL3IBISfreeSwitchList( YSw, ZSw, timeDe, timeSw, flagSw, status);
  printf("Status : %d\n\n",status);


  /*
   *  DAL3IBISicaIsgriNoisEff 
   */
  printf(" ISGRI PIXEL EFFICIENCY - calculation\n");
  status = DAL3IBISgetSizeSwitchList( idxSwListPtr, first, last, &nSw, status);
  printf("    Test DAL3IBISicaIsgriNoisEff with the limits %lld %lld\n", first, last);
  printf("    Number of changes : %ld\n", nSw);
  numPixOff = NULL;
  effTime = NULL;
  numPixOff = malloc((IBIS_NUM_BLOCK+1) * 2* nSw * sizeof(int));
  effTime = malloc((IBIS_NUM_BLOCK+1) * 2* nSw * sizeof(OBTime));
  
  status =  DAL3IBISicaIsgriNoisEff(idxSwListPtr, 1, &first, &last, 
            pixPercOffTime, numChg, effTime, numPixOff, status);

  if(status != ISDC_OK) { 
    printf( "Can not run DAL3IBISicaIsgriNoisEff ! %d \n", status);
    exit(status); 
  }
  printf("\n");
  printf("    The number of level in the timeline for the modules are :\n");
  printf("\t");
  for( i=0; i<= 8 ; i++){
    printf("%ld\t",numChg[i]);
  }
  printf("\n\n");
  for( j=0; j < numChg[8] && j < numChg[0]*1.4; j+=5){
    printf("%d)\t", j);
    for( i=0; i<= 8 ; i++){
      if(j < numChg[i]){  
  if(numPixOff[j][i] > 2048){
    printf("out: %d\t",numPixOff[j][i]);
  }else{
    printf("%d\t",numPixOff[j][i]);
  }
      } else {
        printf("  \t");
      }
    }
    printf("\n");
  } 
  printf("    The negative values are due to bad test data.\n\n");
  
  printf("    The %% OFF time of the pixels: \n");
  l = 5;
  m = 78;
  printf("       pixel[%d][%d]: %f%% OFF \n", m, l, pixPercOffTime[m][l]);
  l = 7;
  m = 3;
  printf("       pixel[%d][%d]: %f%% OFF \n", m, l, pixPercOffTime[m][l]);
  l = 18;
  m = 42;
  printf("       pixel[%d][%d]: %f%% OFF \n", m, l, pixPercOffTime[m][l]);
  l = 56;
  m = 124;
  printf("       pixel[%d][%d]: %f%% OFF \n", m, l, pixPercOffTime[m][l]);
  l = 83;
  m = 42;
  printf("       pixel[%d][%d]: %f%% OFF \n", m, l, pixPercOffTime[m][l]);
  l = 120;
  m = 124;
  printf("       pixel[%d][%d]: %f%% OFF \n", m, l, pixPercOffTime[m][l]);
  printf("\n");
  printf("Status : %d\n\n",status);

  /*
   *   CLOSE AND EXIT. 
   */
  printf("\n That's all, close the data structures\n\n");
  /* Something is broken below, it's not clear what.  For now 
     don't close the files */
  /* status = DALobjectClose( idxCtxtPtr,  DAL_SAVE, status);
     status = DALobjectClose( idxNoisMapPtr, DAL_SAVE, status);
     status = DALobjectClose( idxSwListPtr, DAL_SAVE, status);
     status = DALobjectClose( ctxtPtr,  DAL_SAVE, status);
     status = DALobjectClose( noisMapPtr,  DAL_SAVE, status);
     status = DALobjectClose( swListPtr,  DAL_SAVE, status); 
  */
  printf("Status : %d\n\n",status);
  exit(status); 
}

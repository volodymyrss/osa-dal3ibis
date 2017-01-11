/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                                ICA EFF                                    */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse                            */
/*  Date:    21 Octobre 2003                                                 */
/*  Version: 4.3.4                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  17.10.2003 V 4.3.4                                                       */
/*  ==================                                                       */
/*  17.10: SPR 3056, 3246 and 3247 Rewrote the function to get correct       */
/*                   behaviour.                                              */
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
/*                                                                           */
/* 2. The different switch lists (different creator and precision) are now   */
/*    stored in Data Structure with different extname but with the same      */
/*    structure (ISGR-NMAP-STA and ISGR-NEVT-STA).                           */
/*    Before It was the same extname with keyword value which make the       */
/*    difference.                                                            */
/*                                                                           */
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

#include "dal3ibis.h"

/*****************************************************************************

 Function : DAL3IBISicaIsgriNoisEff                                           

 Description :  This function give some statistic on the noisy pixels:      
            - The percent of time the pixels are off and        
          - The number of pixels which are off at a certain time.     
         The statistics are build for a science window group      
         or an observation group. The calculation are limited to the  
         "good" time interval.  

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
   idxListPtr   dal_element*  I     - Pointer to the INDEX of ISGRI pixel 
                  switches list    
   numGti         int   I     - number of uninterupted "good"        
                  time intervals writtewn to 'gti'     
   gtiStart       OBTime* I     - array of OBTs giving the starts of   
                the GTIs.             
   gtiStop        OBTime* I     - array of OBTs giving the ends of     
                the GTIs.            
   pixPercOffTime double**  O     - array (1 cell per pixel) The value   
                  in the cell is the percentage of     
                time, the pixel was off.       
   numChg     long*     O     - array of long, 1 value per MCE and   
                  1 for the detector.          
                give the number of changes that has  
                been observed.                 
   effTime        OBTime**  O     - array of OBTs. 1 array per MCE and   
                  1 for the detector.        
                OBT from which the number of pixel   
                off is valid.          
   numPixOff      int** O     - array of int. 1 array per MCE and                       1 for the detector.        
                number of pixel off.         
   status         int        I/O      - Error code                           
                             

*****************************************************************************/
int DAL3IBISicaIsgriNoisEff(dal_element *idxListPtr,
          int          numGti,  /* min == 1 */
          OBTime      *gtiStart,
          OBTime      *gtiStop,
          double       pixPercOffTime[ISGRI_SIZE][ISGRI_SIZE],
          long         num_change[IBIS_NUM_BLOCK+1], 
          OBTime     (*eff_time)[IBIS_NUM_BLOCK+1],
          int        (*num_pixOff)[IBIS_NUM_BLOCK+1],
          int          status)
{
int iy, iz; /* indices to define the pixel positions */
int im;     /* indices to define the module number + the detector */
int ig;     /* indices to define the GTI interval */
int ic;     /* indices to define the temporary OFF pixel number timeline */
int iSw;    /* indices to define the pixel switch */
int m, n;   /* define the module number from the pixel position */
  
/* temporary OFF pixels number timeline */
long         nCh[IBIS_NUM_BLOCK+1]; 
long         numChg[IBIS_NUM_BLOCK+1];
OBTime     (*effTime)[IBIS_NUM_BLOCK+1] = NULL;
int        (*numPixOff)[IBIS_NUM_BLOCK+1] = NULL;

/* pixel Switches list */
DAL3_Byte *Y_switch    = NULL;
DAL3_Byte *Z_switch    = NULL;
OBTime *timeDetect     = NULL;
OBTime *timeSwitch     = NULL;
DAL3_Byte *flag_switch = NULL;
long numSwitch = 0;

int alert[ISGRI_SIZE][ISGRI_SIZE];
DAL3_Byte pixStatus[ISGRI_SIZE][ISGRI_SIZE];
OBTime last_switch[ISGRI_SIZE][ISGRI_SIZE];
OBTime totTime[ISGRI_SIZE][ISGRI_SIZE];
OBTime elapsedObt = 0;
OBTime totGti = 0;
    
/* 
 *  Beginning of the function
 * =========================== 
 */
  if(status != ISDC_OK) return(status);
  if(idxListPtr == NULL || numGti < 1) return(DAL3GEN_INVALID_PARAMETER);
  
  /* 
   *  Read the ISGRI pixel Switches list
   * ===================================== 
   */
  status = DAL3IBISgetSizeSwitchList( idxListPtr, gtiStart[0], gtiStop[numGti-1], &numSwitch, status);
  if(status == DAL3IBIS_INDEX_EMPTY)
  {
    numSwitch = 0;
    status = ISDC_OK;
  }
  if(numSwitch > 0 && status == ISDC_OK) 
  {
    status = DAL3IBISallocateSwitchList( &Y_switch, &Z_switch, &timeDetect, &timeSwitch, &flag_switch, numSwitch, status);
    status = DAL3IBISgetSwitchList( idxListPtr, gtiStart[0], gtiStop[numGti-1], Y_switch, Z_switch, timeDetect, timeSwitch, flag_switch, &numSwitch, status);
  }
  if(status != ISDC_OK) return(status);
  
  /*
   *   Memory allocation 
   *  ====================
   */

  if( (numPixOff = malloc((IBIS_NUM_BLOCK+1) * (numSwitch +1) * sizeof(int))) == NULL ||
      (effTime = malloc((IBIS_NUM_BLOCK+1) * (numSwitch +1) * sizeof(OBTime))) == NULL)
  {
    status = DAL_MALLOC_ERROR;
    return(status);
  } 
  /*
   *   Initialization 
   *  ================
   */
  for(im=0; im< (IBIS_NUM_BLOCK+1); im++)
  {
    numChg[im]= 0; 
    for(iSw =0; iSw < numSwitch+1; iSw++)
    {
      numPixOff[iSw][im] = 0;
      effTime[iSw][im] = 0;
    }
  }

  for(iy=0; iy<ISGRI_SIZE; iy++)
  {
    for(iz=0; iz<ISGRI_SIZE; iz++)
    {
      pixStatus[iy][iz] = DAL3IBIS_IPIX_UNKNOWN;
      last_switch[iy][iz] = timeSwitch[0];
      totTime[iy][iz] = 0;
      alert[iy][iz] = 0;
    }
  }

  /*
   *   Get initial status  
   *  ====================
   */

  iSw = 0;
  while(iSw < numSwitch && timeSwitch[iSw] < gtiStart[0])
  {
          switch (flag_switch[iSw])
    {
      case DAL3IBIS_IPIX_OFF    :
          pixStatus[Y_switch[iSw]][Z_switch[iSw]] = DAL3IBIS_IPIX_OFF;
          break;
      case DAL3IBIS_IPIX_DUBIOUS  :
          pixStatus[Y_switch[iSw]][Z_switch[iSw]] = DAL3IBIS_IPIX_DUBIOUS;
          break;
      case DAL3IBIS_SWITCH_ON_OFF   :
          pixStatus[Y_switch[iSw]][Z_switch[iSw]] = DAL3IBIS_IPIX_DUBIOUS;
          break;
      case DAL3IBIS_IPIX_ON     :
          pixStatus[Y_switch[iSw]][Z_switch[iSw]] = DAL3IBIS_IPIX_ON;
          break;
      case DAL3IBIS_SWITCH_OFF_ON   :
          pixStatus[Y_switch[iSw]][Z_switch[iSw]] = DAL3IBIS_IPIX_ON;
          break;
      default:
          status = DAL3IBIS_UNEXPECTED_SWITCH_FLAG;
          return(status);
          break;
    }
    last_switch[Y_switch[iSw]][Z_switch[iSw]] = timeSwitch[iSw];
    iSw++;
  }
  for(iy=0; iy<ISGRI_SIZE; iy++)
  {
    for(iz=0; iz<ISGRI_SIZE; iz++)
    {
      if(pixStatus[iy][iz] == DAL3IBIS_IPIX_UNKNOWN)
      {
            pixStatus[iy][iz] = DAL3IBIS_IPIX_ON;
            if(iSw > 0) last_switch[iy][iz] = timeDetect[iSw-1];
            else last_switch[iy][iz] = timeDetect[0];
      }

      if(pixStatus[iy][iz] != DAL3IBIS_IPIX_ON)
      { 
        if(iy >= ISGRI_SIZE/2) { m=0; }
              else { m = 4; }
        n = 3 - (iz / (ISGRI_SIZE/4));
        numPixOff[0][m+n] ++;
        if( numPixOff[0][m+n] == 1) effTime[0][m+n] = last_switch[iy][iz];
        else if(last_switch[iy][iz] > effTime[0][m+n]) effTime[0][m+n] = last_switch[iy][iz];
          numPixOff[0][8] ++;
        if( numPixOff[0][8] == 1) effTime[0][8] = last_switch[iy][iz];
        else if(last_switch[iy][iz] > effTime[0][8]) effTime[0][8] = last_switch[iy][iz];
      }
    }
  }
  for(im=0; im< (IBIS_NUM_BLOCK+1); im++){
    numChg[im]++;
    numPixOff[numChg[im]][im] = numPixOff[numChg[im]-1][im] ;
    effTime[numChg[im]][im] = effTime[numChg[im]-1][im];
  }
  /*
   *   Loop over all switches
   *  ========================
   */

  for(; iSw < numSwitch && timeDetect[iSw] <= gtiStop[numGti-1]; iSw++)
  {
    /*
     *  Number of OFF pixels / modules
     */
    if(Y_switch[iSw] >= ISGRI_SIZE/2) m=0;
    else m = 4;
    n = 3 - (Z_switch[iSw] / (ISGRI_SIZE/4));
    if((timeSwitch[iSw] > effTime[numChg[m+n]][m+n] ) && 
       (flag_switch[iSw] == DAL3IBIS_SWITCH_ON_OFF ||
        flag_switch[iSw] == DAL3IBIS_SWITCH_OFF_ON  )   )
    {
      numChg[m+n]++;
      numPixOff[numChg[m+n]][m+n] = numPixOff[numChg[m+n]-1][m+n] ;
      effTime[numChg[m+n]][m+n]   = timeSwitch[iSw];
      if(timeSwitch[iSw] > effTime[numChg[8]][8] )
      {
        numChg[8]++;
        numPixOff[numChg[8]][8]     = numPixOff[numChg[8]-1][8];
        effTime[numChg[8]][8]       = timeSwitch[iSw];
      }
    }
    if(flag_switch[iSw] == DAL3IBIS_SWITCH_ON_OFF)
    {
      numPixOff[numChg[m+n]-1][m+n] ++;
      numPixOff[numChg[8]-1][8] ++;
    }
    else if(flag_switch[iSw] == DAL3IBIS_SWITCH_OFF_ON)
    {
      numPixOff[numChg[m+n]][m+n] --;
      numPixOff[numChg[8]][8] --;
    }
    /*
     * percentage of OFF time of each pixels 
     */
    switch (flag_switch[iSw]) 
    {
      case DAL3IBIS_IPIX_OFF    :
      case DAL3IBIS_IPIX_DUBIOUS  :
      case DAL3IBIS_IPIX_ON     :
        if(pixStatus[Y_switch[iSw]][Z_switch[iSw]] != flag_switch[iSw])
        {
          alert[Y_switch[iSw]][Z_switch[iSw]]++;  
        }
        break;
      case DAL3IBIS_SWITCH_ON_OFF   :
        if(pixStatus[Y_switch[iSw]][Z_switch[iSw]] == DAL3IBIS_IPIX_ON)
        {
          pixStatus[Y_switch[iSw]][Z_switch[iSw]] = DAL3IBIS_IPIX_DUBIOUS;
          last_switch[Y_switch[iSw]][Z_switch[iSw]] = timeDetect[iSw];
        }
        else
        {
          alert[Y_switch[iSw]][Z_switch[iSw]]++;  
        }
          break;
      case DAL3IBIS_SWITCH_OFF_ON   :
        if(pixStatus[Y_switch[iSw]][Z_switch[iSw]] == DAL3IBIS_IPIX_ON ||
           pixStatus[Y_switch[iSw]][Z_switch[iSw]] == DAL3IBIS_IPIX_OFF)
        {
          alert[Y_switch[iSw]][Z_switch[iSw]]++;  
        }
          if(pixStatus[Y_switch[iSw]][Z_switch[iSw]] == DAL3IBIS_IPIX_DUBIOUS ||
           pixStatus[Y_switch[iSw]][Z_switch[iSw]] == DAL3IBIS_IPIX_OFF)
          {
          for(ig = 0; ig < numGti; ig++)
          {
            if(( last_switch[Y_switch[iSw]][Z_switch[iSw]] <= gtiStart[ig] && gtiStart[ig] <= timeSwitch[iSw] ) ||
               ( last_switch[Y_switch[iSw]][Z_switch[iSw]] <= gtiStop[ig]  && gtiStop[ig]  <= timeSwitch[iSw] ))
            { 
              if(gtiStart[ig] <= last_switch[Y_switch[iSw]][Z_switch[iSw]] && last_switch[Y_switch[iSw]][Z_switch[iSw]] <= gtiStop[ig] && timeSwitch[iSw] > gtiStop[ig])
              {
                status = DAL3GENelapsedOBT( last_switch[Y_switch[iSw]][Z_switch[iSw]], gtiStop[ig], &elapsedObt, status);
                if(status != ISDC_OK) return(status);
                totTime[Y_switch[iSw]][Z_switch[iSw]] += elapsedObt;
              } 
              else if(last_switch[Y_switch[iSw]][Z_switch[iSw]] <= gtiStart[ig] && gtiStop[ig] <= timeSwitch[iSw]) 
              {
                status = DAL3GENelapsedOBT(gtiStart[ig] , gtiStop[ig], &elapsedObt, status);
                if(status != ISDC_OK) return(status);
                totTime[Y_switch[iSw]][Z_switch[iSw]] += elapsedObt;
              } 
              else if( last_switch[Y_switch[iSw]][Z_switch[iSw]] < gtiStart[ig] && gtiStart[ig] <= timeSwitch[iSw] &&  timeSwitch[iSw] <= gtiStop[ig])
              {
                status = DAL3GENelapsedOBT(gtiStart[ig] , timeSwitch[iSw], &elapsedObt, status);
                if(status != ISDC_OK) return(status);
                totTime[Y_switch[iSw]][Z_switch[iSw]] += elapsedObt;
              }
            }
          }
          pixStatus[Y_switch[iSw]][Z_switch[iSw]] = DAL3IBIS_IPIX_ON;
          last_switch[Y_switch[iSw]][Z_switch[iSw]] = timeSwitch[iSw];
        }
        break;
      default:
        status = DAL3IBIS_UNEXPECTED_SWITCH_FLAG;
        return(status);
        break;
    }
  }
  /* end status */
  for(im=0; im< (IBIS_NUM_BLOCK+1); im++){
    numPixOff[numChg[im]+1][im] = numPixOff[numChg[im]][im] ;
    effTime[numChg[im]+1][im] = gtiStop[numGti-1];
  }

  /*
   *  Calculate the % off time for each pixels
   * ==========================================
   */
  totGti = 0;
  for(ig = 0; ig < numGti; ig++){
    status = DAL3GENelapsedOBT(gtiStart[ig] , gtiStop[ig], &elapsedObt, status);
    if(status != ISDC_OK) return(status);
    totGti += elapsedObt;
  }

  for(iy=0; iy<ISGRI_SIZE; iy++)
  {
    for(iz=0; iz<ISGRI_SIZE; iz++)
    {
      if(pixStatus[iy][iz] == DAL3IBIS_IPIX_OFF)
      { 
        pixPercOffTime[iy][iz] = 100.0;
      }
      else if(pixStatus[iy][iz] == DAL3IBIS_IPIX_ON)
      {
        pixPercOffTime[iy][iz] = (double)totTime[iy][iz] / (double)totGti * 100.0;
      }
      else if(pixStatus[iy][iz] == DAL3IBIS_IPIX_DUBIOUS)
      {
        for(ig = 0; ig < numGti; ig++)
        {
          if(( last_switch[Y_switch[iSw]][Z_switch[iSw]] <= gtiStart[ig] && gtiStart[ig] <= timeSwitch[iSw] ) ||
             ( last_switch[Y_switch[iSw]][Z_switch[iSw]] <= gtiStop[ig]   && gtiStop[ig]   <= gtiStop[numGti-1] ))
            { 
            if(gtiStart[ig] <= last_switch[Y_switch[iSw]][Z_switch[iSw]] && last_switch[Y_switch[iSw]][Z_switch[iSw]] <= gtiStop[ig] && gtiStop[numGti-1] > gtiStop[ig])
            {
              status = DAL3GENelapsedOBT( last_switch[Y_switch[iSw]][Z_switch[iSw]], gtiStop[ig], &elapsedObt, status);
              if(status != ISDC_OK) return(status);
              totTime[Y_switch[iSw]][Z_switch[iSw]] += elapsedObt;
            } 
            else if(last_switch[Y_switch[iSw]][Z_switch[iSw]] <= gtiStart[ig] && gtiStop[ig] <= gtiStop[numGti-1]) 
            {
              status = DAL3GENelapsedOBT(gtiStart[ig] , gtiStop[ig], &elapsedObt, status);
              if(status != ISDC_OK) return(status);
              totTime[Y_switch[iSw]][Z_switch[iSw]] += elapsedObt;
            } 
            else if( last_switch[Y_switch[iSw]][Z_switch[iSw]] < gtiStart[ig] && gtiStart[ig] <= gtiStop[numGti-1] && gtiStop[numGti-1] <= gtiStop[ig])
            {
              status = DAL3GENelapsedOBT(gtiStart[ig], gtiStop[numGti-1], &elapsedObt, status);
              if(status != ISDC_OK) return(status);
              totTime[Y_switch[iSw]][Z_switch[iSw]] += elapsedObt;
            }
          }
        }
        pixPercOffTime[iy][iz] = (double)totTime[iy][iz] / (double)totGti * 100.0;
      }
      /*              
      if(alert[iy][iz] > 0) pixPercOffTime[iy][iz] *= -1;
      */
    }
  }
  
  /* 
   *   Keep only the data for the Good Time Intervals for the  Num Off pixels / module
   * ==================================================================================
   */
  for(im=0; im< (IBIS_NUM_BLOCK+1); im++)
  {
    nCh[im] = 1;
    ic = 0;
    /* Loop on the GTI intervals */
    for(ig = 0; ig < numGti; ig ++)
    { 
      while( ic < numChg[im] && effTime[ic][im]  <= gtiStop[ig])
      {
        /* Get the number of pixel OFF at the beginning of an interval */
        if(effTime[ic][im] < gtiStart[ig])
        { 
          num_pixOff[nCh[im]-1][im] =  numPixOff[ic][im];
          eff_time[nCh[im]-1][im] = gtiStart[ig];
        /* Get the number of pixels OFF in an interval */
        } 
        else if( gtiStart[ig] <= effTime[ic][im] && 
                 effTime[ic][im] <= gtiStop[ig] ) 
        {
          nCh[im] ++;
          num_pixOff[nCh[im]-1][im] =  numPixOff[ic][im];
          eff_time[nCh[im]-1][im] = effTime[ic][im];
        }
        ic ++;
      }
      /* Get the number of pixel OFF at the end of an interval */
      nCh[im] ++;
      num_pixOff[nCh[im]-1][im] = num_pixOff[ic][im];
      eff_time[nCh[im]-1][im] = gtiStop[ig];
    }
    num_change[im] = nCh[im]-1;
  }
  free(numPixOff); 
  free(effTime);
  if(numSwitch > 0)
  {
    status = DAL3IBISfreeSwitchList( Y_switch, Z_switch, timeDetect, timeSwitch, flag_switch, status);
  }
  return(status);
}
/* EOF */

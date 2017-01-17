/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                              ICA ENERGY                                  */
/*                                                                           */
/*  Authors: */
/*  Date:    */
/*  Version: */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*                                                                           */
/*  ==================                                                       */
/*  1.                                                                         */
/*****************************************************************************/

#include "dal3ibis.h"
#include "dal3ibis_calib.h"
#include "dal3hk.h"
#include "ril.h"
            
/****************************************************************************
                                                                          
***************************************************************************/

#define DS_ISGR_HK   "IBIS-DPE.-CNV"
#define KEY_DEF_BIAS      -120.0 
#define KEY_DEF_TEMP        -8.0    /* default when HK1 missing */
#define DS_ISGR_RAW       "ISGR-EVTS-ALL"
#define DS_ISGR_GO        "ISGR-OFFS-MOD"
#define DS_ISGR_3DL2_MOD  "ISGR-3DL2-MOD"
#define DS_PHG2           "ISGR-GAIN-MOD"
#define DS_PHO2           "ISGR-OFF2-MOD"
/* unused structures since V 6.0
 * #define DS_ISGR_RISE_PRO  "ISGR-RISE-PRO"
 * #define DS_ISGR_SWIT      "IBIS-SWIT-CAL"
 * */
/* unused structures since V 5.6.0
 * #define DS_ENER_MOD       "ISGR-DROP-MOD"
 * #define DS_ISGR_RT        "ISGR-RISE-MOD"
 * */
#define KEY_COL_OUT  "ISGRI_PI"

#define DS_ISGR_HK   "IBIS-DPE.-CNV"
#define KEY_MCE_BIAS "I0E_MCDTE_MBIAS"
#define KEY_DEF_BIAS      -120.0    /* default when HK1 missing */
#define KEY_MAX_BIAS       -60.0    /*  155 to disregard RAW 0  */
#define KEY_MCE_TEMP "I0E_MTEMP2_MMDU"
#define KEY_DEF_TEMP        -8.0    /* default when HK1 missing */
#define KEY_RMS_TEMP         1.2    /* disregard MDU Temp */
#define KEY_MIN_TEMP       -50.5    /* to disregard RAW 0 */

static const double DtempH1[8] = {0.43, -0.39, -0.77, 0.84, -0.78, 1.09, -0.08, -0.31};


int DAL3IBISGetISGRIBiasTemperature(dal_element *workGRP,double *meanBias,double *meanT,int status) {
    int i;
    dal_element * isgrHK1_Ptr;

    status=DALobjectFindElement(workGRP, DS_ISGR_HK, &isgrHK1_Ptr, status);
    if (status != ISDC_OK)
        RILlogMessage(NULL, Warning_2, "%13s bintable NOT found.", DS_ISGR_HK);
    else 
        RILlogMessage(NULL, Log_0, "%13s bintable found.", DS_ISGR_HK);

        for (i=0; i<8; i++)
    {
        meanBias[i]=KEY_DEF_BIAS;
        meanT[i]   =KEY_DEF_TEMP;
    }
};



/************************************************************************
 * FUNCTION:  DAL3IBIS_MceIsgriHkCal
 * DESCRIPTION:
 *  Reads the converted HK1 needed for ISGRI LUT1.
 *  Returns ISDC_OK if everything is fine, else returns an error code.
 * ERROR CODES:
 *  DAL error codes
 *  I_ISGR_ERR_MEMORY             Memory allocation error
 *
 * PARAMETERS:
 *  workGRP       dal_element *      in  DOL of the working group
 *  obtStart           OBTime        in  start of time range
 *  obtEnd             OBTime        in  end   of time range
 *  meanT[]              double  in/out  calculated ISGRI mean temperature per MCE
 *  meanBias[]         double    in/out  calculated mean bias per MCE
 *  chatter               int        in  verbosity level
 *  status                int        in  input status
 * RETURN:            int     current status
 ************************************************************************/
int DAL3IBIS_MceIsgriHkCal(dal_element *workGRP,
                         OBTime       obtStart,
                         OBTime       obtEnd,
                         double       meanT[8],
                         double       meanBias[8],
                         int          chatter,
                         int          status)
{
  RILlogMessage(NULL, Log_0, "reading MCE HK in DAL3IBIS");
  int     j, totMCE;
  char    hkName[20], num[3];
  long    i, nValues,
          totVal[8];
  double  myMean, myTot,
         *hkBuff=NULL;
  OBTime *obtime,
          startTime=DAL3_NO_OBTIME,
          endTime = DAL3_NO_OBTIME;
  dal_dataType dataType;

  if (status != ISDC_OK) return status;
  /* SPR 3686: if OBT limits are valid, use S1 PRP OBT limits */
  if (obtStart != DAL3_NO_OBTIME) {

    if ( (obtime=(OBTime *)calloc(1, sizeof(OBTime))) == NULL)
      return(DAL3IBIS_ERR_MEMORY);
    endTime=obtEnd;
    /* check if OBT are not too close */
  do {
    status=DAL3GENelapsedOBT(obtEnd, obtStart, &startTime, status);
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Warning_1, "Error calculating elapsed OBT");
      RILlogMessage(NULL, Warning_1, "Reverting from status=%d to ISDC_OK",
                                    status);
      status=ISDC_OK;
      break;
    }
    *obtime=(DAL3_OBT_SECOND) * (SEC_DELTA_MIN);
    if (startTime < *obtime) {

      RILlogMessage(NULL, Warning_1, "Last OBT (%020lld) too close from first OBT (%020lld)",
                                    obtEnd, obtStart);
      status=DAL3GENskipOBT(obtStart, *obtime, &startTime, status);
      if (status != ISDC_OK) {
        RILlogMessage(NULL, Warning_1, "Error adding %d seconds to first OBT", SEC_DELTA_MIN);
        RILlogMessage(NULL, Warning_1, "Reverting from status=%d to ISDC_OK",
                                      status);
        status=ISDC_OK;
        break;
      }
      endTime=startTime;
      RILlogMessage(NULL, Warning_1, "Adding %d seconds to first OBT, last OBT = %020lld",
                                    SEC_DELTA_MIN, endTime);
    }
  } while (0);
    free(obtime);
    startTime=obtStart;

  }
  obtime=NULL;
  totMCE=0;
  for (j=0; j<8; j++) {

    strcpy(hkName, KEY_MCE_TEMP);
    sprintf(num, "%d", j);
    strcat(hkName, num);
    /* get the information for the requested data */
    status=DAL3HKgetValueInfo(workGRP, hkName, IBIS, DAL3HK_CONVERTED,
                              startTime, endTime,  &nValues, &dataType, status);
    if (status != ISDC_OK) break;
    if (nValues < 1) {
      RILlogMessage(NULL, Warning_1, "%13s has NO valid row.", DS_ISGR_HK);
      status=DAL3IBIS_NO_VALID_HK;
      break;
    }
    /* now allocate the required memory and get the data values */
    status=DALallocateDataBuffer((void **)&hkBuff, nValues*sizeof(double), status);
    status=DALallocateDataBuffer((void **)&obtime, nValues*sizeof(OBTime), status);
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_1, "Cannot allocate buffers for HK data: %s",
                                    hkName);
      break;
    }
    /* retrieve the housekeeping data from the fits file */
    dataType=DAL_DOUBLE;
    status=DAL3HKgetValues(workGRP, hkName, IBIS, DAL3HK_CONVERTED,
                           startTime, endTime, obtime, hkBuff, dataType, status);
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_1, "Cannot get buffers for HK data: %s",
                                    hkName);
      break;
    }
    totVal[j]=0;
    myMean=0.0;
    for (i=0; i<nValues; i++) {
      if (hkBuff[i]  >  KEY_MIN_TEMP) { myMean+=hkBuff[i]; totVal[j]++; }
    }
    if (totVal[j] < 1) {
      RILlogMessage(NULL, Warning_1, "Column %19s has NO valid data.", hkName);
      RILlogMessage(NULL, Warning_1, "Continue to next HK data");
    }
    else {
      meanT[j]=myMean/totVal[j];
      totMCE++;
    }

  }
  if (status != ISDC_OK) {
    /* big problem: don't even try to get next HK data */
    DAL3HKfreeData(status);
    if (hkBuff != NULL) DALfreeDataBuffer((void *)hkBuff, ISDC_OK);
    if (obtime != NULL) DALfreeDataBuffer((void *)obtime, ISDC_OK);
    return status;
  }
  if (totMCE == 0) {
    myMean=KEY_DEF_TEMP;
    RILlogMessage(NULL, Warning_1, "Using default ISGRI mean temperature: %+6.2f degC",
                                  myMean);
  }
  else {
    myMean=0.0; myTot=0.0;
    for (j=0; j<8; j++)
      if (totVal[j] > 0) { myMean+=meanT[j]; myTot+=totVal[j]; }
    myMean/=totMCE;
    if (chatter > 1)
      RILlogMessage(NULL, Log_1, "Mean temp. (%05.1f values) on %d MCEs: %+6.2f degC",
                                myTot/totMCE, totMCE, myMean);
    /* Check probe is OK, otherwise re-computes the mean with valid values */
    i=totMCE;
    totMCE=0;
    for (j=0; j<8; j++)
      if (totVal[j] > 0) {
        if (fabs(meanT[j]-myMean-DtempH1[j]) > KEY_RMS_TEMP) {
          RILlogMessage(NULL, Warning_2,
                             "REJECTING mean temp. on MDU%d: %+6.2f degC",
                             j, meanT[j]);
          totVal[j]=0;
        }
        else totMCE++;
      }
    if (i != totMCE) {
      if (totMCE == 0) {
/*        myMean=KEY_DEF_TEMP; keep previous calculation SPR 4838*/
        RILlogMessage(NULL, Warning_2,
                           "NO new mean temp., CHANGE DtempH1 array");
      }
      else {
        myMean=0.0; myTot=0.0;
        for (j=0; j<8; j++)
          if (totVal[j] > 0) { myMean+=meanT[j]; myTot+=totVal[j]; }
        myMean/=totMCE;
        if (chatter > 1)
          RILlogMessage(NULL, Log_1, "NEW  mean  (%05.1f values) on %d MCEs: %+6.2f degC",
                                    myTot/totMCE, totMCE, myMean);
      }
    }
  }
if (chatter > 3) {
  totMCE=0;
  for (j=0; j<8; j++) {

    strcpy(hkName, KEY_MCE_BIAS);
    sprintf(num, "%d", j);
    strcat(hkName, num);
    /* get the information for the requested data */
    status=DAL3HKgetValueInfo(workGRP, hkName, IBIS, DAL3HK_CONVERTED,
                              startTime, endTime,  &nValues, &dataType, status);
    if (status != ISDC_OK) break;
    if (nValues < 1) {
      RILlogMessage(NULL, Warning_1, "%13s has NO valid row.", DS_ISGR_HK);
      status=DAL3IBIS_NO_VALID_HK;
      break;
    }
    /* now allocate the required memory and get the data values */
    status=DALallocateDataBuffer((void **)&hkBuff, nValues*sizeof(double), status);
    status=DALallocateDataBuffer((void **)&obtime, nValues*sizeof(OBTime), status);
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_1, "Cannot allocate buffers for HK data: %s",
                                    hkName);
      break;
    }
    /* retrieve the housekeeping data from the fits file */
    dataType=DAL_DOUBLE;
    status=DAL3HKgetValues(workGRP, hkName, IBIS, DAL3HK_CONVERTED,
                           startTime, endTime, obtime, hkBuff, dataType, status);
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_1, "Cannot get buffers for HK data: %s",
                                    hkName);
      break;
    }
    totVal[j]=0;
    myMean=0.0;
    for (i=0; i<nValues; i++) {
      if (hkBuff[i]  <  KEY_MAX_BIAS) { myMean+=hkBuff[i]; totVal[j]++; }
    }
    if (totVal[j] < 1) {
      RILlogMessage(NULL, Warning_1, "Column %19s has NO valid data.", hkName);
      RILlogMessage(NULL, Warning_1, "Continue to next HK data");
      /* meanBias[j] not changed, already contains default KEY_DEF_BIAS */
    }
    else {
      meanBias[j]=myMean/totVal[j];
      totMCE++;
    }

  }
  if (totMCE) {
    myMean=0.0; myTot=0.0;
    for (j=0; j<8; j++)
      if (totVal[j] > 0) { myMean+=meanBias[j]; myTot+=totVal[j]; }
    myMean/=totMCE;
    if (chatter > 1)
      RILlogMessage(NULL, Log_1, "Mean bias (%05.1f values) on %d MCEs: %+6.1f V",
                                myTot/totMCE, totMCE, myMean);
  }
  else
    RILlogMessage(NULL, Warning_1, "Using default ISGRI mean bias: %+6.1f V",
                                  KEY_DEF_BIAS);
}
  /* in any case must call this function to free internal buffers */
  DAL3HKfreeData(status);
  if (hkBuff != NULL) DALfreeDataBuffer((void *)hkBuff, ISDC_OK);
  if (obtime != NULL) DALfreeDataBuffer((void *)obtime, ISDC_OK);
  return status;
}


inline int DAL3IBIS_reconstruct_ISGRI_energy(
        long isgriPha,
        short riseTime,
        short isgriY,
        short isgriZ,
        
        double *ptr_isgri_energy,
        double *ptr_isgri_pi,

        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,

        infoEvt_struct *ptr_infoEvt,
        int status
        ) {

    double rt;
    double pha;

    short irt;
    int ipha;
    int ipha2;
    long index_cc;
    double index_cc_real;

    //pixelNo = 128*(int)isgriY[j] + (int)isgriZ[j];

    // 256 channels for LUT2 calibration scaled 
    rt = 2.*riseTime/2.4+5.0;  

    if ((isgriY<0) | (isgriY>=128) |(isgriZ<0) |(isgriZ>128)) {
         RILlogMessage(NULL, Warning_1, "very bad pixel %i %i", isgriY,isgriZ);
         ptr_infoEvt->bad_pixel_yz++;
         *ptr_isgri_energy=0;
         *ptr_isgri_pi=irt;
         return;
    };

   rt = rt * ptr_ISGRI_energy_calibration->LUT1.rt_gain[isgriY][isgriZ] + ptr_ISGRI_energy_calibration->LUT1.rt_offset[isgriY][isgriZ];
    pha = isgriPha * ptr_ISGRI_energy_calibration->LUT1.pha_gain[isgriY][isgriZ] + ptr_ISGRI_energy_calibration->LUT1.pha_offset[isgriY][isgriZ];
    pha=isgriPha;


    /// compression to LUT2 index, optimize
    irt = round(rt); 
    ipha = floor(pha/2);
    
    if (irt < 0)        {irt=0;   ptr_infoEvt->rt_too_low++;}
    else if (irt >= ISGRI_LUT2_MOD_N_RT) {irt=ISGRI_LUT2_MOD_N_RT-1; ptr_infoEvt->rt_too_high++;}

    if (ipha >=ISGRI_LUT2_MOD_N_PHA-1) {ipha=ISGRI_LUT2_MOD_N_PHA-2; ptr_infoEvt->pha_too_high++;}
    else if (ipha<0) {ipha=0; ptr_infoEvt->pha_too_low++;}

    *ptr_isgri_energy=ptr_ISGRI_energy_calibration->LUT2[irt][ipha] \
            +(ptr_ISGRI_energy_calibration->LUT2[irt][ipha+1]-ptr_ISGRI_energy_calibration->LUT2[irt][ipha])*(pha-(double)ipha);
    // only pha interpolation, as before

    if (ptr_isgri_energy <= 0) {
        ptr_infoEvt->negative_energy++;
        *ptr_isgri_energy=0;
    };

    *ptr_isgri_pi=irt;
}

int DAL3IBIS_reconstruct_ISGRI_energies(
        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,
        ISGRI_events_struct *ptr_ISGRI_events,
        int chatter,
        int status
        ) {

    long i,x;

    for (i=0; i<ptr_ISGRI_events->numEvents; i++) {

        DAL3IBIS_reconstruct_ISGRI_energy(
                ptr_ISGRI_events->isgriPha[i],
                ptr_ISGRI_events->riseTime[i],
                ptr_ISGRI_events->isgriY[i],
                ptr_ISGRI_events->isgriZ[i],

                &ptr_ISGRI_events->isgri_energy[i],
                &ptr_ISGRI_events->isgri_pi[i],

                ptr_ISGRI_energy_calibration,

                &ptr_ISGRI_events->infoEvt,
                status
                );
    }


    if (chatter>2) {
        RILlogMessage(NULL, Log_0, "   bad pixel YZ: %li",ptr_ISGRI_events->infoEvt.bad_pixel_yz);
        RILlogMessage(NULL, Log_0, "     RT too low: %li",ptr_ISGRI_events->infoEvt.rt_too_low);
        RILlogMessage(NULL, Log_0, "    RT too high: %li",ptr_ISGRI_events->infoEvt.rt_too_high);
        RILlogMessage(NULL, Log_0, "    PHA too low: %li",ptr_ISGRI_events->infoEvt.pha_too_low);
        RILlogMessage(NULL, Log_0, "   PHA too high: %li",ptr_ISGRI_events->infoEvt.pha_too_high);
        RILlogMessage(NULL, Log_0, "negative energy: %li",ptr_ISGRI_events->infoEvt.negative_energy);
    };
};

void allocate_2d(void *** ptr, int N1, int N2, int elem, int status) {
    int i;
    RILlogMessage(NULL,Log_0,"will allocate %i",N1*sizeof(char*));
    *ptr=NULL;
    status=DALallocateDataBuffer((void **)&(*ptr), N1*sizeof(char*), status);
    for (i=0;i<N1;i++) {
        (*ptr)[i]=NULL;
        status=DALallocateDataBuffer((void **)&((*ptr)[i]), N2*elem, status);
    };
    return status;
}

void deallocate_2d() {
    //status=DALallocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2), ISGRI_LUT2_MOD_N_RT*sizeof(char*), status);
    //for (i=0;i<ISGRI_LUT2_MOD_N_RT;i++) 
    //    status=DALallocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2[i]), ISGRI_LUT2_MOD_N_PHA*sizeof(DAL_DOUBLE), status);
}

int DAL3IBIS_init_ISGRI_energy_calibration(ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration) {
    int status=ISDC_OK;
    int i_mdu,i;

    for (i_mdu=0; i_mdu<8;i_mdu++) {
        ptr_ISGRI_energy_calibration->MDU_correction.pha_offset[i_mdu]=0;
        ptr_ISGRI_energy_calibration->MDU_correction.pha_gain[i_mdu]=0;
        ptr_ISGRI_energy_calibration->MDU_correction.pha_gain2[i_mdu]=0;
        ptr_ISGRI_energy_calibration->MDU_correction.rt_offset[i_mdu]=0;
        ptr_ISGRI_energy_calibration->MDU_correction.rt_gain[i_mdu]=0;
        ptr_ISGRI_energy_calibration->MDU_correction.rt_pha_cross_gain[i_mdu]=0;
    } 

    struct LUT1_struct {
        double ** pha_gain;
        double ** pha_offset;
        double ** rt_gain;
        double ** rt_offset;
    } LUT1;

    allocate_2d(&ptr_ISGRI_energy_calibration->LUT2, ISGRI_LUT2_MOD_N_RT, ISGRI_LUT2_MOD_N_PHA, sizeof(DAL_DOUBLE),status);
    
    allocate_2d(&ptr_ISGRI_energy_calibration->LUT1.pha_gain, 128, 128, sizeof(DAL_DOUBLE),status);
    allocate_2d(&ptr_ISGRI_energy_calibration->LUT1.pha_offset, 128, 128, sizeof(DAL_DOUBLE),status);
    allocate_2d(&ptr_ISGRI_energy_calibration->LUT1.rt_gain, 128, 128, sizeof(DAL_DOUBLE),status);
    allocate_2d(&ptr_ISGRI_energy_calibration->LUT1.rt_offset, 128, 128, sizeof(DAL_DOUBLE),status);

    /*
    struct LUT2_rapid_evolution_struct { // per pointing in principle
        double * ijd;
        double * pha_gain;
        double * pha_gain2;
        double * pha_offset;
        double * rt_gain;
        double * rt_offset;
    } LUT2_rapid_evolution_struct;
*/

}

int DAL3IBISupdateTBiasCorrection( dal_element *workGRP,
        OBTime obtStart, OBTime obtEnd,
        ISGRI_energy_calibration_struct *ISGRI_energy_calibration,
        int chatter,
        int          status ) {

    int i;
    dal_element * isgrHK1_Ptr;

    double meanBias[N_MDU];
    double meanT[N_MDU];

    char logString[DAL_BIG_STRING];
    char tmp_string[DAL_BIG_STRING];

    status=DALobjectFindElement(workGRP, DS_ISGR_HK, &isgrHK1_Ptr, status);
    if (status != ISDC_OK)
        RILlogMessage(NULL, Warning_2, "%13s bintable NOT found.", DS_ISGR_HK);
    else if (chatter > 2)
        RILlogMessage(NULL, Log_0, "%13s bintable found.", DS_ISGR_HK);

    for (i=0; i<8; i++) 
    {
        meanBias[i]=KEY_DEF_BIAS;
        meanT[i]   =KEY_DEF_TEMP;
    }

    status=DAL3IBIS_MceIsgriHkCal(workGRP, obtStart, obtEnd,
            meanT, meanBias, chatter, status);

    if (status != ISDC_OK) {
        RILlogMessage(NULL, Warning_2, "Reverting from status=%d to ISDC_OK",
                status);
        RILlogMessage(NULL, Warning_2, "Using constant ISGRI temperature and bias (%+6.2f %+6.1f)",
                KEY_DEF_TEMP, KEY_DEF_BIAS);
    }
    else if (chatter > 3) {
        RILlogMessage(NULL, Log_0, "Mean ISGRI module bias (V):");
        strcpy(logString, "");
        for (i=0; i<8; i++) {
            sprintf(tmp_string, " %+6.1f", meanBias[i]);
            strcat(logString, tmp_string);
        }
        RILlogMessage(NULL, Log_0, logString);
        RILlogMessage(NULL, Log_0, "Mean ISGRI module Temperature (C):");
        strcpy(logString, "");
        for (i=0; i<8; i++) {
            sprintf(tmp_string, " %+6.1f", meanT[i]);
            strcat(logString, tmp_string);
        }
        RILlogMessage(NULL, Log_0, logString);
    }
    status=ISDC_OK;
}


int DAL3IBIS_read_ISGRI_events(dal_element *workGRP,
                               ISGRI_events_struct *ptr_ISGRI_events,
                               int gti,
                               int chatter,
                               int status)
{
  short  selected=0;
  long   buffSize;

  dal_dataType type;
  ISDCLevel    myLevel;

  do {

    selected=1;
    if (gti) myLevel=PRP;  else myLevel=RAW;
    status=DAL3IBISselectEvents(workGRP, ISGRI_EVTS, myLevel, gti,
                                &ptr_ISGRI_events->obtStart, &ptr_ISGRI_events->obtEnd, NULL, status);
    status=DAL3IBISgetNumEvents(&ptr_ISGRI_events->numEvents, status);
    //status=DAL3IBISgetNumEvents(&(ptr_ISGRI_events->numEvents), status);

    if ((status == DAL3IBIS_NO_IBIS_EVENTS) || (status == DAL_TABLE_HAS_NO_ROWS)) {
      RILlogMessage(NULL, Warning_1, "Reverting from status=%d to ISDC_OK", status);
      RILlogMessage(NULL, Warning_1, "NO event selected. Execution stopped.");
    //  status=ISDC_OK;
      ptr_ISGRI_events->numEvents=0l;
      break;
    }
    else if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Events selection failed. Status=%d",
                                  status);
      break;
    }
    /* test useful when: dal3gen>5.0.0 or RAW <> PRP (0 rows, useGTI=yes) */
    if (ptr_ISGRI_events->numEvents == 0l) {
      RILlogMessage(NULL, Warning_1, "NO event selected. Execution stopped.");
      break;
    }
    if (chatter > 0)
      RILlogMessage(NULL, Log_2, "Number of selected events: %9ld", ptr_ISGRI_events->numEvents);

  /*#################################################################*/
  /* Allocate memory buffers */
  /*#################################################################*/
    ptr_ISGRI_events->isgriPha=NULL;
    ptr_ISGRI_events->riseTime=NULL;
    ptr_ISGRI_events->isgriY=NULL;
    ptr_ISGRI_events->isgriZ=NULL;
    
    ptr_ISGRI_events->isgri_energy=NULL;
    ptr_ISGRI_events->isgri_pi=NULL;

    buffSize= ptr_ISGRI_events->numEvents * sizeof(DAL3_Word);
    status=DALallocateDataBuffer((void **)&(ptr_ISGRI_events->isgriPha), buffSize, status);
    buffSize= ptr_ISGRI_events->numEvents * sizeof(DAL3_Byte);
    status=DALallocateDataBuffer((void **)&(ptr_ISGRI_events->riseTime), buffSize, status);
    status=DALallocateDataBuffer((void **)&(ptr_ISGRI_events->isgriY),   buffSize, status);
    status=DALallocateDataBuffer((void **)&(ptr_ISGRI_events->isgriZ),   buffSize, status);
    
    buffSize= ptr_ISGRI_events->numEvents * sizeof(double);
    status=DALallocateDataBuffer((void **)&(ptr_ISGRI_events->isgri_energy),   buffSize, status);
    status=DALallocateDataBuffer((void **)&(ptr_ISGRI_events->isgri_pi),   buffSize, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot allocate buffers for input/output data");
      break;
    }

  /*#################################################################*/
  /* Read RAW data */
  /*#################################################################*/
    type=DAL_USHORT;
    status=DAL3IBISgetEvents(ISGRI_PHA, &type, (void *)ptr_ISGRI_events->isgriPha, status);
    type=DAL_BYTE;
    status=DAL3IBISgetEvents(RISE_TIME, &type, (void *)ptr_ISGRI_events->riseTime, status);
    type=DAL_BYTE;
    status=DAL3IBISgetEvents(ISGRI_Y,   &type, (void *)ptr_ISGRI_events->isgriY,   status);
    type=DAL_BYTE;
    status=DAL3IBISgetEvents(ISGRI_Z,   &type, (void *)ptr_ISGRI_events->isgriZ,   status);
    if (gti) {
        type=DAL3_OBT;
        status=DAL3IBISgetEventsBins(OB_TIME, &type, 1,1, &ptr_ISGRI_events->obtStart, status);
        buffSize= ptr_ISGRI_events->numEvents;
        status=DAL3IBISgetEventsBins(OB_TIME, &type, buffSize,buffSize, &ptr_ISGRI_events->obtEnd, status);

        if ((chatter > 2) && (gti))
            RILlogMessage(NULL, Log_0, "OBT range: %020lld , %020lld", ptr_ISGRI_events->obtStart, ptr_ISGRI_events->obtEnd);

        if ((ptr_ISGRI_events->obtStart < 0) || (ptr_ISGRI_events->obtEnd < 0)) {
            if (gti) {
                RILlogMessage(NULL, Warning_1, "At least one OBT limit is negative.");
                RILlogMessage(NULL, Warning_1, "Using all ScW to calculate mean bias and temperature.");
            }
            ptr_ISGRI_events->obtStart=DAL3_NO_OBTIME;
            ptr_ISGRI_events->obtEnd=DAL3_NO_OBTIME;
        }
    }
    else {
      ptr_ISGRI_events->obtStart=DAL3_NO_OBTIME;
      ptr_ISGRI_events->obtEnd=DAL3_NO_OBTIME;
    }
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot get input data");
      break;
    }

  } while(0);
  if (selected > 0) status=DAL3IBIScloseEvents(status);
      
  RILlogMessage(NULL, Log_2, "ISGRI event information successfully extracted");

  return status;
}


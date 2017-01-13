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
    dal_element isgrHK1_Ptr;

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


int DAL3IBISGetISGRIEnergy(dal_element *ogPtr,
        int          status) {
    RILlogMessage(NULL, Warning_2, "requested ISGRI energy correction! placeholder here");
};


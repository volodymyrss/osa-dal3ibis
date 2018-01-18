/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                              CALIBRATION                                  */
/*                                                                           */
/*  Authors: Volodymyr Savchenko                                             */
/*  Date:    17/04/15                                                        */
/*  Version: 1.0                                                             */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*                                                                           */
/*  ==================                                                       */
/*  1. created                                                                      */
/*****************************************************************************/

#include "dal3ibis.h"
#include "dal3ibis_calib.h"
#include "dal3ibis_calib_aux.h"
#include "dal3ibis_calib_ebands.h"
#include "dal3hk.h"
#include "dal3aux.h"
#include "ril.h"
            

#define KEY_DEF_BIAS      -120.0 
#define KEY_DEF_TEMP        -8.0    /* default when HK1 missing */

#define DS_ISGR_HK        "IBIS-DPE.-CNV"
#define DS_ISGR_RAW       "ISGR-EVTS-ALL"
#define DS_ISGR_LUT1      "ISGR-OFFS-MOD"
#define DS_ISGR_LUT2      "ISGR-RISE-MOD"
#define DS_ISGR_L2RE      "ISGR-L2RE-MOD"
#define DS_ISGR_MCEC      "ISGR-MCEC-MOD"
#define DS_ISGR_EFFC      "ISGR-EFFC-MOD"
#define DS_PHG2           "ISGR-GAIN-MOD"
#define DS_PHO2           "ISGR-OFF2-MOD"
#define DS_PICS_GO        "PICS-ENER-MOD"


#define KEY_MCE_BIAS "I0E_MCDTE_MBIAS"
#define KEY_DEF_BIAS      -120.0    /* default when HK1 missing */
#define KEY_MAX_BIAS       -60.0    /*  155 to disregard RAW 0  */
#define KEY_MCE_TEMP "I0E_MTEMP2_MMDU"
#define KEY_DEF_TEMP        -8.0    /* default when HK1 missing */
#define KEY_RMS_TEMP         1.2    /* disregard MCE Temp */
#define KEY_MIN_TEMP       -50.5    /* to disregard RAW 0 */

#define I_ISGR_ERR_MEMORY         -122050
#define I_ISGR_ERR_BAD_INPUT      -122051
#define I_ISGR_ERR_ISGR_OFFS_BAD  -122052
#define I_ISGR_ERR_ISGR_RISE_BAD  -122053
#define I_ISGR_ERR_ISGR_OUT_COR   -122054
#define I_ISGR_ERR_IBIS_IREM_BAD  -122055
#define I_ISGR_ERR_ISGR_PHGO2_BAD -122056

#define I_COMP_SCA_ERR_MEMORY              -142050
#define I_COMP_SCA_ERR_BAD_INPUT           -142051
#define I_COMP_SCA_ERR_ISGR_OFFS_BAD       -142052
#define I_COMP_SCA_ERR_ISGR_RISE_BAD       -142053
#define I_COMP_SCA_ERR_PICS_ENER_BAD       -142054
#define I_COMP_SCA_ERR_COMP_OUT_COR        -142055
#define I_COMP_SCA_ERR_IBIS_IREM_BAD       -142056
#define I_COMP_SCA_ERR_ISGR_PHGO2_BAD      -142057


int doICgetNewestDOL(char * category,char * filter, double valid_time, char * DOL,int status) {
    char ic_group[DAL_MAX_STRING];
    char *current_ic;

    if (!(current_ic=getenv("CURRENT_IC"))) {
        RILlogMessage(NULL, Error_1, "requested to search IC in DAL3IBIS, but environment is insufficient!");
        RILlogMessage(NULL, Error_1, "likely, you need to provide the IC structure \"%s\" in the parameter",category);
        return I_ISGR_ERR_BAD_INPUT;
    }


    snprintf(ic_group,DAL_MAX_STRING,"%s/idx/ic/ic_master_file.fits[1]",current_ic); 
    status=ICgetNewestDOL(ic_group,
            "OSA",
            category,filter,valid_time,DOL,status);
    return status;
}


static const double DtempH1[8] = {0.43, -0.39, -0.77, 0.84, -0.78, 1.09, -0.08, -0.31};
double slopeMCE[8]={-1.8,-2.0,-2.3,-2.7,-0.5,-2.4,-0.8,-0.5} ;


// implements temperature and bias dependency additional to the existing LUT1
// (note that bias is forced to be 1.2, see DAL3IBIS_MceIsgriHkCal)
// also this slightly interferes with the revolution-scale MCE correction

int DAL3IBIS_correct_LUT1_for_temperature_bias(
        dal_element *workGRP,
        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,
        IBIS_events_struct *ptr_IBIS_events,
        int chatter,
        int status) {

    int i_y,i_z;
    int pixelNo;
    int mce;
    char logstr_pha_gain[DAL_BIG_STRING];
    char logstr_pha_offset[DAL_BIG_STRING];
    char logstr_rt_gain[DAL_BIG_STRING];
    char logstr_rt_offset[DAL_BIG_STRING];

    double meanTemp[8], meanBias[8];
    status=DAL3IBIS_MceIsgriHkCal(workGRP,ptr_IBIS_events->obtStart,ptr_IBIS_events->obtStop,meanTemp,meanBias,chatter,status);

    sprintf(logstr_pha_gain,  "Bias-Temperature MCE PHA gain  ");
    sprintf(logstr_pha_offset,"                 MCE PHA offset");
    sprintf(logstr_rt_gain,   "                 MCE RT  gain  ");
    sprintf(logstr_rt_offset, "                 MCE RT  offset");

    for (mce=0;mce<8;mce++) {
        sprintf(strchr(logstr_pha_gain, '\0'),
                " %5.1lf%%",(pow(meanTemp[mce],-1.11)*pow(meanBias[mce],-0.0832)-1)*100);

        sprintf(logstr_pha_offset+strlen(logstr_pha_offset),
                " %5.1lf%%",(pow(meanTemp[mce],slopeMCE[mce])*pow(meanBias[mce],0.0288)-1)*100);

        sprintf(logstr_rt_gain+strlen(logstr_rt_gain),
                " %5.1lf%%",(pow(meanTemp[mce],0.518)*pow(meanBias[mce],0.583)-1)*100);
        
        sprintf(logstr_rt_offset+strlen(logstr_rt_offset),
                " %5.1lf%%",(pow(meanTemp[mce],0.625)*pow(meanBias[mce],0.530)-1)*100);
        
    }
    if (chatter>3) {
        RILlogMessage(NULL,Log_0,"%s",logstr_pha_gain);
        RILlogMessage(NULL,Log_0,"%s",logstr_pha_offset);
        RILlogMessage(NULL,Log_0,"%s",logstr_rt_gain);
        RILlogMessage(NULL,Log_0,"%s",logstr_rt_offset);
    }

    for (i_y=0;i_y<128;i_y++)
        for (i_z=0;i_z<128;i_z++)
        {
            pixelNo = yz_to_pixelNo(i_y,i_z);
            mce     = yz_to_mce(i_y,i_z);

            ptr_ISGRI_energy_calibration->LUT1.pha_gain[i_y][i_z]   *=  pow(meanTemp[mce],-1.11)*pow(meanBias[mce],-0.0832);
            ptr_ISGRI_energy_calibration->LUT1.pha_offset[i_y][i_z] *=  pow(meanTemp[mce],slopeMCE[mce])*pow(meanBias[mce],0.0288);
            ptr_ISGRI_energy_calibration->LUT1.rt_gain[i_y][i_z]    *=  pow(meanTemp[mce],0.518)*pow(meanBias[mce],0.583);
            ptr_ISGRI_energy_calibration->LUT1.rt_offset[i_y][i_z]  *=  pow(meanTemp[mce],0.625)*pow(meanBias[mce],0.530);
            
        }

    return status;
}


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
 *
 * rewrite!!!
 *
 *
 ************************************************************************/
int DAL3IBIS_MceIsgriHkCal(dal_element *workGRP,
                         OBTime       obtStart,
                         OBTime       obtStop,
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
    dal_element * isgrHK1_Ptr;

    char logString[DAL_BIG_STRING];
    char tmp_string[DAL_BIG_STRING];
    double  myMean, myTot,
            *hkBuff=NULL;
    OBTime *obtime,
           startTime=DAL3_NO_OBTIME,
           endTime = DAL3_NO_OBTIME;
    dal_dataType dataType;

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

    if (status != ISDC_OK) {
        RILlogMessage(NULL, Log_0, "not ok!");
        return status;
    }
    /* SPR 3686: if OBT limits are valid, use S1 PRP OBT limits */
    if (obtStart != DAL3_NO_OBTIME) {

        if ( (obtime=(OBTime *)calloc(1, sizeof(OBTime))) == NULL)
            return(DAL3IBIS_ERR_MEMORY);

        endTime=obtStop;
        /* check if OBT are not too close */
        do {
            status=DAL3GENelapsedOBT(obtStop, obtStart, &startTime, status);
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
                        obtStop, obtStart);
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
        if (status != ISDC_OK) {
            RILlogMessage(NULL, Error_1, "Cannot get HK value info: %i for %s",status,hkName);
            break;
        }
        if (nValues < 1) {
            RILlogMessage(NULL, Warning_1, "%13s has NO valid row.", DS_ISGR_HK);
            status=DAL3IBIS_NO_VALID_HK;
            break;
        }
        /* now allocate the required memory and get the data values */
        if (status != ISDC_OK) {
            RILlogMessage(NULL, Error_1, "Cannot allocate buffers for HK data: %s",
                    hkName);
            break;
        }
        /* retrieve the housekeeping data from the fits file */
        dataType=DAL_DOUBLE;
        status=DALallocateDataBuffer((void **)&hkBuff, nValues*sizeof(double), status);
        status=DALallocateDataBuffer((void **)&obtime, nValues*sizeof(OBTime), status);
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
        } else {
            meanT[j]=myMean/totVal[j];
            totMCE++;
        }

    }
    if (status != ISDC_OK) {
        /* big problem: don't even try to get next HK data */
        DAL3HKfreeData(status);
        if (hkBuff != NULL) DAL_GC_freeDataBuffer((void *)hkBuff, ISDC_OK);
        if (obtime != NULL) DAL_GC_freeDataBuffer((void *)obtime, ISDC_OK);
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
                            "REJECTING mean temp. on MCE%d: %+6.2f degC",
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
            if (status != ISDC_OK) {
                RILlogMessage(NULL, Error_1, "Cannot allocate buffers for HK data: %s",
                        hkName);
                break;
            }
            /* retrieve the housekeeping data from the fits file */
            status=DALallocateDataBuffer((void **)&hkBuff, nValues*sizeof(double), status);
            status=DALallocateDataBuffer((void **)&obtime, nValues*sizeof(OBTime), status);
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
    if (hkBuff != NULL) DAL_GC_freeDataBuffer((void *)hkBuff, ISDC_OK);
    if (obtime != NULL) DAL_GC_freeDataBuffer((void *)obtime, ISDC_OK);

    if (status != ISDC_OK) {
        RILlogMessage(NULL, Warning_2, "Reverting from status=%d to ISDC_OK",
                status);
        RILlogMessage(NULL, Warning_2, "Using constant ISGRI temperature and bias (%+6.2f %+6.1f)",
                KEY_DEF_TEMP, KEY_DEF_BIAS);
    } else if (chatter > 3) {
        RILlogMessage(NULL, Log_0, "Mean ISGRI module bias (V):");
        strcpy(logString, "");
        for (i=0; i<8; i++) {
            sprintf(hkName, " %+6.1f", meanBias[i]);
            strcat(logString, hkName);
        }
        RILlogMessage(NULL, Log_0, logString);
        RILlogMessage(NULL, Log_0, "Mean ISGRI module Temperature (C):");
        strcpy(logString, "");
        for (i=0; i<8; i++) {
            sprintf(hkName, " %+6.1f", meanT[i]);
            strcat(logString, hkName);
        }
        RILlogMessage(NULL, Log_0, logString);
    }
    status=ISDC_OK;

    for (j=0;j<8;j++)
    {
        meanT[j]=(meanT[j]+273.0)/273.0; /* to scale temperature in ratio to minimum Kelvin */
        //meanBias[j]=-meanBias[j]/100. ; 
        meanBias[j]=1.2;  // because who cares about the bias
    }
        
    return status;
}

static int l2re_index_memory=0; // this global variable will preserve last successfully found index - faster than searching

inline double interpolate_1d_array(double *arr, int i, double bin_fraction) {
    return arr[i]+(arr[i+1]-arr[i])*bin_fraction;
}

void compute_l2re_parameters(
        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,
        double ijd,
        double *ptr_l2re_rt_offset,
        double *ptr_l2re_rt_gain,
        double *ptr_l2re_pha_offset,
        double *ptr_l2re_pha_gain,
        double *ptr_l2re_pha_gain2
    ) {

    int i_l2re=l2re_index_memory;

    long int n_entries=ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.n_entries;
    double *l2re_ijd=ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.ijd;

    double bin_fraction=0;

    int n_steps=0;

    while (1) {
        if ( (l2re_ijd[i_l2re]<ijd) && (l2re_ijd[i_l2re+1]>=ijd)) {
            break;
        }

        if (ijd<=l2re_ijd[i_l2re]) {
            if (i_l2re<=0)
                break;
            i_l2re--;
        }

        if (ijd>l2re_ijd[i_l2re+1]) {
            if (i_l2re>=n_entries-2)
                break;
            i_l2re++;
        }
        n_steps++;
    }

    l2re_index_memory=i_l2re;

    bin_fraction=(ijd-l2re_ijd[i_l2re])/(l2re_ijd[i_l2re+1]-l2re_ijd[i_l2re]);

    *ptr_l2re_rt_offset = interpolate_1d_array(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.rt_offset,i_l2re,bin_fraction);
    *ptr_l2re_rt_gain = interpolate_1d_array(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.rt_gain,i_l2re,bin_fraction);
    *ptr_l2re_pha_offset = interpolate_1d_array(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_offset,i_l2re,bin_fraction);
    *ptr_l2re_pha_gain = interpolate_1d_array(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_gain,i_l2re,bin_fraction);
    *ptr_l2re_pha_gain2 = interpolate_1d_array(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_gain2,i_l2re,bin_fraction);
}

inline int DAL3IBIS_reconstruct_ISGRI_energy(
        long isgriPha,
        short riseTime,
        short isgriY,
        short isgriZ,
        double ijd,
        
        float *ptr_isgri_energy,
        DAL3_Byte *ptr_isgri_pi,

        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,

        infoEvt_struct *ptr_infoEvt,
        int pha_scale,
        int status
        ) {

    double rt;
    double pha;

    short irt;
    short mce;
    int ipha;
    int ipha2;

    double l2re_rt_offset;
    double l2re_rt_gain;
    double l2re_pha_offset;
    double l2re_pha_gain;
    double l2re_pha_gain2;

    double l2re_correction=0;

    if ((isgriY<0) | (isgriY>=128) |(isgriZ<0) |(isgriZ>=128)) { // 127?..
         ptr_infoEvt->bad_pixel_yz++;
         *ptr_isgri_energy=0;
         *ptr_isgri_pi=irt;
         return -1;
    };


    pha = pha_scale * ( isgriPha + DAL3GENrandomDoubleX1() - 0.5);
    rt = riseTime + DAL3GENrandomDoubleX1() - 0.5;
        
    // 256 channels for LUT2 calibration scaled 
    rt = 2.*rt/2.4+5.0;  
    
    rt = rt * ptr_ISGRI_energy_calibration->LUT1.rt_gain[isgriY][isgriZ] + ptr_ISGRI_energy_calibration->LUT1.rt_offset[isgriY][isgriZ];
    pha = pha * ptr_ISGRI_energy_calibration->LUT1.pha_gain[isgriY][isgriZ] + ptr_ISGRI_energy_calibration->LUT1.pha_offset[isgriY][isgriZ];

    
    // MCE correction
    mce     = yz_to_mce(isgriY,isgriZ);

    rt = rt * ptr_ISGRI_energy_calibration->MCE_correction.rt_gain[mce] + ptr_ISGRI_energy_calibration->MCE_correction.rt_offset[mce];
    pha =   ptr_ISGRI_energy_calibration->MCE_correction.pha_offset[mce] 
          + pha * ptr_ISGRI_energy_calibration->MCE_correction.pha_gain[mce] 
          + 2*15./pha * ptr_ISGRI_energy_calibration->MCE_correction.pha_gain2[mce] 
          + rt * ptr_ISGRI_energy_calibration->MCE_correction.rt_pha_cross_gain[mce];

    // LUT2 rapid evolution 
    
    compute_l2re_parameters(ptr_ISGRI_energy_calibration,
                           ijd,
                           &l2re_rt_offset,
                           &l2re_rt_gain,
                           &l2re_pha_offset,
                           &l2re_pha_gain,
                           &l2re_pha_gain2);
    
    rt = l2re_rt_offset + \
         l2re_rt_gain * rt;

    pha = l2re_pha_offset + \
          l2re_pha_gain * pha + \
          l2re_pha_gain2 * pha*pha;

    l2re_correction = l2re_pha_offset + \
                      l2re_pha_gain*60 + \
                      l2re_pha_gain2*60*60;

    /// compression to LUT2 index
    irt = round(rt); 
    
    if (irt < 0)        {irt=0;   ptr_infoEvt->rt_too_low++;}
    else if (irt >= ISGRI_LUT2_N_RT) {irt=ISGRI_LUT2_N_RT-1; ptr_infoEvt->rt_too_high++;}

    ipha = floor(pha);
    

    if (ipha >=ISGRI_LUT2_N_PHA-1) {ipha=ISGRI_LUT2_N_PHA-2; ptr_infoEvt->pha_too_high++;}
    else if (ipha<0) {ipha=0; ptr_infoEvt->pha_too_low++;}

    // only pha interpolation, as before
    *ptr_isgri_energy=ptr_ISGRI_energy_calibration->LUT2[irt+ipha*ISGRI_LUT2_N_RT];
    
    double gradient=(ptr_ISGRI_energy_calibration->LUT2[irt+(ipha+1)*ISGRI_LUT2_N_RT]-ptr_ISGRI_energy_calibration->LUT2[irt+ipha*ISGRI_LUT2_N_RT]); // ipha+1 danger
    
    *ptr_isgri_energy+=gradient*(pha-(double)ipha);

    
    // invalid LUT2 values
    if (*ptr_isgri_energy < 0) {
        ptr_infoEvt->negative_energy++;
        *ptr_isgri_energy=0;
    };

    *ptr_isgri_pi=irt;
    ptr_infoEvt->good++;
    ptr_infoEvt->l2re_correction = (ptr_infoEvt->l2re_correction*(ptr_infoEvt->good-1)+l2re_correction)/ptr_infoEvt->good;
}


inline int DAL3IBIS_reconstruct_PICsIT_energy(
        long picsitPha,
        short picsitY,
        short picsitZ,
        
        float *ptr_picsit_energy,

        PICsIT_energy_calibration_struct *ptr_PICsIT_energy_calibration,

        infoEvt_struct *ptr_infoEvt,
        int pha_scale,
        int status
        ) {

    double pha;

    int ipha;
    
    pha=pha_scale * ( (double)picsitPha + DAL3GENrandomDoubleX1()); 

    int pixelNo =  64*(int)picsitY + (int)picsitZ;

    *ptr_picsit_energy = PICSIT_GAIN * ptr_PICsIT_energy_calibration->gain_offset[0][pixelNo]*pha
                       + PICSIT_OFFSET + ptr_PICsIT_energy_calibration->gain_offset[1][pixelNo];

    //no bad!

    ptr_infoEvt->good++;
}

int DAL3IBIS_reconstruct_Compton_energies(
        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,
        PICsIT_energy_calibration_struct *ptr_PICsIT_energy_calibration,
        IBIS_events_struct *ptr_IBIS_events,
        int chatter,
        int status
        ) {

    long i;

    if (ptr_IBIS_events->event_kind != COMPTON_SGLE && ptr_IBIS_events->event_kind != COMPTON_MULE) 
        return -1;

    RILlogMessage(NULL, Log_0, "reconstructing Compton energies");

    for (i=0; i<ptr_IBIS_events->numEvents; i++) {

        DAL3IBIS_reconstruct_ISGRI_energy(
                ptr_IBIS_events->isgriPha[i],
                ptr_IBIS_events->riseTime[i],
                ptr_IBIS_events->isgriY[i],
                ptr_IBIS_events->isgriZ[i],
                ptr_IBIS_events->IJD[i],

                &ptr_IBIS_events->isgri_energy[i],
                &ptr_IBIS_events->isgri_pi[i],

                ptr_ISGRI_energy_calibration,

                &ptr_IBIS_events->infoEvt,
                ptr_IBIS_events->isgriPha_scale,
                status
                );
        
        DAL3IBIS_reconstruct_PICsIT_energy(
                ptr_IBIS_events->picsitPha[i],
                ptr_IBIS_events->picsitY[i],
                ptr_IBIS_events->picsitZ[i],

                &ptr_IBIS_events->picsit_energy[i],

                ptr_PICsIT_energy_calibration,

                &ptr_IBIS_events->infoEvt,
                ptr_IBIS_events->picsitPha_scale,
                status
                );
    }


    if (chatter>2) {
        RILlogMessage(NULL, Log_0, "           good: %li",ptr_IBIS_events->infoEvt.good);
        RILlogMessage(NULL, Log_0, "   bad pixel YZ: %li",ptr_IBIS_events->infoEvt.bad_pixel_yz);
        RILlogMessage(NULL, Log_0, "     RT too low: %li",ptr_IBIS_events->infoEvt.rt_too_low);
        RILlogMessage(NULL, Log_0, "    RT too high: %li",ptr_IBIS_events->infoEvt.rt_too_high);
        RILlogMessage(NULL, Log_0, "    PHA too low: %li",ptr_IBIS_events->infoEvt.pha_too_low);
        RILlogMessage(NULL, Log_0, "   PHA too high: %li",ptr_IBIS_events->infoEvt.pha_too_high);
        RILlogMessage(NULL, Log_0, "negative energy: %li",ptr_IBIS_events->infoEvt.negative_energy);
        RILlogMessage(NULL, Log_0, "\n");
        RILlogMessage(NULL, Log_0, "L2RE average PHA correction");
        RILlogMessage(NULL, Log_0,"       at 60 keV: %.5lg",ptr_IBIS_events->infoEvt.l2re_correction);
    };

    return status;
};

int DAL3IBIS_reconstruct_ISGRI_energies(
        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,
        IBIS_events_struct *ptr_IBIS_events,
        int chatter,
        int status
        ) {

    long i;
    
    RILlogMessage(NULL, Log_0, "reconstructing ISGRI energies");


    for (i=0; i<ptr_IBIS_events->numEvents; i++) {
        DAL3IBIS_reconstruct_ISGRI_energy(
                ptr_IBIS_events->isgriPha[i],
                ptr_IBIS_events->riseTime[i],
                ptr_IBIS_events->isgriY[i],
                ptr_IBIS_events->isgriZ[i],
                ptr_IBIS_events->IJD[i],

                &ptr_IBIS_events->isgri_energy[i],
                &ptr_IBIS_events->isgri_pi[i],

                ptr_ISGRI_energy_calibration,

                &ptr_IBIS_events->infoEvt,
                1,
                status
                );
    }


    if (chatter>2) {
        RILlogMessage(NULL, Log_0, "           good: %li",ptr_IBIS_events->infoEvt.good);
        RILlogMessage(NULL, Log_0, "   bad pixel YZ: %li",ptr_IBIS_events->infoEvt.bad_pixel_yz);
        RILlogMessage(NULL, Log_0, "     RT too low: %li",ptr_IBIS_events->infoEvt.rt_too_low);
        RILlogMessage(NULL, Log_0, "    RT too high: %li",ptr_IBIS_events->infoEvt.rt_too_high);
        RILlogMessage(NULL, Log_0, "    PHA too low: %li",ptr_IBIS_events->infoEvt.pha_too_low);
        RILlogMessage(NULL, Log_0, "   PHA too high: %li",ptr_IBIS_events->infoEvt.pha_too_high);
        RILlogMessage(NULL, Log_0, "negative energy: %li",ptr_IBIS_events->infoEvt.negative_energy);
        RILlogMessage(NULL, Log_0, "\n");
        RILlogMessage(NULL, Log_0, "L2RE average PHA correction at 60 keV:");
        RILlogMessage(NULL, Log_0,"                : %.5lg",ptr_IBIS_events->infoEvt.l2re_correction);
    };

    return status;
};

int allocate_2d(void *** ptr, int N1, int N2, int elem, int status) {
    int i;
    *ptr=NULL;
    status=DAL_GC_allocateDataBuffer((void **)&(*ptr), N1*sizeof(char*), status, "2d allocation");

    if (status!=ISDC_OK) {
        RILlogMessage(NULL,Error_1,"allocation failed!");
        return status;
    }

    for (i=0;i<N1;i++) {
        (*ptr)[i]=NULL;
        status=DALallocateDataBuffer((void **)&((*ptr)[i]), N2*elem, status); // !! unhandled
    };
    return status;
}

void deallocate_2d() {
    //status=DALallocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2), ISGRI_LUT2_MOD_N_RT*sizeof(char*), status);
    //for (i=0;i<ISGRI_LUT2_MOD_N_RT;i++) 
    //    status=DALallocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2[i]), ISGRI_LUT2_MOD_N_PHA*sizeof(DAL_DOUBLE), status);
}

int DAL3IBIS_init_ISGRI_energy_calibration(ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,int status) {
    int i_mdu;

    for (i_mdu=0; i_mdu<8;i_mdu++) {
        ptr_ISGRI_energy_calibration->MCE_correction.pha_offset[i_mdu]=0;
        ptr_ISGRI_energy_calibration->MCE_correction.pha_gain[i_mdu]=1;
        ptr_ISGRI_energy_calibration->MCE_correction.pha_gain2[i_mdu]=0;
        ptr_ISGRI_energy_calibration->MCE_correction.rt_offset[i_mdu]=0;
        ptr_ISGRI_energy_calibration->MCE_correction.rt_gain[i_mdu]=1;
        ptr_ISGRI_energy_calibration->MCE_correction.rt_pha_cross_gain[i_mdu]=0;
    } 

    status=allocate_2d((void ***)&ptr_ISGRI_energy_calibration->LUT1.pha_gain, 128, 128, sizeof(double),status);
    status=allocate_2d((void ***)&ptr_ISGRI_energy_calibration->LUT1.pha_offset, 128, 128, sizeof(double),status);
    status=allocate_2d((void ***)&ptr_ISGRI_energy_calibration->LUT1.rt_gain, 128, 128, sizeof(double),status);
    status=allocate_2d((void ***)&ptr_ISGRI_energy_calibration->LUT1.rt_offset, 128, 128, sizeof(double),status);
    
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2), ISGRI_LUT2_N_RT*ISGRI_LUT2_N_PHA*sizeof(double), status,"LUT2 evolution IJD");

    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.ijd), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution IJD");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_gain), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution PHA gain");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_gain2), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution PHA gain2");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_offset), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution PHA offset");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.rt_gain), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution RT gain");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.rt_offset), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution RT offset");

    return status;
}


int DAL3IBIS_init_PICsIT_energy_calibration(PICsIT_energy_calibration_struct *ptr_PICsIT_energy_calibration,int status) {
    int i;

    for (i=0; i<PICSIT_GO_N_DATA; i++) {
        status=DAL_GC_allocateDataBuffer((void **)&(ptr_PICsIT_energy_calibration->gain_offset[i]), PICSIT_N_PIX*sizeof(float), status,"PICsIT GO");
    }


    return status;
}

int DAL3IBIS_read_IBIS_events(dal_element *workGRP,
                               IBIS_type event_kind,
                               IBIS_events_struct *ptr_IBIS_events,
                               int gti,
                               int chatter,
                               int status
                               )
{
    ptr_IBIS_events->obtStart=DAL3_NO_OBTIME;
    ptr_IBIS_events->obtStop=DAL3_NO_OBTIME;
    ptr_IBIS_events->numEvents=0;
    ptr_IBIS_events->infoEvt.good=0;
    ptr_IBIS_events->infoEvt.bad_pixel_yz=0;
    ptr_IBIS_events->infoEvt.rt_too_low=0;
    ptr_IBIS_events->infoEvt.rt_too_high=0;
    ptr_IBIS_events->infoEvt.pha_too_low=0;
    ptr_IBIS_events->infoEvt.pha_too_high=0;
    ptr_IBIS_events->infoEvt.negative_energy=0;
    ptr_IBIS_events->infoEvt.l2re_correction=0;
    ptr_IBIS_events->event_kind=event_kind;
    if (event_kind == COMPTON_SGLE || event_kind == COMPTON_MULE ) {
        ptr_IBIS_events->isgriPha_scale=8;
        ptr_IBIS_events->picsitPha_scale=4;
    } else {
        ptr_IBIS_events->isgriPha_scale=1;
        ptr_IBIS_events->picsitPha_scale=0;
    }


  short  selected=0;
  long   buffSize;
  long int numEvents;

  dal_dataType type;
  ISDCLevel    myLevel;

  do {

    selected=1;
    if (gti) myLevel=PRP;  else myLevel=RAW;

    status=DAL3IBISselectEvents(workGRP, event_kind, myLevel, gti,
                                &ptr_IBIS_events->obtStart, &ptr_IBIS_events->obtStop, NULL, status);
    status=DAL3IBISgetNumEvents(&ptr_IBIS_events->numEvents, status);
    
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_1, "selecting events: %i", status);
      return status;
    }


    if ((status == DAL3IBIS_NO_IBIS_EVENTS) || (status == DAL_TABLE_HAS_NO_ROWS)) {
      RILlogMessage(NULL, Warning_1, "Reverting from status=%d to ISDC_OK", status);
      RILlogMessage(NULL, Warning_1, "NO event selected. Execution stopped.");
    //  status=ISDC_OK;
      ptr_IBIS_events->numEvents=0l;
      break;
    }
    else if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Events selection failed. Status=%d",
                                  status);
      break;
    }
    /* test useful when: dal3gen>5.0.0 or RAW <> PRP (0 rows, useGTI=yes) */
    if (ptr_IBIS_events->numEvents == 0l) {
      RILlogMessage(NULL, Warning_1, "NO event selected. Execution stopped.");
      break;
    }
    if (chatter > 0)
      RILlogMessage(NULL, Log_2, "Number of selected events: %9ld", ptr_IBIS_events->numEvents);

  /*#################################################################*/
  /* Allocate memory buffers */
  /*#################################################################*/

    buffSize= ptr_IBIS_events->numEvents * sizeof(DAL3_Word);
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->isgriPha), buffSize, status,"ISGRI events PHA");
    buffSize= ptr_IBIS_events->numEvents * sizeof(DAL3_Byte);
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->riseTime), buffSize, status,"ISGRI events RT");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->isgriY),   buffSize, status,"ISGRI events Y");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->isgriZ),   buffSize, status,"ISGRI events Z");
    buffSize= ptr_IBIS_events->numEvents * sizeof(double);
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->IJD), buffSize, status,"ISGRI events Z");

    if (event_kind == COMPTON_SGLE || event_kind == COMPTON_MULE ) {
        status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->picsitPha),   buffSize, status,"PICsIT events PHA");
        status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->picsitY),   buffSize, status,"PICsIT events Y");
        status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->picsitZ),   buffSize, status,"PICsIT events Z");

        buffSize= ptr_IBIS_events->numEvents * sizeof(float);
        status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->picsit_energy),   buffSize, status,"PICsIT events ENERGY");
    }

    buffSize= ptr_IBIS_events->numEvents * sizeof(float);
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->isgri_energy),   buffSize, status,"ISGRI events ENERGY");
    buffSize= ptr_IBIS_events->numEvents * sizeof(DAL3_Byte);
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_IBIS_events->isgri_pi),   buffSize, status,"ISGRI events PI");
    

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot allocate buffers for input/output data");
      break;
    }

  /*#################################################################*/
  /* Read RAW data */
  /*#################################################################*/
    type=DAL_USHORT;
    status=DAL3IBISgetEvents(ISGRI_PHA, &type, (void *)ptr_IBIS_events->isgriPha, status);
    type=DAL_BYTE;
    status=DAL3IBISgetEvents(RISE_TIME, &type, (void *)ptr_IBIS_events->riseTime, status);
    type=DAL_BYTE;
    status=DAL3IBISgetEvents(ISGRI_Y,   &type, (void *)ptr_IBIS_events->isgriY,   status);
    type=DAL_BYTE;
    status=DAL3IBISgetEvents(ISGRI_Z,   &type, (void *)ptr_IBIS_events->isgriZ,   status);
            
    OBTime *event_OBT;
    status=DAL_GC_allocateDataBuffer((void **)&(event_OBT), ptr_IBIS_events->numEvents*sizeof(OBTime), status,"tmp OBT array");

    type=DAL3_OBT;
    status=DAL3IBISgetEvents(OB_TIME,   &type, (void *)event_OBT,   status);
    status=DAL3AUXconvertOBT2IJD(workGRP, TCOR_ANY, ptr_IBIS_events->numEvents, (OBTime*)event_OBT, (double*)ptr_IBIS_events->IJD, status);

    if (event_kind == COMPTON_SGLE || event_kind == COMPTON_MULE ) {
        type=DAL_BYTE;
        status=DAL3IBISgetEvents(PICSIT_PHA,&type, (void *)ptr_IBIS_events->picsitPha, status);
        type=DAL_BYTE;
        status=DAL3IBISgetEvents(PICSIT_Y,  &type, (void *)ptr_IBIS_events->picsitY,   status);
        type=DAL_BYTE;
        status=DAL3IBISgetEvents(PICSIT_Z,  &type, (void *)ptr_IBIS_events->picsitZ,   status);
    }


    if (gti) {
        TRY_BLOCK_BEGIN

            type=DAL3_OBT;
            TRY( DAL3IBISgetEventsBins(OB_TIME, &type, 1,1, &ptr_IBIS_events->obtStart, status), status,  "get OBT start");
            buffSize= ptr_IBIS_events->numEvents;
            TRY( DAL3IBISgetEventsBins(OB_TIME, &type, buffSize,buffSize, &ptr_IBIS_events->obtStop, status), status, "get OBT stop");

            double ijds[2];
            OBTime obts[2]={ptr_IBIS_events->obtStart,ptr_IBIS_events->obtStop};
            TRY( DAL3AUXconvertOBT2IJD(workGRP, TCOR_ANY, 2, (OBTime*)obts, (double*)ijds, status), status, "convert OBT 2 IJD");
              
            RILlogMessage(NULL, Log_1, "IJD: %.15lg - %.15lg; %.5lg s", ijds[0], ijds[1],(ijds[1]-ijds[0])*24*3600);

            ptr_IBIS_events->ijdStart=ijds[0];
            ptr_IBIS_events->ijdStop=ijds[1];

            if ((chatter > 2) && (gti))
                RILlogMessage(NULL, Log_0, "OBT range: %020lld , %020lld", ptr_IBIS_events->obtStart, ptr_IBIS_events->obtStop);

            if ((ptr_IBIS_events->obtStart < 0) || (ptr_IBIS_events->obtStop < 0)) {
                if (gti) {
                    RILlogMessage(NULL, Warning_1, "At least one OBT limit is negative.");
                    RILlogMessage(NULL, Warning_1, "Using all ScW to calculate mean bias and temperature.");
                }
                ptr_IBIS_events->obtStart=DAL3_NO_OBTIME;
                ptr_IBIS_events->obtStop=DAL3_NO_OBTIME;
            }

        TRY_BLOCK_END
    }
    else {
      ptr_IBIS_events->obtStart=DAL3_NO_OBTIME;
      ptr_IBIS_events->obtStop=DAL3_NO_OBTIME;
    }

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot get input data");
      break;
    }

  } while(0);
  if (selected > 0) status=DAL3IBIScloseEvents(status);
      
  if (status == ISDC_OK)
    RILlogMessage(NULL, Log_2, "ISGRI event information successfully extracted %i",status);

  return status;
}

int DAL3IBIS_populate_newest_DS(double ijdStart, double ijdStop, void * calibration_struct, char *DS, functype_open_DS func_open_DS, functype_read_DS func_read_DS, int chatter, int status) {
    char dol_start[DAL_MAX_STRING];
    char dol_stop[DAL_MAX_STRING];

    TRY_BLOCK_BEGIN
        RILlogMessage(NULL,Log_0,"Will search for %s for IJD %.15lg and %.15lg",DS,ijdStart,ijdStop);

        TRY( doICgetNewestDOL(DS,"",ijdStart,dol_start,status) ,I_ISGR_ERR_BAD_INPUT, "searching for %s",DS);
        TRY( doICgetNewestDOL(DS,"",ijdStop,dol_stop,status) ,I_ISGR_ERR_BAD_INPUT,  "searching for %s",DS);

        if (strcmp(dol_start,dol_stop)!=0) {
            RILlogMessage(NULL,Log_0,"different DOL for start and stop: %s and %s",dol_start,dol_stop);
            return -1;
        }

        RILlogMessage(NULL,Log_0,"Found %s as %s for IJD %.15lg and %.15lg",DS,dol_start,ijdStart,ijdStop);
        
        TRY( DAL3IBIS_populate_DS(dol_start, calibration_struct, DS, func_open_DS, func_read_DS, chatter, status), -1, "populating calibration from %s ", DS);

    TRY_BLOCK_END

    return status;
}

int DAL3IBIS_populate_DS(char *dol,  void * calibration_struct, char *DS, functype_open_DS func_open_DS, functype_read_DS func_read_DS, int chatter, int status) {
    RILlogMessage(NULL, Log_0, "");
    RILlogMessage(NULL, Log_0, "will load %s calibration from %s",DS,dol);
    dal_element *ptr_dal;
    status=(*func_open_DS)(dol,&ptr_dal,chatter,status);
    status=(*func_read_DS)(&ptr_dal,calibration_struct,chatter,status);
    return status;
}

int DAL3IBIS_populate_DS_flexible_IJD(char *dol, double ijdStart, double ijdStop,  void * calibration_struct, char *DS, functype_open_DS func_open_DS, functype_read_DS func_read_DS, int chatter, int status) {
    if (strcmp(dol,"auto")==0) {
        RILlogMessage(NULL, Log_0, "will search for bintable %s in IC",DS);
        return DAL3IBIS_populate_newest_DS(ijdStart, ijdStop, calibration_struct, DS, func_open_DS, func_read_DS, chatter, status);
    }       
    return DAL3IBIS_populate_DS(dol,  calibration_struct, DS, func_open_DS, func_read_DS, chatter, status);
}
    
int DAL3IBIS_populate_DS_flexible(char *dol, IBIS_events_struct *ptr_IBIS_events,  void * calibration_struct, char *DS, functype_open_DS func_open_DS, functype_read_DS func_read_DS, int chatter, int status) {
    return DAL3IBIS_populate_DS_flexible_IJD(dol, ptr_IBIS_events->ijdStart, ptr_IBIS_events->ijdStop, calibration_struct, DS, func_open_DS, func_read_DS, chatter, status);
}

    

int DAL3IBIS_open_LUT1(char *dol_LUT1, dal_element **ptr_ptr_dal_LUT1, int chatter,int status) {
    char keyVal[DAL_BIG_STRING];
    long numRow;
    int numCol;

    status=DAL_GC_objectOpen(dol_LUT1, ptr_ptr_dal_LUT1, status);
    status=DALelementGetName(*ptr_ptr_dal_LUT1, keyVal, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "%13s bintable cannot be opened. Status=%d",
                                  DS_ISGR_LUT1, status);
    }

    if (strcmp(keyVal, DS_ISGR_LUT1)) {
      RILlogMessage(NULL, Error_2, "File (%s) should be a %13s not %13s",
                                  dol_LUT1, DS_ISGR_LUT1, keyVal);
      status=I_ISGR_ERR_BAD_INPUT;
    }

    status=DALtableGetNumRows(*ptr_ptr_dal_LUT1, &numRow, status);
    status=DALtableGetNumCols(*ptr_ptr_dal_LUT1, &numCol, status);
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot get size of table %13s. Status=%d",
                                  DS_ISGR_LUT1, status);
    }

    if (numRow != ISGRI_N_PIXEL_Y*ISGRI_N_PIXEL_Z) {
      status=I_ISGR_ERR_ISGR_OFFS_BAD;
      RILlogMessage(NULL, Error_2, "Wrong number of rows (%ld) in %13s.",
                                  numRow, DS_ISGR_LUT1);
    }

    if (numCol != ISGRI_LUT1_N_COL) {
      status=I_ISGR_ERR_ISGR_OFFS_BAD;
      RILlogMessage(NULL, Error_2, "Wrong number of columns (%d) in %13s.",
                                  numCol, DS_ISGR_LUT1);
    }

    return status;
}

inline int yz_to_pixelNo(int y, int z) { // or z y?
    return  128*y+z;
}

inline int yz_to_mce(int y, int z) { // or z y?
    return 7 - z/32 - 4*(y/64); 
}


int DAL3IBIS_read_LUT1(dal_element **ptr_dal_LUT1, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_dataType type;
    int i_y, i_z;
    int i,j;
    double gh,oh,gt,ot;

    double **LUT1_tmp=NULL;
      
    status=allocate_2d((void ***)&LUT1_tmp,ISGRI_LUT1_N_COL,ISGRI_N_PIXEL_Y*ISGRI_N_PIXEL_Z,sizeof(double),status);
    if (status != ISDC_OK) 
        return status;

    if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI gain-offset LUT1 reading...");
    for (j=0; j<ISGRI_LUT1_N_COL-1; j++) {
      type=DAL_DOUBLE;
      status=DALtableGetCol(*ptr_dal_LUT1, NULL, j+1, &type, NULL,
                            (void *)(LUT1_tmp[j]), status);
    }

   //type=DAL_INT;
   //status=DALtableGetCol(*ptr_dal_LUT1, NULL, j+1, &type, NULL,
   //                      (void *)(LUT1_tmp[j]), status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot read LUT1 columns. Status=%d", status);
      return status;
    }
    
    for (i_y=0; i_y<ISGRI_N_PIXEL_Y; i_y++) for (i_z=0; i_z<ISGRI_N_PIXEL_Z; i_z++) {
      i=yz_to_pixelNo(i_y,i_z);

      gh = LUT1_tmp[0][i]/10. * 2.;
      oh = LUT1_tmp[1][i] * 2.;
      gt = LUT1_tmp[2][i]/100.* 30.;
      ot = (LUT1_tmp[3][i]+2) * 20.;

      ptr_ISGRI_energy_calibration->LUT1.pha_gain[i_y][i_z]=gh;
      ptr_ISGRI_energy_calibration->LUT1.pha_offset[i_y][i_z]=oh;
      ptr_ISGRI_energy_calibration->LUT1.rt_gain[i_y][i_z]=gt;
      ptr_ISGRI_energy_calibration->LUT1.rt_offset[i_y][i_z]=ot;
    }

    return status;
}


int DAL3IBIS_open_LUT2(char *dol_LUT2, dal_element **ptr_dal_LUT2, int chatter, int status) {
    char keyVal[DAL_MAX_STRING];
    int numRow, numCol, numAxes;
    long int dimAxes[2];  
    dal_dataType type;
      
    RILlogMessage(NULL, Log_0, "opening %s",dol_LUT2);

    status=DAL_GC_objectOpen(dol_LUT2, ptr_dal_LUT2, status);
    status=DALelementGetName(*ptr_dal_LUT2, keyVal, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "%13s image cannot be opened. Status=%d",
                                  DS_ISGR_LUT2, status);
      return status=I_ISGR_ERR_BAD_INPUT; // file not found!
    }
    if (strcmp(keyVal, DS_ISGR_LUT2)) {
      RILlogMessage(NULL, Error_2, "File (%s) should be a %13s not %s",
                                  dol_LUT2, DS_ISGR_LUT2, keyVal);
      return status=I_ISGR_ERR_BAD_INPUT;
    }

    return status;
}

int DAL3IBIS_open_LUT2_image(char *dol_LUT2, dal_element **ptr_dal_LUT2, int chatter, int status) {
    char keyVal[DAL_MAX_STRING];
    int numRow, numCol, numAxes;
    long int dimAxes[2];  
    dal_dataType type;

    status=DAL_GC_objectOpen(dol_LUT2, ptr_dal_LUT2, status);
    status=DALelementGetName(*ptr_dal_LUT2, keyVal, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "%13s image cannot be opened. Status=%d",
                                  DS_ISGR_LUT2, status);
      return status=I_ISGR_ERR_BAD_INPUT; // file not found!
    }
    if (strcmp(keyVal, DS_ISGR_LUT2)) {
      RILlogMessage(NULL, Error_2, "File (%s) should be a %13s not %s",
                                  dol_LUT2, DS_ISGR_LUT2, keyVal);
      return status=I_ISGR_ERR_BAD_INPUT;
    }

    status=DALarrayGetStruct(*ptr_dal_LUT2, &type, &numAxes, dimAxes, status);
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot get the 2 sizes of array %13s. Status=%d",
                                  DS_ISGR_LUT2, status);
      return status=I_ISGR_ERR_ISGR_RISE_BAD;
    }

    if (numAxes != ISGRI_DIM_LUT2) {
      RILlogMessage(NULL, Error_2, "%13s image must be a 2D array.", DS_ISGR_LUT2);
      return status=I_ISGR_ERR_ISGR_RISE_BAD;
    }

    if (  (dimAxes[0]!=ISGRI_LUT2_N_RT) || (dimAxes[1]!=ISGRI_LUT2_N_PHA)) {
      RILlogMessage(NULL, Error_2, "%13s array dimensions must be: %d*%d not %d*%d",
                                  DS_ISGR_LUT2, ISGRI_LUT2_N_RT, ISGRI_LUT2_N_PHA, dimAxes[0], dimAxes[1]);
      status=I_ISGR_ERR_ISGR_RISE_BAD;
      return status;
    }

    return status;
}

int DAL3IBIS_read_LUT2_image(dal_element **ptr_ptr_dal_LUT2, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_dataType type;
    long int my_numValues;
    long int my_startValues[2]={1,1},
        my_endValues[2]={ISGRI_LUT2_N_RT,ISGRI_LUT2_N_PHA};

    if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI rise-time LUT2 reading...");

    status=DAL_GC_allocateDataBuffer((void **)&ptr_ISGRI_energy_calibration->LUT2, ISGRI_LUT2_N_RT*ISGRI_LUT2_N_PHA*sizeof(double), status, "LUT2");
    if (status != ISDC_OK) 
        return status;

    type=DAL_DOUBLE;
    status=DALarrayGetSection(*ptr_ptr_dal_LUT2, ISGRI_DIM_LUT2, my_startValues,
                              my_endValues, &type, &my_numValues,
                              (void *)ptr_ISGRI_energy_calibration->LUT2, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot read LUT2 array. Status=%d", status);
      return status;
    }

    return status;
}

// should make flexible channels and interpolation
int DAL3IBIS_read_LUT2(dal_element **ptr_ptr_dal_LUT2, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_dataType type;
    int pha,rt;
    long int numRows;
    double energy[ISGRI_LUT2_N_PHA];
    int channel[ISGRI_LUT2_N_PHA];
    double corr[ISGRI_LUT2_N_PHA][ISGRI_LUT2_N_RT];

    if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI rise-time LUT2 reading...");

    TRY_BLOCK_BEGIN

     //   TRY( DAL_GC_allocateDataBuffer((void **)&ptr_ISGRI_energy_calibration->LUT2, ISGRI_LUT2_N_RT*ISGRI_LUT2_N_PHA*sizeof(double), status, "LUT2"), 0, "allocating LUT2");
    
        TRY( DALtableGetNumRows(*ptr_ptr_dal_LUT2,&numRows,status) , -1, "getting Num Rows for %s",DS_ISGR_LUT2);
        if (chatter > 3) RILlogMessage(NULL, Log_0, "%s rows %li...",DS_ISGR_LUT2,numRows);

        if (numRows!=ISGRI_LUT2_N_PHA) {
            RILlogMessage(NULL, Error_1, "LUT2 table has wrong length %i",numRows);
            return -1;
        }


        type=DAL_DOUBLE;
        TRY( DALtableGetCol(*ptr_ptr_dal_LUT2, NULL, 1, &type, NULL, (void *)(energy), status), -1, "reading LUT2 energy" );
        
        type=DAL_INT;
        TRY( DALtableGetCol(*ptr_ptr_dal_LUT2, NULL, 2, &type, NULL, (void *)(channel), status), -1, "reading LUT2 channel");
        
        type=DAL_DOUBLE;
        TRY( DALtableGetCol(*ptr_ptr_dal_LUT2, NULL, 3, &type, NULL, (void **)(corr), status), -1, "reading correction");


        for (pha=0;pha<ISGRI_LUT2_N_PHA;pha++) {
            for (rt=0;rt<ISGRI_LUT2_N_RT;rt++) {
                ptr_ISGRI_energy_calibration->LUT2[rt+pha*ISGRI_LUT2_N_RT]=corr[pha][rt]*energy[pha];
            }
        }

    TRY_BLOCK_END
    
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot read LUT2 array. Status=%d", status);
      return status;
    }

    return status;
}

////////////////////////

int DAL3IBIS_open_L2RE(char *dol_L2RE, dal_element **ptr_dal_L2RE, int chatter, int status) {

    char keyVal[DAL_MAX_STRING];
    int numRow, numCol, numAxes;
    long int dimAxes[2];  
    dal_dataType type;

    status=DAL_GC_objectOpen(dol_L2RE, ptr_dal_L2RE, status);
    status=DALelementGetName(*ptr_dal_L2RE, keyVal, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "%13s LUT2 rapid evolution cannot be opened. Status=%d",
                                  DS_ISGR_L2RE, status);
      return status=I_ISGR_ERR_BAD_INPUT; // file not found!
    }
    if (strcmp(keyVal, DS_ISGR_L2RE)) {
      RILlogMessage(NULL, Error_2, "File (%s) should be a %13s not %s",
                                  dol_L2RE, DS_ISGR_L2RE, keyVal);
      return status=I_ISGR_ERR_BAD_INPUT;
    }

    /*
    status=DALarrayGetStruct(*ptr_dal_LUT2, &type, &numAxes, dimAxes, status);
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "Cannot get the 2 sizes of array %13s. Status=%d",
                                  DS_ISGR_LUT2, status);
      return status=I_ISGR_ERR_ISGR_RISE_BAD;
    }

    if (numAxes != ISGRI_DIM_LUT2) {
      RILlogMessage(NULL, Error_2, "%13s image must be a 2D array.", DS_ISGR_LUT2);
      return status=I_ISGR_ERR_ISGR_RISE_BAD;
    }

    if (  (dimAxes[0]!=ISGRI_LUT2_N_RT) || (dimAxes[1]!=ISGRI_LUT2_N_PHA)) {
      RILlogMessage(NULL, Error_2, "%13s array dimensions must be: %d*%d not %d*%d",
                                  DS_ISGR_LUT2, ISGRI_LUT2_N_RT, ISGRI_LUT2_N_PHA, dimAxes[0], dimAxes[1]);
      status=I_ISGR_ERR_ISGR_RISE_BAD;
      return status;
    }*/

    return status;
}

int DAL3IBIS_read_L2RE(dal_element **ptr_ptr_dal_L2RE, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_dataType type;

    if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI L2RE reading...");

    DALtableGetNumRows(*ptr_ptr_dal_L2RE,&ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.n_entries,status);
    // check if too much
    
    if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI L2RE rows: %li",ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.n_entries);
    
    type=DAL_DOUBLE;
    status=DALtableGetCol(*ptr_ptr_dal_L2RE, NULL, 1, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.ijd), status);
    status=DALtableGetCol(*ptr_ptr_dal_L2RE, NULL, 2, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_offset), status);
    status=DALtableGetCol(*ptr_ptr_dal_L2RE, NULL, 3, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_gain), status);
    status=DALtableGetCol(*ptr_ptr_dal_L2RE, NULL, 4, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_gain2), status);
    status=DALtableGetCol(*ptr_ptr_dal_L2RE, NULL, 5, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.rt_offset), status);
    status=DALtableGetCol(*ptr_ptr_dal_L2RE, NULL, 6, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.rt_gain), status);
    
    double minijd=-1;
    double maxijd=-1;
    if (chatter > 3) {
        int i;
        for (i=0; i<ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.n_entries; i++) {
            double ijd=ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.ijd[i];
            if (minijd<0 || ijd < minijd) minijd=ijd;
            if (maxijd<0 || ijd > maxijd) maxijd=ijd;
        }
    }
    RILlogMessage(NULL, Log_0, "ISGRI L2RE IJD range: %.5lg - %.5lg",minijd,maxijd);

    return status;
}
    
////////////////////////////
//
// MCE correction open/read
//
////////////////////////////

int DAL3IBIS_open_MCEC(char *dol_MCEC, dal_element **ptr_dal_MCEC, int chatter, int status) { 

    char keyVal[DAL_MAX_STRING];
    int numRow, numCol, numAxes;
    long int dimAxes[2];  

    status=DAL_GC_objectOpen(dol_MCEC, ptr_dal_MCEC, status);
    status=DALelementGetName(*ptr_dal_MCEC, keyVal, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "%13s could not be opened as %s. Status=%d",
                                  DS_ISGR_MCEC, dol_MCEC, status);
      return status=I_ISGR_ERR_BAD_INPUT; // file not found!
    }
    if (strcmp(keyVal, DS_ISGR_MCEC)) {
      RILlogMessage(NULL, Error_2, "File (%s) should be a %13s not %s",
                                  dol_MCEC, DS_ISGR_MCEC, keyVal);
      return status=I_ISGR_ERR_BAD_INPUT;
    }

    return status;
}

int DAL3IBIS_read_MCEC(dal_element **ptr_ptr_dal_MCEC, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_dataType type;

    if (chatter > 3) RILlogMessage(NULL, Log_0, "reading ISGRI MCEC");

    long int numRows;
    DALtableGetNumRows(*ptr_ptr_dal_MCEC,&numRows,status);
    // check if too much
    
    if ( numRows != 8 ) {
        return -1;
    }
    
    type=DAL_DOUBLE;
    status=DALtableGetCol(*ptr_ptr_dal_MCEC, NULL, 2, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->MCE_correction.pha_offset), status);
    status=DALtableGetCol(*ptr_ptr_dal_MCEC, NULL, 3, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->MCE_correction.pha_gain), status);
    status=DALtableGetCol(*ptr_ptr_dal_MCEC, NULL, 4, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->MCE_correction.pha_gain2), status);
    status=DALtableGetCol(*ptr_ptr_dal_MCEC, NULL, 5, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->MCE_correction.rt_offset), status);
    status=DALtableGetCol(*ptr_ptr_dal_MCEC, NULL, 6, &type, NULL,
                            (void *)(ptr_ISGRI_energy_calibration->MCE_correction.rt_gain), status);

    int mce;
    char logstr_pha_gain2[DAL_BIG_STRING];
    char logstr_pha_gain[DAL_BIG_STRING];
    char logstr_pha_offset[DAL_BIG_STRING];
    char logstr_rt_gain[DAL_BIG_STRING];
    char logstr_rt_offset[DAL_BIG_STRING];
    
    sprintf(logstr_pha_gain2,  "MCE correction PHA gain-2 ");
    sprintf(logstr_pha_gain,  "                PHA gain   ");
    sprintf(logstr_pha_offset,"                PHA offset ");
    sprintf(logstr_rt_gain,   "                RT  gain   ");
    sprintf(logstr_rt_offset, "                RT  offset ");

    for (mce=0;mce<8;mce++) {
        sprintf(strchr(logstr_pha_gain2, '\0'),
                " %5.3lf",ptr_ISGRI_energy_calibration->MCE_correction.pha_gain2[mce]);

        sprintf(strchr(logstr_pha_gain, '\0'),
                " %5.3lf",ptr_ISGRI_energy_calibration->MCE_correction.pha_gain[mce]);

        sprintf(logstr_pha_offset+strlen(logstr_pha_offset),
                " %5.3lf",ptr_ISGRI_energy_calibration->MCE_correction.pha_offset[mce]);

        sprintf(logstr_rt_gain+strlen(logstr_rt_gain),
                " %5.3lf",ptr_ISGRI_energy_calibration->MCE_correction.rt_gain[mce]);
        
        sprintf(logstr_rt_offset+strlen(logstr_rt_offset),
                " %5.3lf",ptr_ISGRI_energy_calibration->MCE_correction.rt_offset[mce]);
        
    }


    if (chatter>3) {
        RILlogMessage(NULL,Log_0,"%s",logstr_pha_gain2);
        RILlogMessage(NULL,Log_0,"%s",logstr_pha_gain);
        RILlogMessage(NULL,Log_0,"%s",logstr_pha_offset);
        RILlogMessage(NULL,Log_0,"%s",logstr_rt_gain);
        RILlogMessage(NULL,Log_0,"%s",logstr_rt_offset);
    }

    
    return status;
}
    
////////////////////////

// c++ does not like this funny overrriding
int DAL3IBIS_populate_EFFC_flexible_IJD(char *dol, double ijdStart, double ijdStop,  void * calibration_struct, char *DS, int chatter, int status) {
    return DAL3IBIS_populate_DS_flexible_IJD(dol, ijdStart, ijdStop,  calibration_struct, DS, DAL3IBIS_open_EFFC, DAL3IBIS_read_EFFC, chatter, status);
}

int DAL3IBIS_open_EFFC(char *dol_EFFC, dal_element **ptr_dal_EFFC, int chatter, int status) { 

    char keyVal[DAL_MAX_STRING];
 //   int numRow, numCol, numAxes;
 //   long int dimAxes[2];  
 //   dal_dataType type;

    status=DAL_GC_objectOpen(dol_EFFC, ptr_dal_EFFC, status);
    status=DALelementGetName(*ptr_dal_EFFC, keyVal, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "%13s ISGRI efficiency cannot be opened. Status=%d",
                                  DS_ISGR_EFFC, status);
      return status=I_ISGR_ERR_BAD_INPUT; // file not found!
    }
    if (strcmp(keyVal, DS_ISGR_EFFC)) {
      RILlogMessage(NULL, Error_2, "File (%s) should be a %13s not %s",
                                  dol_EFFC, DS_ISGR_EFFC, keyVal);
      return status=I_ISGR_ERR_BAD_INPUT;
    }

    return status;
}


int DAL3IBIS_read_EFFC(dal_element **ptr_ptr_dal_EFFC, ISGRI_efficiency_struct *ptr_ISGRI_efficiency, int chatter, int status) {
    dal_dataType type;
    long int numRows;
    

    int pixel_grouping[MAX_PIXEL_GROUPS];
    int pixel_group[MAX_PIXEL_GROUPS];
    double efficiency[MAX_PIXEL_GROUPS][N_E_BAND];
    int i,j;

    TRY_BLOCK_BEGIN

        if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI EFFC reading...");

        TRY( DALtableGetNumRows(*ptr_ptr_dal_EFFC,&numRows,status) , -1, "getting Num Rows for EFFC");
        if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI EFFC rows %li...",numRows);

        if (numRows>MAX_PIXEL_GROUPS) {
            RILlogMessage(NULL, Error_1, "too many pixel groups %li",numRows);
            return -1;
        }
        
        type=DAL_INT;
        TRY( DALtableGetCol(*ptr_ptr_dal_EFFC, NULL, 1, &type, NULL, (void *)(pixel_grouping), status), -1, "reading pixel grouping" );
        
        type=DAL_INT;
        TRY( DALtableGetCol(*ptr_ptr_dal_EFFC, NULL, 2, &type, NULL, (void *)(pixel_group), status), -1, "reading pixel groups");
        
        type=DAL_DOUBLE;
        TRY( DALtableGetCol(*ptr_ptr_dal_EFFC, NULL, 3, &type, NULL, (void **)(efficiency), status), -1, "reading efficiency");

        for (i=0;i<numRows;i++) {
            double ***eff;
            int maxgroup=0;
            if ( pixel_grouping[i] == PIXEL_GROUPING_MCE ) {
                eff=(double ***)&(ptr_ISGRI_efficiency->MCE_efficiency);
                maxgroup=N_MCE;
            } else if ( pixel_grouping[i] == PIXEL_GROUPING_LT ) {
                eff=(double ***)&(ptr_ISGRI_efficiency->LT_efficiency);
                maxgroup=N_LT;
            } else {
                RILlogMessage(NULL, Error_1, "invalid grouping %i",pixel_grouping[i]);
                continue;
            }

            if ((pixel_group[i]>=maxgroup)) {
                RILlogMessage(NULL, Error_1, "invalid group %i for grouping %i max %i",pixel_group[i],pixel_grouping[i],maxgroup);
                continue;
            }

            if (pixel_group[i]<0) {
                RILlogMessage(NULL, Log_0, "groupping %i contains mapping of size %i:",pixel_grouping[i],-pixel_group[i]);

                if (pixel_grouping[i] == PIXEL_GROUPING_LT) { // so far only LT mapping allowed
                    //if (-pixel_group[i] > N_LT) 
                    //    RILlogMessage(NULL, Log_0, "too many pixel groups");
                    //    return -1; // !!
                    for (j=0;j<-pixel_group[i] && j<N_LT;j++) {
                        ptr_ISGRI_efficiency->LT_mapping[j]=efficiency[i][j];
                        RILlogMessage(NULL, Log_0, "efficiency group member mapping %i => %.5lg",j,efficiency[i][j]);
                    }
                } else {
                    RILlogMessage(NULL, Warning_1, "undefined grouping: %i",pixel_grouping[i]);
                }
                //else ignore
            } else {
                for (j=0;j<N_E_BAND;j++)
                    (*((double (*)[maxgroup][N_E_BAND])eff))[pixel_group[i]][j]=efficiency[i][j];
            }
        }

    TRY_BLOCK_END

    
    return status;
}

inline int get_LT_index(double LT, ISGRI_efficiency_struct *ptr_ISGRI_efficiency) {
    int i;
    if (LT>0) 
        for (i=0;i<N_LT;i++) {
            if ( fabs(ptr_ISGRI_efficiency->LT_mapping[i] - LT) < ptr_ISGRI_efficiency->LT_approximation ) { 
                return i;
            }
        }
    return -1;
}

int DAL3IBIS_get_ISGRI_efficiency(double energy, int y, int z, ISGRI_efficiency_struct *ptr_ISGRI_efficiency, double *ptr_efficiency, int chatter, int status) {
    int channel;
    int LT_index;
    int mce;

    double efficiency_1;
    double efficiency_2;

    LT_index=ptr_ISGRI_efficiency->LT_map_indexed[y][z];
    if (LT_index < 0) {
        *ptr_efficiency=0;
        return status;
    }

    
    channel=C256_get_channel(energy);
    
    mce=yz_to_mce(y,z);
        
    efficiency_1=ptr_ISGRI_efficiency->MCE_efficiency[mce][channel];
    efficiency_1*=ptr_ISGRI_efficiency->LT_efficiency[LT_index][channel];
    
    if (channel>=N_E_BAND-1) {
        *ptr_efficiency=efficiency_1;
        return status;
    }
    
    efficiency_2=ptr_ISGRI_efficiency->MCE_efficiency[mce][channel+1];
    efficiency_2*=ptr_ISGRI_efficiency->LT_efficiency[LT_index][channel+1];

    if (chatter>10) {
        RILlogMessage(NULL, Log_1, "Y: %i Z: %i LT: %.5lg LT index %i",y,z,ptr_ISGRI_efficiency->LT_map[y][z],LT_index);
        RILlogMessage(NULL, Log_1, "Energy %.5lg channel %i %.5lg - %.5lg eff 1,2 %.5lg %.5lg", energy,channel,C256_get_E_min(channel),C256_get_E_min(channel+1),efficiency_1,efficiency_2);
    }

    *ptr_efficiency=( energy - C256_get_E_min(channel) ) / ( C256_get_E_min(channel+1) - C256_get_E_min(channel) ) * ( efficiency_2 - efficiency_1 ) + efficiency_1;

    return status;
}

int DAL3IBIS_read_REV_context_maps(dal_element   *REVcontext,       // DOL to the REV context
        int           Revol,             // Revolution number of the SCW
        OBTime        OBTend,            // End Time of the SCW
        dal_double    **LowThreshMap,    // Output: Map of Low Thresholds (keV)
        dal_int       **ONpixelsREVmap,  // Output: Map of Pixels Status for this REV
        ISGRI_efficiency_struct *ptr_ISGRI_efficiency,
        unsigned char chatter)
{
    int	    
        status= ISDC_OK,
        RILstatus= ISDC_OK;
        
    int y,z;

    // ===================================================
    //      Retrieve and convert Low Thresholds map
    // ===================================================
        
    RILstatus= RILlogMessage(NULL,Log_0,"Getting Revolution LowThresholds in DAL3IBIS");

    // Allocation
    float *BufferF= NULL;
    if((BufferF= (float*)calloc(ISGRI_SIZE*ISGRI_SIZE, sizeof(float)))==NULL) {
        RILstatus= RILlogMessage(NULL,Error_1,"Error in allocating memory for LowThreshold buffer.");
        return ERR_ISGR_OSM_MEMORY_ALLOC;
    }

    // Use Dal3Ibis library: LT(keV) are set to 0 if LT(step) is dummy or 63 (noisy pixel)
    if((status= DAL3IBISGetlowthresholdKev(REVcontext,OBTend,BufferF,status))!=ISDC_OK) {
        RILstatus= RILlogMessage(NULL,Error_1,"Getting Revolution LowThresholds failed, status %d.",status);
        // SCREW 1746: if reading failed, set to average value (depends on Revol)
        double MeanLT= 18.2;
        if(Revol>55) MeanLT= 17.2;
        if(Revol>256) MeanLT= 15.3;
        RILstatus= RILlogMessage(NULL,Warning_1,"All Pixels LowThresholds set to %2.1fkeV.",MeanLT);
        for(y=0;y<ISGRI_SIZE;y++)
            for(z=0;z<ISGRI_SIZE;z++) 
                LowThreshMap[y][z]= MeanLT;
        RILstatus= RILlogMessage(NULL,Warning_1,"Reverting to ISDC_OK.");
        status= ISDC_OK;
    } else {
        // Rearrange buffer into matrix
        for(y=0;y<ISGRI_SIZE;y++) {
            for(z=0;z<ISGRI_SIZE;z++) {	    
                LowThreshMap[y][z]= BufferF[z*ISGRI_SIZE+y];
            }
        }
    }
        
    /// this is not too good at all

    int LT_count[N_LT] = {0};
    int N_good_total=0;

    for(y=0;y<ISGRI_SIZE;y++) {
        for(z=0;z<ISGRI_SIZE;z++) {	    
            ptr_ISGRI_efficiency->LT_map[y][z]=LowThreshMap[y][z];
            ptr_ISGRI_efficiency->LT_map_indexed[y][z]=get_LT_index(ptr_ISGRI_efficiency->LT_map[y][z], ptr_ISGRI_efficiency);
            if (chatter>9)
                RILstatus = RILlogMessage(NULL, Log_1,"y: %i z: %i LT %.5lg LT index %i",y,z,LowThreshMap[y][z],ptr_ISGRI_efficiency->LT_map_indexed[y][z]);

            if (ptr_ISGRI_efficiency->LT_map_indexed[y][z]>=0) {
                LT_count[ptr_ISGRI_efficiency->LT_map_indexed[y][z]]++;
                N_good_total++;
            }
        }
    }

    RILstatus = RILlogMessage(NULL, Log_1,"%i / %.5lg%% of pixels have usable LT",N_good_total,((float)N_good_total)/ISGRI_SIZE/ISGRI_SIZE*100.);
    int i;
    if (chatter>0) {
        for (i=0;i<N_LT;i++) {
            RILstatus = RILlogMessage(NULL, Log_1,"Index %3i LT %8.5lg N %5i %8.4lg%% or %8.4lg%% of usable",
                        i,
                        ptr_ISGRI_efficiency->LT_mapping[i],
                        LT_count[i],
                        ((float)(LT_count[i]))/ISGRI_SIZE/ISGRI_SIZE*100.,
                        ((float)(LT_count[i]))/N_good_total*100.
                        );
        }
    }

    if(BufferF) { free(BufferF); BufferF= NULL; }


    // ===================================================
    //     Retrieve Initial Pixel status on this REV
    // ===================================================

    // Retrieve map
    DAL3_Byte BufferB[ISGRI_SIZE][ISGRI_SIZE];
    if((status= DAL3IBISctxtGetImaPar(REVcontext, &OBTend, ISGRI_PIX_STA, BufferB, status)) !=ISDC_OK )
    {
        RILstatus= RILlogMessage(NULL,Warning_1,"Error finding Pixels Initial Status, error= %d.", 
                ERR_ISGR_OSM_FILE_NECESSARY_NOTFOUND);
        RILstatus= RILlogMessage(NULL,Warning_1,"All Pixels status initialized to ON.");
        for(y=0;y<ISGRI_SIZE;y++)
            for(z=0;z<ISGRI_SIZE;z++) 
                ONpixelsREVmap[y][z]= 1;
        RILstatus= RILlogMessage(NULL,Warning_1,"Reverting to ISDC_OK to continue.");
        status= ISDC_OK;
    } else {
        for(y=0;y<ISGRI_SIZE;y++)
            for(z=0;z<ISGRI_SIZE;z++) 
                ONpixelsREVmap[y][z]= BufferB[z][y];
    }

    if(chatter>3) {
        int 
            NumON=0,
            NumBadLT=0;
        for(y=0;y<ISGRI_SIZE;y++)
            for(z=0;z<ISGRI_SIZE;z++)
            {
                // Pixels ON at initial REV status
                if(ONpixelsREVmap[y][z])
                    NumON++;
                // Pixels that switched during the SCW
                if(LowThreshMap[y][z]<1e-10)
                    NumBadLT++;
            }
        RILstatus= RILlogMessage(NULL, Log_1, "REV stats: NumPixelsON= %d, NumBadLT= %d.", NumON,NumBadLT);
    }

    return status;
}



/// picsit
//
//


int DAL3IBIS_open_PICsIT_GO(char *dol,
        dal_element **picsEnerTabPtr,
        int chatter,
        int status)
{
    char keyVal[DAL_MAX_STRING];
    long numRow;
    int numCol;

    TRY_BLOCK_BEGIN
        TRY( DALobjectOpen(dol, picsEnerTabPtr, status), status, "open %s",dol);
        TRY( DALelementGetName(*picsEnerTabPtr, keyVal, status), status, "get element name");

        if (strcmp(keyVal, DS_PICS_GO)) {
            FAIL(I_COMP_SCA_ERR_BAD_INPUT,"File (%s) should be a %13s", dol, DS_PICS_GO);
        }

        TRY( DALtableGetNumRows(*picsEnerTabPtr, &numRow, status), I_COMP_SCA_ERR_PICS_ENER_BAD, "reading PICsIT GO table");
        TRY( DALtableGetNumCols(*picsEnerTabPtr, &numCol, status), I_COMP_SCA_ERR_PICS_ENER_BAD, "reading PICsIT GO table");

        if (numRow != PICSIT_N_PIX) {
            status=I_COMP_SCA_ERR_PICS_ENER_BAD;
            FAIL(I_COMP_SCA_ERR_PICS_ENER_BAD,"Wrong number of rows (%ld) in %13s.", numRow, DS_PICS_GO);
        }

        if (numCol != PICSIT_GO_N_COL) {
            FAIL(I_COMP_SCA_ERR_PICS_ENER_BAD,"Wrong number of columns (%ld) in %13s.", numCol, DS_PICS_GO);
        }

   TRY_BLOCK_END

   return status;
}



int DAL3IBIS_read_PICsIT_GO(dal_element **ptr_picsEnerTabPtr,
        PICsIT_energy_calibration_struct *ptr_PICsIT_energy_calibration,
        int chatter,
        int status)
{
    int   j,
          delta = PICSIT_GO_N_COL-PICSIT_GO_N_DATA;
    dal_dataType  type;

    TRY_BLOCK_BEGIN

        if (chatter > 3) {
            RILlogMessage(NULL, Log_0, "PICsIT energy correction LUT reading...");
            RILlogMessage(NULL, Log_0, "Global gain  : %g keV/channel", PICSIT_GAIN);
            RILlogMessage(NULL, Log_0, "Global offset: %g keV", PICSIT_OFFSET);
        }
        for (j=1; j<=PICSIT_GO_N_DATA; j++) {
            type=DAL_FLOAT;
            TRY( DALtableGetCol(*ptr_picsEnerTabPtr, NULL, j+delta, &type, NULL,
                    (void *)(ptr_PICsIT_energy_calibration->gain_offset[j-1]), status), status, "Cannot read PICsIT LUT columns." );
        }

    TRY_BLOCK_END

    return status;
}



int DAL3IBIS_print_all_events(dal_element *DAL_DS,int status) {
    TRY_BLOCK_BEGIN
        long ib_ev[5];

        TRY( DAL3IBISshowAllEvents(DAL_DS,ib_ev,status), status, "show all events");

        RILlogMessage(NULL, Log_0, "ISGRI Events            : %ld\n",ib_ev[ISGRI_EVTS]);
        RILlogMessage(NULL, Log_0, "PICsIT Single Events    : %ld\n",ib_ev[PICSIT_SGLE]);
        RILlogMessage(NULL, Log_0, "PICsIT Multiple Events  : %ld\n",ib_ev[PICSIT_MULE]);
        RILlogMessage(NULL, Log_0, "COMPTON Single Events   : %ld\n",ib_ev[COMPTON_SGLE]);
        RILlogMessage(NULL, Log_0, "COMPTON Multiple Events : %ld\n",ib_ev[COMPTON_MULE]);

    TRY_BLOCK_END

    return status;
}



/*double get_ISGRI_efficiency(double energy,
        double LT_setting,
        int MCE) {
    
}*/


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
#include "dal3aux.h"
#include "ril.h"
            
/****************************************************************************
                                                                          
***************************************************************************/

#define DS_ISGR_HK   "IBIS-DPE.-CNV"
#define KEY_DEF_BIAS      -120.0 
#define KEY_DEF_TEMP        -8.0    /* default when HK1 missing */
#define DS_ISGR_RAW       "ISGR-EVTS-ALL"
#define DS_ISGR_LUT1      "ISGR-OFFS-MOD"
#define DS_ISGR_LUT2      "ISGR-LUT2-MOD"
#define DS_ISGR_L2RE      "ISGR-L2RE-MOD"
#define DS_ISGR_MCEC      "ISGR-MCEC-MOD"
#define DS_PHG2           "ISGR-GAIN-MOD"
#define DS_PHO2           "ISGR-OFF2-MOD"
#define KEY_COL_OUT  "ISGRI_PI"

#define DS_ISGR_HK   "IBIS-DPE.-CNV"
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


// to memtools
//



typedef struct DAL_GC_allocation_struct {
    char comment[DAL_MAX_STRING];
    void *ptr;
    DAL_GC_RESOURCE_KIND resource_kind;
} DAL_GC_allocation_struct;


static struct DAL_GC_struct {
    int n_entries;
    DAL_GC_allocation_struct allocations[DAL_GC_MAX_ALLOCATIONS];
} DAL_GC;

void DAL_GC_register_allocation(void *ptr, DAL_GC_RESOURCE_KIND resource_kind, char *comment) {
    DAL_GC.allocations[DAL_GC.n_entries].ptr=ptr;
    DAL_GC.allocations[DAL_GC.n_entries].resource_kind=resource_kind;
    strncpy(DAL_GC.allocations[DAL_GC.n_entries].comment,comment,DAL_MAX_STRING);
    DAL_GC.n_entries++;
}

void DAL_GC_print() {
    int i;

    for (i=0;i<DAL_GC.n_entries;i++) {
        if (DAL_GC.allocations[i].resource_kind == DAL_GC_MEMORY_RESOURCE) {
            RILlogMessage(NULL,Log_0,"GC resource memory: %s",DAL_GC.allocations[i].comment);
        } else if (DAL_GC.allocations[i].resource_kind == DAL_GC_MEMORY_RESOURCE) {
            RILlogMessage(NULL,Log_0,"GC resource DAL object: %s",DAL_GC.allocations[i].comment);
        }
    }
}

void DAL_GC_free_all() {
    int i,status;

    for (i=DAL_GC.n_entries-1;i>=0;i--) {
        if (DAL_GC.allocations[i].resource_kind == DAL_GC_MEMORY_RESOURCE) {
            RILlogMessage(NULL,Log_0,"GC to free resource memory: %s",DAL_GC.allocations[i].comment);
            free(DAL_GC.allocations[i].ptr);
        } else if (DAL_GC.allocations[i].resource_kind == DAL_GC_DAL_OBJECT_RESOURCE) {
            RILlogMessage(NULL,Log_0,"GC to free resource DAL object: %s",DAL_GC.allocations[i].comment);
            status=DALobjectClose((dal_object)(DAL_GC.allocations[i].ptr), DAL_SAVE, ISDC_OK);
        } else {
        };
    }
    
}

int DAL_GC_allocateDataBuffer(void **buffer, 
                              long buffSize, 
                              int status,
                              char *comment)
{
    *buffer=NULL;
    status=DALallocateDataBuffer(buffer, 
                                 buffSize, 
                                 status);
    DAL_GC_register_allocation(*buffer, DAL_GC_MEMORY_RESOURCE, comment);
    return status;
}

int DAL_GC_freeDataBuffer(void *buffer,
                      int   status)
{
    int i;
    int found=0;

    status=DALfreeDataBuffer(buffer,status);

    for (i=0;i<DAL_GC.n_entries;i++) {
        if (DAL_GC.allocations[i].ptr == buffer) found=1;
        if ( (found==1) && (i<DAL_GC.n_entries-1) )
            DAL_GC.allocations[i]=DAL_GC.allocations[i+1];
    }
    if ( found==1 ) 
        DAL_GC.n_entries--;

    return status;
}


int DAL_GC_objectOpen(const char   *DOL,    /* I DOL of object to open          */
        dal_object   *object, /* O DAL element pointer            */
        int           status) {
    status=DALobjectOpen(DOL,object,status);
    DAL_GC_register_allocation((void*)object, DAL_GC_DAL_OBJECT_RESOURCE,(char *)DOL);

    return status;
}



/// this is not right!
int doICgetNewestDOL(char * category,char * filter, double valid_time, char * DOL,int status) {
    char ic_group[DAL_MAX_STRING];
    snprintf(ic_group,DAL_MAX_STRING,"%s/idx/ic/ic_master_file.fits[1]",getenv("CURRENT_IC"));
    status=ICgetNewestDOL(ic_group,
            "OSA",
            category,filter,valid_time,DOL,status);
    return status;
}

// dealocate and close

static const double DtempH1[8] = {0.43, -0.39, -0.77, 0.84, -0.78, 1.09, -0.08, -0.31};
double slopeMCE[8]={-1.8,-2.0,-2.3,-2.7,-0.5,-2.4,-0.8,-0.5} ;


// implements temperature and bias dependency additional to the existing LUT1
// (note that bias is forced to be 1.2, see DAL3IBIS_MceIsgriHkCal)
// also this slightly interferes with the revolution-scale MCE correction
int DAL3IBIS_correct_LUT1_for_temperature_bias(
        dal_element *workGRP,
        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,
        ISGRI_events_struct *ptr_ISGRI_events,
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
    status=DAL3IBIS_MceIsgriHkCal(workGRP,ptr_ISGRI_events->obtStart,ptr_ISGRI_events->obtEnd,meanTemp,meanBias,chatter,status);

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
            mce     = yz_to_mce(i_y,i_y);

            ptr_ISGRI_energy_calibration->LUT1.pha_gain[i_y][i_z]*pow(meanTemp[mce],-1.11)*pow(meanBias[mce],-0.0832);
            ptr_ISGRI_energy_calibration->LUT1.pha_offset[i_y][i_z]*pow(meanTemp[mce],slopeMCE[mce])*pow(meanBias[mce],0.0288);
            ptr_ISGRI_energy_calibration->LUT1.rt_gain[i_y][i_z]*pow(meanTemp[mce],0.518)*pow(meanBias[mce],0.583);
            ptr_ISGRI_energy_calibration->LUT1.rt_offset[i_y][i_z]*pow(meanTemp[mce],0.625)*pow(meanBias[mce],0.530);
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
        meanBias[j]=-meanBias[j]/100. ;
        meanBias[j]=1.2;  // because who cares about the bias
    }
        
    return status;
}


inline int DAL3IBIS_reconstruct_ISGRI_energy(
        long isgriPha,
        short riseTime,
        short isgriY,
        short isgriZ,
        
        float *ptr_isgri_energy,
        DAL3_Byte *ptr_isgri_pi,

        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,

        infoEvt_struct *ptr_infoEvt,
        int status
        ) {

    double rt;
    double pha;

    short irt;
    int ipha;
    int ipha2;

    pha+=DAL3GENrandomDoubleX1();
    riseTime+=DAL3GENrandomDoubleX1();

    // 256 channels for LUT2 calibration scaled 
    rt = 2.*riseTime/2.4+5.0;  

    if ((isgriY<0) | (isgriY>=128) |(isgriZ<0) |(isgriZ>=128)) { // 127?..
         ptr_infoEvt->bad_pixel_yz++;
         *ptr_isgri_energy=0;
         *ptr_isgri_pi=irt;
         return;
    };

    rt = rt * ptr_ISGRI_energy_calibration->LUT1.rt_gain[isgriY][isgriZ] + ptr_ISGRI_energy_calibration->LUT1.rt_offset[isgriY][isgriZ];
    pha = isgriPha * ptr_ISGRI_energy_calibration->LUT1.pha_gain[isgriY][isgriZ] + ptr_ISGRI_energy_calibration->LUT1.pha_offset[isgriY][isgriZ];

    // LUT2 rapid here TODO

    /// compression to LUT2 index
    irt = round(rt); 
    
    if (irt < 0)        {irt=0;   ptr_infoEvt->rt_too_low++;}
    else if (irt >= ISGRI_LUT2_N_RT) {irt=ISGRI_LUT2_N_RT-1; ptr_infoEvt->rt_too_high++;}

    ipha = floor(pha/2);

    if (ipha >=ISGRI_LUT2_N_PHA-1) {ipha=ISGRI_LUT2_N_PHA-2; ptr_infoEvt->pha_too_high++;}
    else if (ipha<0) {ipha=0; ptr_infoEvt->pha_too_low++;}

    // only pha interpolation, as before
    *ptr_isgri_energy=ptr_ISGRI_energy_calibration->LUT2[irt+ipha*ISGRI_LUT2_N_RT]; \
            +(ptr_ISGRI_energy_calibration->LUT2[irt+ipha*ISGRI_LUT2_N_RT+1]-ptr_ISGRI_energy_calibration->LUT2[irt+ipha*ISGRI_LUT2_N_RT])*(pha/2.-(double)ipha);

    // invalid LUT2 values
    if (ptr_isgri_energy <= 0) {
        ptr_infoEvt->negative_energy++;
        *ptr_isgri_energy=0;
    };

    *ptr_isgri_pi=irt;
    ptr_infoEvt->good++;
}

int DAL3IBIS_reconstruct_ISGRI_energies(
        ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration,
        ISGRI_events_struct *ptr_ISGRI_events,
        int chatter,
        int status
        ) {

    long i;

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
        RILlogMessage(NULL, Log_0, "           good: %li",ptr_ISGRI_events->infoEvt.good);
        RILlogMessage(NULL, Log_0, "   bad pixel YZ: %li",ptr_ISGRI_events->infoEvt.bad_pixel_yz);
        RILlogMessage(NULL, Log_0, "     RT too low: %li",ptr_ISGRI_events->infoEvt.rt_too_low);
        RILlogMessage(NULL, Log_0, "    RT too high: %li",ptr_ISGRI_events->infoEvt.rt_too_high);
        RILlogMessage(NULL, Log_0, "    PHA too low: %li",ptr_ISGRI_events->infoEvt.pha_too_low);
        RILlogMessage(NULL, Log_0, "   PHA too high: %li",ptr_ISGRI_events->infoEvt.pha_too_high);
        RILlogMessage(NULL, Log_0, "negative energy: %li",ptr_ISGRI_events->infoEvt.negative_energy);
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

    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.ijd), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution IJD");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_gain), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution PHA gain");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_gain2), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution PHA gain2");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.pha_offset), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution PHA offset");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.rt_gain), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution RT gain");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.rt_offset), ISGRI_REVO_N*sizeof(double), status,"LUT2 evolution RT offset");

    return status;
}



int DAL3IBIS_read_ISGRI_events(dal_element *workGRP,
                               ISGRI_events_struct *ptr_ISGRI_events,
                               int gti,
                               int chatter,
                               int status)
{
    ptr_ISGRI_events->obtStart=DAL3_NO_OBTIME;
    ptr_ISGRI_events->obtEnd=DAL3_NO_OBTIME;
    ptr_ISGRI_events->numEvents=0;
    ptr_ISGRI_events->infoEvt.good=0;
    ptr_ISGRI_events->infoEvt.bad_pixel_yz=0;
    ptr_ISGRI_events->infoEvt.rt_too_low=0;
    ptr_ISGRI_events->infoEvt.rt_too_high=0;
    ptr_ISGRI_events->infoEvt.pha_too_low=0;
    ptr_ISGRI_events->infoEvt.pha_too_high=0;
    ptr_ISGRI_events->infoEvt.negative_energy=0;


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
    
    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_1, "selecting events: %i", status);
      return status;
    }

    double ijds[2];
    OBTime obts[2]={ptr_ISGRI_events->obtStart,ptr_ISGRI_events->obtEnd};
    status=DAL3AUXconvertOBT2IJD(workGRP, TCOR_ANY, 2, (OBTime*)obts, (double*)ijds, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_1, "error converting to IJD: %i", status);
      return status;
    }
      
    RILlogMessage(NULL, Log_1, "IJD: %.5lg - %.5lg", ijds[0], ijds[1], status);

    ptr_ISGRI_events->ijdStart=ijds[0];
    ptr_ISGRI_events->ijdEnd=ijds[1];

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
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_events->isgriPha), buffSize, status,"ISGRI events PHA");
    buffSize= ptr_ISGRI_events->numEvents * sizeof(DAL3_Byte);
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_events->riseTime), buffSize, status,"ISGRI events RT");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_events->isgriY),   buffSize, status,"ISGRI events Y");
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_events->isgriZ),   buffSize, status,"ISGRI events Z");
    
    buffSize= ptr_ISGRI_events->numEvents * sizeof(float);
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_events->isgri_energy),   buffSize, status,"ISGRI events ENERGY");
    buffSize= ptr_ISGRI_events->numEvents * sizeof(DAL3_Byte);
    status=DAL_GC_allocateDataBuffer((void **)&(ptr_ISGRI_events->isgri_pi),   buffSize, status,"ISGRI events PI");

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
      
  if (status == ISDC_OK)
    RILlogMessage(NULL, Log_2, "ISGRI event information successfully extracted %i",status);

  ptr_ISGRI_events->infoEvt.good=0;
  ptr_ISGRI_events->infoEvt.bad_pixel_yz=0;
  ptr_ISGRI_events->infoEvt.rt_too_low=0;
  ptr_ISGRI_events->infoEvt.rt_too_high=0;
  ptr_ISGRI_events->infoEvt.pha_too_low=0;
  ptr_ISGRI_events->infoEvt.pha_too_high=0;
  ptr_ISGRI_events->infoEvt.negative_energy=0;

  return status;
}
    
int DAL3IBIS_populate_newest_LUT1(ISGRI_events_struct *ptr_ISGRI_events, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    char dol_LUT1[DAL_MAX_STRING];

    status=doICgetNewestDOL("ISGR-OFFS-MOD","",ptr_ISGRI_events->ijdStart,dol_LUT1,status);
    RILlogMessage(NULL,Log_0,"Found ISGR-OFFS-MOD as %s",dol_LUT1);

    status=DAL3IBIS_populate_LUT1(dol_LUT1, ptr_ISGRI_energy_calibration, chatter, status);
    return status;
}

int DAL3IBIS_populate_LUT1(char *dol_LUT1, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_element *ptr_dal_LUT1;
    status=DAL3IBIS_open_LUT1(dol_LUT1,&ptr_dal_LUT1,chatter,status);
    status=DAL3IBIS_read_LUT1(&ptr_dal_LUT1,ptr_ISGRI_energy_calibration,chatter,status);
    return status;
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

int DAL3IBIS_populate_newest_LUT2(ISGRI_events_struct *ptr_ISGRI_events, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    char dol_LUT2[DAL_MAX_STRING];

    status=doICgetNewestDOL("ISGR-LUT2-MOD","",ptr_ISGRI_events->ijdStart,dol_LUT2,status);
    RILlogMessage(NULL,Log_0,"Found ISGR-LUT2-MOD as %s",dol_LUT2);

    status=DAL3IBIS_populate_LUT2(dol_LUT2, ptr_ISGRI_energy_calibration, chatter, status);
    return status;
}

int DAL3IBIS_populate_LUT2(char *dol_LUT2, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_element *ptr_dal_LUT2;
    status=DAL3IBIS_open_LUT2(dol_LUT2,&ptr_dal_LUT2,chatter,status);
    status=DAL3IBIS_read_LUT2(&ptr_dal_LUT2,ptr_ISGRI_energy_calibration,chatter,status);
    return status;
}

int DAL3IBIS_open_LUT2(char *dol_LUT2, dal_element **ptr_dal_LUT2, int chatter, int status) {
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

int DAL3IBIS_read_LUT2(dal_element **ptr_ptr_dal_LUT2, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
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

////////////////////////

int DAL3IBIS_populate_newest_LUT2_rapid_evolution(ISGRI_events_struct *ptr_ISGRI_events, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    char dol_L2RE[DAL_MAX_STRING];
    
    if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI rise-time LUT2 rapid evolution reading...");

    status=doICgetNewestDOL("ISGR-L2RE-MOD","",ptr_ISGRI_events->ijdStart,dol_L2RE,status);
    
    if (status != ISDC_OK) {
        RILlogMessage(NULL,Error_1,"did not find ISGR-L2RE-MOD");
        return status;
    }
    RILlogMessage(NULL,Log_0,"Found ISGR-L2RE-MOD as %s",dol_L2RE);

    status=DAL3IBIS_populate_L2RE(dol_L2RE, ptr_ISGRI_energy_calibration, chatter, status);

    return status;
}

int DAL3IBIS_populate_L2RE(char *dol_L2RE, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_element *ptr_dal_L2RE;
    status=DAL3IBIS_open_L2RE(dol_L2RE,&ptr_dal_L2RE,chatter,status);
    status=DAL3IBIS_read_L2RE(&ptr_dal_L2RE,ptr_ISGRI_energy_calibration,chatter,status);
    return status;
}

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
    
    if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI L2RE rows: %i",ptr_ISGRI_energy_calibration->LUT2_rapid_evolution.n_entries);
    
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
    
////////////////////////

int DAL3IBIS_populate_newest_MCEC(ISGRI_events_struct *ptr_ISGRI_events, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    char dol_MCEC[DAL_MAX_STRING];
    
    if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI rise-time MCE evolution reading...");

    status=doICgetNewestDOL("ISGR-MCEC-MOD","",ptr_ISGRI_events->ijdStart,dol_MCEC,status);
    
    if (status != ISDC_OK) {
        RILlogMessage(NULL,Error_1,"did not find ISGR-MCEC-MOD");
        return status;
    }
    RILlogMessage(NULL,Log_0,"Found ISGR-MCEC-MOD as %s",dol_MCEC);

    status=DAL3IBIS_populate_MCEC(dol_MCEC, ptr_ISGRI_energy_calibration, chatter, status);

    return status;
}

int DAL3IBIS_populate_MCEC(char *dol_MCEC, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_element *ptr_dal_MCEC;
    status=DAL3IBIS_open_MCEC(dol_MCEC,&ptr_dal_MCEC,chatter,status);
    status=DAL3IBIS_read_MCEC(&ptr_dal_MCEC,ptr_ISGRI_energy_calibration,chatter,status);
    return status;
}

int DAL3IBIS_open_MCEC(char *dol_MCEC, dal_element **ptr_dal_MCEC, int chatter, int status) { // these are identical!!!

    char keyVal[DAL_MAX_STRING];
    int numRow, numCol, numAxes;
    long int dimAxes[2];  
    dal_dataType type;

    status=DAL_GC_objectOpen(dol_MCEC, ptr_dal_MCEC, status);
    status=DALelementGetName(*ptr_dal_MCEC, keyVal, status);

    if (status != ISDC_OK) {
      RILlogMessage(NULL, Error_2, "%13s LUT2 rapid evolution cannot be opened. Status=%d",
                                  DS_ISGR_MCEC, status);
      return status=I_ISGR_ERR_BAD_INPUT; // file not found!
    }
    if (strcmp(keyVal, DS_ISGR_MCEC)) {
      RILlogMessage(NULL, Error_2, "File (%s) should be a %13s not %s",
                                  dol_MCEC, DS_ISGR_MCEC, keyVal);
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

int DAL3IBIS_read_MCEC(dal_element **ptr_ptr_dal_MCEC, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status) {
    dal_dataType type;

    if (chatter > 3) RILlogMessage(NULL, Log_0, "ISGRI MCEC reading...");

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
    
    return status;
}
    

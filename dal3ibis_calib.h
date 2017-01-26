/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS CALIBRATION  LIBRARY                 */
/*                                                                           */
/*       the intend of this part of the DAL3IBIS library                     */
/*       is to provide a transformed, calibrated view of the ISGRI data      */
/*                                                                           */
/*****************************************************************************/


#define N_MCE 8
#define N_LT 16
#define ISGRI_N_PIXEL_Y 128
#define ISGRI_N_PIXEL_Z 128


#define ISGRI_LUT2_N_RT 256
#define ISGRI_LUT2_N_PHA 2048 // 256?
#define ISGRI_DIM_LUT2 2

#define ISGRI_REVO_N 128

#define ISGRI_LUT1_N_COL 5

#define MAX_PIXEL_GROUPS 32
#define PIXEL_GROUPING_MCE 0
#define PIXEL_GROUPING_LT 1

// keep band treatement separate
#define N_E_BAND 256 

double E_band_min[N_E_BAND];
double E_band_max[N_E_BAND];

#define E_BAND_N_REVERSE 2048
#define E_BAND_REVERSE_STEP 0.5

int E_band_reverse[E_BAND_N_REVERSE];

inline double get_E_min(int ch);
inline double get_E_max(int ch);
inline int get_channel(double energy);



// ISGRI detector energy transformation structure
typedef struct ISGRI_energy_calibration_struct {

    // this read from table and update from temperature and bias
    struct MCE_correction_struct { 
        double pha_offset[N_MCE];
        double pha_gain[N_MCE];
        double pha_gain2[N_MCE];
        double rt_offset[N_MCE];
        double rt_gain[N_MCE];
        double rt_pha_cross_gain[N_MCE];
    } MCE_correction;

    // constant per revolution or always
    struct LUT1_struct {
        double ** pha_gain;
        double ** pha_offset;
        double ** rt_gain;
        double ** rt_offset;
        int ** pixtype;
    } LUT1;

    double * LUT2; // i_rt + i_pha*ISGRI_LUT2_N_RT

    // in principle per pointing, but interpolated
    struct LUT2_rapid_evolution_struct { 
        long int n_entries;
        double * ijd;
        double * pha_gain; 
        double * pha_gain2; 
        double * pha_offset;
        double * rt_gain;
        double * rt_offset;
        double * rt_pha_cross_gain;
    } LUT2_rapid_evolution;

} ISGRI_energy_calibration_struct;

// structure describing efficiency of ISGRI components
typedef struct {

    double MCE_efficiency[N_MCE][N_E_BAND]; // energy-dependent
    double LT_efficiency[N_LT][N_E_BAND]; // energy-dependent, per LT class
    //double pixel_efficiency[ISGRI_N_PIXEL_Y][ISGRI_N_PIXEL_Z]; // grey, also used to kill pixels

} ISGRI_efficiency_struct;

typedef struct infoEvt_struct {
    long good;
    long bad_pixel_yz;
    long rt_too_low;
    long rt_too_high;
    long pha_too_low;
    long pha_too_high;
    long negative_energy;
} infoEvt_struct;

typedef struct ISGRI_events_struct {
    long       numEvents;

    OBTime     obtStart;
    OBTime     obtEnd;

    double     ijdStart;
    double     ijdEnd;

    DAL3_Word *isgriPha;
    DAL3_Byte *riseTime;
    DAL3_Byte *isgriY;
    DAL3_Byte *isgriZ;

    float *isgri_energy;
    DAL3_Byte  *isgri_pi;

    infoEvt_struct infoEvt;
} ISGRI_events_struct;

int DAL3IBISreadMCEcorrection( dal_element *MCECorrectionStructure, 
				ISGRI_energy_calibration_struct *ISGRI_energy_calibration, 
				int          status );

int DAL3IBISreadLUT1( dal_element *LUT1Structure, 
				ISGRI_energy_calibration_struct *ISGRI_energy_calibration, 
				int          status );

int DAL3IBISreadLUT2( dal_element *LUT2Structure, 
				ISGRI_energy_calibration_struct *ISGRI_energy_calibration, 
				int          status );


int DAL3IBISreadMCEEfficiency( dal_element *MCEEfficiencyStructure, 
				ISGRI_efficiency_struct *ISGRI_efficiency, 
				int          status );

int DAL3IBISreadLTEfficiency( dal_element *LTEfficiencyStructure, 
				ISGRI_efficiency_struct *ISGRI_efficiency, 
				int          status );

int DAL3IBISreadPixelEfficiency( dal_element *PixelEfficiencyStructure, 
				ISGRI_efficiency_struct *ISGRI_efficiency, 
				int          status );

// compute
int DAL3IBISGetISGRIEfficiency(
                ISGRI_efficiency_struct *ISGRI_efficiency,
				int          status);

// auxiliary calls to read HK

int DAL3IBIS_MceIsgriHkCal(dal_element *workGRP,
        OBTime       obtStart,
        OBTime       obtEnd,
        double       meanT[8],
        double       meanBias[8],
        int          chatter,
        int          status);

int print_error(int status);

#define TRY_BLOCK_BEGIN  do { 
#define TRY_BLOCK_END } while(0);
#define TRY(call,fail_status,...) if ( (status=call) != ISDC_OK ) { RILlogMessage(NULL,Error_1,"error in the call: %i, call forwards %i",status,fail_status); print_error(status); print_error(fail_status); RILlogMessage(NULL,Error_1,##__VA_ARGS__); break;}



#ifndef __CINT__
#ifdef __cplusplus
}
#endif
#endif

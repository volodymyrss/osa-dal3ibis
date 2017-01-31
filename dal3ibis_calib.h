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

#include "isdc.h"

#define I_ISGR_RANGE                          (I_ERROR_CODE_START-20000)
#define I_ISGR_SCW_ERR                        (I_ISGR_RANGE-1000)
#define I_ISGR_OSM_ERR                        (I_ISGR_SCW_ERR-200)
#define ERR_ISGR_OSM_UNUSED                   (I_ISGR_OSM_ERR-1)    // Unused error                               
#define ERR_ISGR_OSM_TABLE_EMPTY              (I_ISGR_OSM_ERR-2)    // Data table is empty                        
#define ERR_ISGR_OSM_NROWS_NVALUES            (I_ISGR_OSM_ERR-3)    // Number of rows != number of values         
#define ERR_ISGR_OSM_DEREFERENCE              (I_ISGR_OSM_ERR-4)    // Dereferencing error                        
#define ERR_ISGR_OSM_FILE_NOTFOUND            (I_ISGR_OSM_ERR-5)    // Data i/o file not found => continue        
#define ERR_ISGR_OSM_FILE_NECESSARY_NOTFOUND  (I_ISGR_OSM_ERR-6)    // Data necessary i/o file not found => abort 
#define ERR_ISGR_OSM_OUTPUT_FILE_CREATION     (I_ISGR_OSM_ERR-7)    // Output file creation error => abort        
#define ERR_ISGR_OSM_OUTPUT_INDEX_CREATION    (I_ISGR_OSM_ERR-8)    // Index file creation error => abort         
#define ERR_ISGR_OSM_MEMORY_ALLOC             (I_ISGR_OSM_ERR-9)    // Memmory allocation error                   
#define ERR_ISGR_OSM_SHD_INDX                 (I_ISGR_OSM_ERR-10)   // Shadowgram index does not exist            
#define ERR_ISGR_OSM_EFFI_SHD_INDX            (I_ISGR_OSM_ERR-11)   // Shadowgram efficiency index does not exist 
#define ERR_ISGR_OSM_DSP_INDX                 (I_ISGR_OSM_ERR-12)   // Spectral index does not exist              
#define ERR_ISGR_OSM_LCR_INDX                 (I_ISGR_OSM_ERR-13)   // Light curve index does not exist           
#define ERR_ISGR_OSM_WRITE_STATITICS          (I_ISGR_OSM_ERR-14)   // Impossible to write statistics             
#define ERR_ISGR_OSM_WRITE_SHADOWGRAM         (I_ISGR_OSM_ERR-15)   // Impossible to write image                  
#define ERR_ISGR_OSM_WRITE_EFFICIENCY_SHD     (I_ISGR_OSM_ERR-16)   // Impossible to write image efficiency       
#define ERR_ISGR_OSM_WRITE_SPECTRA            (I_ISGR_OSM_ERR-17)   // Impossible to write spectra                
#define ERR_ISGR_OSM_WRITE_EFFICIENCY_DSP     (I_ISGR_OSM_ERR-18)   // Impossible to write spectra efficiency     
#define ERR_ISGR_OSM_WRITE_LIGHTCURVES        (I_ISGR_OSM_ERR-19)   // Impossible to write lightcurve             
#define ERR_ISGR_OSM_WRITE_EFFICIENCY_LCR     (I_ISGR_OSM_ERR-20)   // Impossible to write lightcurve efficiency  
#define ERR_ISGR_OSM_DATA_INCONSISTENCY       (I_ISGR_OSM_ERR-21)   // Some data inconsistency                    



#define TRY_BLOCK_BEGIN  do { 
#define TRY_BLOCK_END } while(0);
#define TRY(call,fail_status,...) if ( (status=call) != ISDC_OK ) { char message[DAL_MAX_STRING]; sprintf(message,##__VA_ARGS__); report_try_error(status,fail_status,message,__FILE__,__LINE__); break;}
#define FAIL(status,...) { char message[DAL_MAX_STRING]; sprintf(message,##__VA_ARGS__); report_try_error(status,status,message,__FILE__,__LINE__); break;}



#define DS_ISGR_LUT1      "ISGR-OFFS-MOD"
#define DS_ISGR_LUT2      "ISGR-RISE-MOD"
#define DS_ISGR_L2RE      "ISGR-L2RE-MOD"
#define DS_ISGR_MCEC      "ISGR-MCEC-MOD"
#define DS_ISGR_EFFC      "ISGR-EFFC-MOD"
#define DS_PICS_GO   "PICS-ENER-MOD"


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

static double E_band_min[N_E_BAND];
static double E_band_max[N_E_BAND];

#define E_BAND_N_REVERSE 2048
#define E_BAND_REVERSE_STEP 0.5

static int E_band_reverse[E_BAND_N_REVERSE];

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

#define PICSIT_N_PIX    4096l
#define PICSIT_GO_N_COL    4
#define PICSIT_GO_N_DATA   2 /* only 2 last columns used */
#define PICSIT_GAIN      7.1
#define PICSIT_OFFSET  -22.0 /* in keV */



typedef struct PICsIT_energy_calibration_struct {
    float *gain_offset[PICSIT_GO_N_DATA];;
} PICsIT_energy_calibration_struct;

typedef struct ISGRI_efficiency_struct {

    double MCE_efficiency[N_MCE][N_E_BAND]; // energy-dependent
    double LT_efficiency[N_LT][N_E_BAND]; // energy-dependent, per LT class

    double LT_mapping[N_LT];

    double LT_map[ISGRI_N_PIXEL_Y][ISGRI_N_PIXEL_Z];
    int LT_map_indexed[ISGRI_N_PIXEL_Y][ISGRI_N_PIXEL_Z];

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

typedef struct IBIS_events_struct {
    long       numEvents;

    OBTime     obtStart;
    OBTime     obtStop;

    double     ijdStart;
    double     ijdStop;

    DAL3_Word *isgriPha;
    DAL3_Byte *riseTime;
    DAL3_Byte *isgriY;
    DAL3_Byte *isgriZ;
    
    DAL3_Byte *picsitPha;
    DAL3_Byte *picsitY;
    DAL3_Byte *picsitZ;

    float *isgri_energy;
    DAL3_Byte  *isgri_pi;
    
    float *picsit_energy;

    infoEvt_struct infoEvt;

    IBIS_type event_kind;
} IBIS_events_struct;


// auxiliary calls to read HK

int DAL3IBIS_MceIsgriHkCal(dal_element *workGRP,
        OBTime       obtStart,
        OBTime       obtStop,
        double       meanT[8],
        double       meanBias[8],
        int          chatter,
        int          status);

int print_error(int status);

int explain_error(int status,  char *error);

typedef int (*functype_open_DS)(char *, dal_element **, int , int);
typedef int (*functype_read_DS)(dal_element **, void *, int , int);


int DAL3IBIS_open_LUT1(char *dol_LUT1, dal_element **ptr_ptr_dal_LUT1, int chatter,int status);
int DAL3IBIS_open_LUT2(char *dol_LUT2, dal_element **ptr_dal_LUT2, int chatter, int status);
//int DAL3IBIS_open_LUT2_image(char *dol_LUT2, dal_element **ptr_dal_LUT2, int chatter, int status;
int DAL3IBIS_open_L2RE(char *dol_L2RE, dal_element **ptr_dal_L2RE, int chatter, int status);
int DAL3IBIS_open_MCEC(char *dol_MCEC, dal_element **ptr_dal_MCEC, int chatter, int status);
int DAL3IBIS_open_EFFC(char *dol_EFFC, dal_element **ptr_dal_EFFC, int chatter, int status);
int DAL3IBIS_read_LUT1(dal_element **ptr_dal_LUT1, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status);
//int DAL3IBIS_read_LUT2_image(dal_element **ptr_ptr_dal_LUT2, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status);
int DAL3IBIS_read_LUT2(dal_element **ptr_ptr_dal_LUT2, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status);
int DAL3IBIS_read_L2RE(dal_element **ptr_ptr_dal_L2RE, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status);
int DAL3IBIS_read_MCEC(dal_element **ptr_ptr_dal_MCEC, ISGRI_energy_calibration_struct *ptr_ISGRI_energy_calibration, int chatter, int status);
int DAL3IBIS_read_EFFC(dal_element **ptr_ptr_dal_EFFC, ISGRI_efficiency_struct *ptr_ISGRI_efficiency, int chatter, int status);

int DAL3IBIS_open_PICsIT_GO(char *dol, dal_element **picsEnerTabPtr, int chatter, int status);
int DAL3IBIS_read_PICsIT_GO(dal_element **ptr_picsEnerTabPtr, PICsIT_energy_calibration_struct *ptr_PICsIT_energy_calibration, int chatter, int status);


int DAL3IBIS_read_IBIS_events(dal_element *workGRP,
        IBIS_type event_kind,
        IBIS_events_struct *ptr_IBIS_events,
        int gti,
        int chatter,
        int status
        );

int DAL3IBIS_read_REV_context_maps(dal_element   *REVcontext,       // DOL to the REV context
        int           Revol,             // Revolution number of the SCW
        OBTime        OBTend,            // End Time of the SCW
        dal_double    **LowThreshMap,    // Output: Map of Low Thresholds (keV)
        dal_int       **ONpixelsREVmap,  // Output: Map of Pixels Status for this REV
        ISGRI_efficiency_struct *ptr_ISGRI_efficiency,
        unsigned char chatter);

int DAL3IBIS_get_ISGRI_efficiency(double energy, int y, int z, ISGRI_efficiency_struct *ptr_ISGRI_efficiency, double *ptr_efficiency, int chatter, int status);

int DAL3IBIS_populate_DS(char *dol,  void * calibration_struct, char *DS, functype_open_DS func_open_DS, functype_read_DS func_read_DS, int chatter, int status);

#ifndef __CINT__
#ifdef __cplusplus
//}
#endif
#endif

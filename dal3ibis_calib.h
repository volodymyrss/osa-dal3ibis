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


#define N_MDU 8
#define N_LT 16
#define N_ISGRI_PIXEL_Y 128
#define N_ISGRI_PIXEL_Z 128

// ISGRI detector energy transformation structure
typedef struct {

    struct MDU_correction {
        double pha_offset[N_MDU];
        double pha_gain[N_MDU];
        double pha_gain2[N_MDU];
        double rt_offset[N_MDU];
        double rt_gain[N_MDU];
        double rt_pha_cross_gain[N_MDU];
    };

    struct LUT1 {
        double ** pha_gain;
        double ** pha_offset;
        double ** rt_gain;
        double ** rt_offset;
    };

    double ** LUT2;

    struct LUT2_rapid_evolution { // per pointing in principle
        double * ijd;
        double * pha_gain; 
        double * pha_gain2; 
        double * pha_offset;
        double * rt_gain;
        double * rt_offset;
    };

} ISGRI_energy_calibration_description;

int DAL3IBISreadMDUcorrection( dal_element *MDUCorrectionStructure, 
				ISGRI_energy_calibration_description *ISGRI_energy_calibration, 
				int          status );

int DAL3IBISreadLUT1( dal_element *LUT1Structure, 
				ISGRI_energy_calibration_description *ISGRI_energy_calibration, 
				int          status );

int DAL3IBISreadLUT2( dal_element *LUT2Structure, 
				ISGRI_energy_calibration_description *ISGRI_energy_calibration, 
				int          status );

int DAL3IBISTransformISGRIEnergy(int * ISGRI_PHA,
                int * ISGRI_RT,
                ISGRI_energy_calibration_description *ISGRI_energy_calibration,
				int          status );

// structure describing efficiency of ISGRI components
typedef struct {

    double * MDU_efficiency[N_MDU]; // energy-dependent
    double * LT_efficiency[N_LT]; // energy-dependent, per LT class
    double pixel_efficiency[N_ISGRI_PIXEL_Y][N_ISGRI_PIXEL_Y]; // grey, also used to kill pixels

} ISGRI_efficiency_description;

int DAL3IBISreadMDUEfficiency( dal_element *MDUEfficiencyStructure, 
				ISGRI_efficiency_description *ISGRI_efficiency, 
				int          status );

int DAL3IBISreadLTEfficiency( dal_element *LTEfficiencyStructure, 
				ISGRI_efficiency_description *ISGRI_efficiency, 
				int          status );

int DAL3IBISreadPixelEfficiency( dal_element *PixelEfficiencyStructure, 
				ISGRI_efficiency_description *ISGRI_efficiency, 
				int          status );

// compute
int DAL3IBISGetISGRIEfficiency(
                ISGRI_efficiency_description *ISGRI_efficiency,
				int          status);

// auxiliary calls to read HK

int DAL3IBIS_MceIsgriHkCal(dal_element *workGRP,
        OBTime       obtStart,
        OBTime       obtEnd,
        double       meanT[8],
        double       meanBias[8],
        int          chatter,
        int          status);

#ifndef __CINT__
#ifdef __cplusplus
}
#endif
#endif

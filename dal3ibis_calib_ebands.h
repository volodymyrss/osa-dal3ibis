#ifndef DAL3IBIS_CALIB_EBANDS_H
#define DAL3IBIS_CALIB_EBANDS_H

#define N_E_BAND 256 
#define E_BAND_N_REVERSE 2048
#define E_BAND_REVERSE_STEP 0.5

/*double C256_E_band_min[N_E_BAND];
double C256_E_band_max[N_E_BAND];

int C256_E_band_reverse[E_BAND_N_REVERSE];*/

inline double C256_get_E_min(int ch);
inline double C256_get_E_max(int ch);
inline int C256_get_channel(double energy);

int C256_setup_E_bands(int chatter);

inline double C256_get_E_min(int ch);

inline double C256_get_E_max(int ch);

inline int C256_get_channel(double energy);

#endif

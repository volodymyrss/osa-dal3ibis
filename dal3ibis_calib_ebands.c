#include "dal3ibis_calib_ebands.h"
#include <stdio.h>
#include <math.h>

// this is all hard-coded to avoid reading an extra file
// this is all hard-coded because templates enforce 256 bins anyway
// this is all hard-coded because background maps are provided in the same bins

double C256_E_band_min[N_E_BAND];
double C256_E_band_max[N_E_BAND];

int C256_E_band_reverse[E_BAND_N_REVERSE];


int C256_setup_E_bands(int chatter) {
    int bin;
    double DeltaE=0.;
    int i;

    C256_E_band_min[0]=12.;

    for(bin=0;bin<N_E_BAND;bin++) {
        DeltaE += 0.5*(1+round(0.054*bin));

        C256_E_band_max[bin]=12.0+DeltaE;

        if (bin>0) {
            C256_E_band_min[bin] = C256_E_band_max[bin-1];
        }

        if (chatter>9) 
            printf("%.5lg %.5lg %.5lg %i %i\n",C256_E_band_min[bin], C256_E_band_max[bin], DeltaE,(int)trunc(C256_E_band_min[bin]/0.5),(int)trunc(C256_E_band_max[bin]/0.5));
        for (i=trunc(C256_E_band_min[bin]/E_BAND_REVERSE_STEP);i<trunc(C256_E_band_max[bin]/E_BAND_REVERSE_STEP);i++) 
            C256_E_band_reverse[i]=bin;
    }

}

inline double C256_get_E_min(int ch) {
    if ( ch<0 || ch>=N_E_BAND ) return 0.; 
    return (double)C256_E_band_min[ch];
}

inline double C256_get_E_max(int ch) {
    if ( ch<0 || ch>=N_E_BAND ) return 0.; 
    return (double)C256_E_band_max[ch];
}

inline int C256_get_channel(double energy) {
    int r_ch;
    int ch;

    r_ch=trunc(energy/E_BAND_REVERSE_STEP);

    if ( r_ch<0 || r_ch>=E_BAND_N_REVERSE ) return 0; 

    ch=C256_E_band_reverse[r_ch];

    if (energy>C256_E_band_max[ch] || energy<C256_E_band_min[ch]) return 0;

    return ch;
}


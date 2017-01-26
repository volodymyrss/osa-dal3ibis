#include "dal3ibis_calib_ebands.h"

int setup_E_bands() {
    int bin;
    double DeltaE=0.;
    int i;

    E_band_min[0]=12.;

    for(bin=0;bin<N_E_BAND;bin++) {
        DeltaE += 0.5*(1+round(0.054*bin));

        E_band_max[bin]=12.0+DeltaE;

        if (bin>0) {
            E_band_min[bin] = E_band_max[bin-1];
        }

        printf("%.5lg %.5lg %.5lg %i %i\n",E_band_min[bin], E_band_max[bin], DeltaE,(int)trunc(E_band_min[bin]/0.5),(int)trunc(E_band_max[bin]/0.5));
        for (i=trunc(E_band_min[bin]/E_BAND_REVERSE_STEP);i<trunc(E_band_max[bin]/E_BAND_REVERSE_STEP);i++) 
            E_band_reverse[i]=bin;
    }

}

inline double get_E_min(int ch) {
    if ( ch<0 || ch>=N_E_BAND ) return 0.; 
    return (double)E_band_min[ch];
}

inline double get_E_max(int ch) {
    if ( ch<0 || ch>=N_E_BAND ) return 0.; 
    return E_band_max[ch];
}

inline int get_channel(double energy) {
    int r_ch;
    int ch;

    r_ch=trunc(energy/E_BAND_REVERSE_STEP);

    if ( r_ch<0 || r_ch>=E_BAND_N_REVERSE ) return 0; 

    ch=E_band_reverse[r_ch];

    if (energy>E_band_max[ch] || energy<E_band_min[ch]) return 0;

    return ch;
}


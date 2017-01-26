#define N_E_BAND 256 

double E_band_min[N_E_BAND];
double E_band_max[N_E_BAND];

#define E_BAND_N_REVERSE 2048
#define E_BAND_REVERSE_STEP 0.5

int E_band_reverse[E_BAND_N_REVERSE];

inline double get_E_min(int ch);
inline double get_E_max(int ch);
inline int get_channel(double energy);


int setup_E_bands();

inline double get_E_min(int ch);

inline double get_E_max(int ch);

inline int get_channel(double energy);

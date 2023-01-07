#define MS_MATRIX_URI "https://github.com/lherg/ms_matrix.lv2"

#define DB_CO(g) ((g) > -60.0f ? powf(10.0f, (g)*0.05f) : 0.0f)
#define CO_DB(c) ((c) > 1e-10f ? 20*log10(c) : -99.0f)

typedef enum { 
  MTX_SWAP,
  MTX_MMUTE,
  MTX_SMUTE,  
  MTX_MHPF,
  MTX_SHPF,
  MTX_MPHASE,
  MTX_SPHASE,
  MTX_HMGAIN,
  MTX_HSGAIN,
  MTX_FMGAIN,
  MTX_FSGAIN,
  MTX_PEAKIN0,
  MTX_PEAKIN1,
  MTX_PEAKOUT0,
  MTX_PEAKOUT1,
  MTX_RMSIN0,
  MTX_RMSIN1,
  MTX_RMSOUT0,
  MTX_RMSOUT1,     
  MTX_INPUT0,
  MTX_OUTPUT0,
  MTX_INPUT1,
  MTX_OUTPUT1,
  MTX_LAST
} PortIndex;

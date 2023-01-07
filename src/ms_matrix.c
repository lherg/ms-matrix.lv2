// Copyright 2006-2016 David Robillard <d@drobilla.net>
// Copyright 2006 Steve Harris <steve@plugin.org.uk>
// SPDX-License-Identifier: ISC

#include "lv2/core/lv2.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "ms_matrix.h"

typedef struct {
  // Port buffers
	float* _port[MTX_LAST];

  double sample_rate;
  double rms_time;
  float peakin0;
  float peakin1;
  float peakout0;
  float peakout1;

  float s_rmsin0;
  float s_rmsin1;
  float rmsin0;  
  float rmsin1;

  float s_rmsout0;
  float s_rmsout1;
  float rmsout0;  
  float rmsout1;  

	uint32_t rms_sample_count;
	uint32_t rms_sample_max;
} Amp;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
  Amp* amp = (Amp*)calloc(1, sizeof(Amp));

  amp->sample_rate = rate;
  amp->rms_time = 0.3f;
  amp->rms_sample_max = ceilf(rate * 0.05f); //?? TODO ms

  amp->rms_sample_count = 0; 

  amp->peakin0 = 0.f;
  amp->peakin1 = 0.f;
  amp->peakout0 = 0.f;
  amp->peakout1 = 0.f;

  amp->s_rmsin0 = 0;
  amp->s_rmsin1 = 0;
  amp->rmsin0 = 0;  
  amp->rmsin1 = 0;

  amp->s_rmsout0 = 0;
  amp->s_rmsout1 = 0;
  amp->rmsout0 = 0;  
  amp->rmsout1 = 0;  

  return (LV2_Handle)amp;
}

static void
connect_port(LV2_Handle instance, uint32_t port, void* data)
{
  Amp* amp = (Amp*)instance;

	if (port <MTX_LAST) {
		amp->_port[port] = (float*)data;
	}
}

static void
activate(LV2_Handle instance){ }

static void
run(LV2_Handle instance, uint32_t n_samples)
{
  Amp* amp = (Amp*)instance;

  const bool        swap  = *(amp->_port[MTX_SWAP]);

  const bool        mmute  = *(amp->_port[MTX_MMUTE]);
  const bool        smute = *(amp->_port[MTX_SMUTE]);

  const bool        mhpf  = *(amp->_port[MTX_MHPF]);
  const bool        shpf = *(amp->_port[MTX_SHPF]);

  const bool        mphase   = *(amp->_port[MTX_MPHASE]);
  const bool        sphase  = *(amp->_port[MTX_SPHASE]);

  const float        hmgain   = *(amp->_port[MTX_HMGAIN]);
  const float        hsgain   = *(amp->_port[MTX_HSGAIN]);  

  const float        fmgain   = *(amp->_port[MTX_FMGAIN]);
  const float        fsgain   = *(amp->_port[MTX_FSGAIN]); 

	float* ins[2]  = { amp->_port[MTX_INPUT0], amp->_port[MTX_INPUT1] };
	float* outs[2] = { amp->_port[MTX_OUTPUT0], amp->_port[MTX_OUTPUT1] };

  float mcoef = DB_CO(hmgain + fmgain);
  float scoef = DB_CO(hsgain + fsgain);

  if (mphase) mcoef = - mcoef;
  if (sphase) scoef = - scoef;

  if (mmute) mcoef = 0;
  if (smute) scoef = 0;

  //fprintf(stderr, "LV2|mswap: %d\n", swap);  
  //fprintf(stderr, "LV2|mmute: %d\n", mmute);
  //fprintf(stderr, "LV2|smute: %d\n", smute);

  for (uint32_t pos = 0; pos < n_samples; pos++) {
    outs[0][pos] = swap ? ins[1][pos] * mcoef + ins[0][pos] * scoef : ins[0][pos] * mcoef + ins[1][pos] * scoef;
    outs[1][pos] = swap ? ins[1][pos] * mcoef - ins[0][pos] * scoef : ins[0][pos] * mcoef - ins[1][pos] * scoef;

    // Compute Peak in / out
    if (fabsf(ins[0][pos])  > amp->peakin0  ) amp->peakin0  = fabsf(ins[0][pos]);
    if (fabsf(ins[1][pos])  > amp->peakin1  ) amp->peakin1  = fabsf(ins[1][pos]); 
    if (fabsf(outs[0][pos]) > amp->peakout0 ) amp->peakout0 = fabsf(outs[0][pos]);
    if (fabsf(outs[1][pos]) > amp->peakout1 ) amp->peakout1 = fabsf(outs[1][pos]);

    // Compute sum rms in / out
    amp->rms_sample_count++;
    amp->s_rmsin0 += (ins[0][pos] * ins[0][pos]);
    amp->s_rmsin1 += (ins[1][pos] * ins[1][pos]);
    amp->s_rmsout0 += (outs[0][pos] * outs[0][pos]);
    amp->s_rmsout1 += (outs[1][pos] * outs[1][pos]);

    if (amp->rms_sample_count == amp->rms_sample_max){
      *amp->_port[MTX_PEAKIN0]=CO_DB(amp->peakin0);
      *amp->_port[MTX_PEAKIN1]=CO_DB(amp->peakin1);
      *amp->_port[MTX_PEAKOUT0]=CO_DB(amp->peakout0);
      *amp->_port[MTX_PEAKOUT1]=CO_DB(amp->peakout1);
       
      amp->rmsin0 = CO_DB(sqrtf( amp->s_rmsin0 / amp->rms_sample_max ));
      amp->rmsin1 = CO_DB(sqrtf( amp->s_rmsin1 / amp->rms_sample_max ));
      amp->rmsout0 = CO_DB(sqrtf( amp->s_rmsout0 / amp->rms_sample_max ));
      amp->rmsout1 = CO_DB(sqrtf( amp->s_rmsout1 / amp->rms_sample_max ));
      *amp->_port[MTX_RMSIN0]=amp->rmsin0;
      *amp->_port[MTX_RMSIN1]=amp->rmsin1;
      *amp->_port[MTX_RMSOUT0]=amp->rmsout0;
      *amp->_port[MTX_RMSOUT1]=amp->rmsout1;

      // reset
      amp->peakin0=0;
      amp->peakin1=0;
      amp->peakout0=0;
      amp->peakout1=0;       
      amp->rms_sample_count = 0;
      amp->rmsin0=0;
      amp->rmsin1=0;
      amp->rmsout0=0;
      amp->rmsout1=0;      
      amp->s_rmsin0=0;
      amp->s_rmsin1=0;
      amp->s_rmsout0=0;
      amp->s_rmsout1=0;
 
    }    
  }
}

static void
deactivate(LV2_Handle instance)
{}

static void
cleanup(LV2_Handle instance)
{
  free(instance);
}

static const void*
extension_data(const char* uri)
{
  return NULL;
}

static const LV2_Descriptor descriptor = {MS_MATRIX_URI,
                                          instantiate,
                                          connect_port,
                                          activate,
                                          run,
                                          deactivate,
                                          cleanup,
                                          extension_data};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
  return index == 0 ? &descriptor : NULL;
}

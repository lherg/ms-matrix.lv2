// Copyright 2006-2016 David Robillard <d@drobilla.net>
// Copyright 2006 Steve Harris <steve@plugin.org.uk>
// SPDX-License-Identifier: ISC

#include "lv2/core/lv2.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ms_matrix.h"

typedef struct {
  float rate;
  float freq;

  float B0, B1, B2, A0, A1, A2;
  float b0, b1, b2, a0, a1, a2;
  float x1, x2, y1, y2;

} Filter;

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
	Filter mhip;
	Filter ship;  
    
} Amp;

static void hip_setup (Filter *hip, float rate, float freq) {
  memset (hip, 0, sizeof(Filter));
	hip->rate = rate;
	hip->freq = freq;

	float wc = 2 * M_PI * freq; // (rad / s)

  hip->A0 = 0;
  hip->A1 = 1 / wc;
  hip->A2 = 1; 
  hip->B0 = 0;
  hip->B1 = 1 / wc;
  hip->B2 = 0;  

  hip->a0 = (hip->A2 + 2 * hip->A1 *rate + 4 * hip->A0 * rate *rate) / (hip->A2 + 2 * hip->A1 *rate + 4 * hip->A0 * rate *rate);
  hip->a1 = (2 * hip->A2 - 8 *  hip->A0 * rate *rate) / (hip->A2 + 2 * hip->A1 *rate + 4 * hip->A0 * rate *rate);
  hip->a2 = (hip->A2 - 2 * hip->A1 *rate + 4 * hip->A0 * rate *rate) / (hip->A2 + 2 * hip->A1 *rate + 4 * hip->A0 * rate *rate);
  hip->b0 = (hip->B2 + 2 * hip->B1 *rate + 4 * hip->B0 * rate *rate) / (hip->A2 + 2 * hip->A1 *rate + 4 * hip->A0 * rate *rate);
  hip->b1 = (2 * hip->B2 - 8 *  hip->B0 * rate *rate) / (hip->A2 + 2 * hip->A1 *rate + 4 * hip->A0 * rate *rate);
  hip->b2 = (hip->B2 - 2 * hip->B1 *rate + 4 * hip->B0 * rate *rate) / (hip->A2 + 2 * hip->A1 *rate + 4 * hip->A0 * rate *rate);  

  hip->x1 = 0.f;
  hip->x2 = 0.f; 
  hip->y1 = 0.f;
  hip->y2 = 0.f;   
}

static void hip_compute (Filter *hip, uint32_t n_samples, float *buf) {
  
  float x, y;
  
  for (uint32_t i = 0; i < n_samples; ++i)
  {
    x = buf[i];
    y = hip->b0 * x + hip->b1 * hip->x1 + hip->b2 * hip->x2 - hip->a1 * hip->y1 - hip->a2 * hip->y2 ;

    hip->y2 = hip->y1;   
    hip->y1 = y;  
    hip->x2 = hip->x1;   
    hip->x1 = x;    

    buf [i] = y;
  
  } 
}


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
  Amp* amp = (Amp*)calloc(1, sizeof(Amp));

  amp->sample_rate = rate;
  amp->rms_time = 0.3f;
  amp->rms_sample_max = ceilf(rate * 0.05f); //50 ms

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

  hip_setup (&amp->mhip, rate, 80);
  hip_setup (&amp->ship, rate, 80);  
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

	if (mhpf) hip_compute (&amp->mhip, n_samples, ins[0]);
	if (shpf) hip_compute (&amp->ship, n_samples, ins[1]);

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

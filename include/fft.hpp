/* libfft.h - include file for fast Fourier transform library
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#ifndef LIBFFT_H
#define LIBFFT_H

#include "util.hpp"

#define MAXFFTSIZE 32'768
#define LOG2_MAXFFTSIZE 15

class fft_t {
public:
  fft_t(float sample_rate, int size);
  virtual ~fft_t();

  void apply(const float *x, bool inv = false);
  const float *compute_amplitude();
  const float *get_amp_table();
  const float *get_frq_table();
  const peak_t *get_peak();

private:
  int    bitreverse[MAXFFTSIZE];
  int    bits;
  float *xr;
  float *xi;
  float *f;
  float *amp;
  peak_t peak;
};

#endif // LIBFFT_H

/* libfft.c - fast Fourier transform library
 *
 * Copyright (C) 1989 by Jef Poskanzer.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 *
 * minor midifications July 2012 Bjorn Roche
 */

#include "fft.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* fft_init - initialize for fast Fourier transform
 *
 * b    power of two such that 2**nu = number of samples
 */
fft_t::fft_t(float sample_rate, int size)
{
  int i, j, k;

  bits = log2(size);

  if (bits > LOG2_MAXFFTSIZE) {
    fprintf(stderr, "%d is too many bits, max is %d\n", bits, LOG2_MAXFFTSIZE);
    exit(EXIT_FAILURE);
  }

  for (i = (1 << bits) - 1; i >= 0; --i) {
    k = 0;

    for (j = 0; j < bits; ++j) {
      k *= 2;

      if (i & (1 << j)) {
        k += 1;
      }
    }

    bitreverse[i] = k;
  }

  xr  = new float[size];
  xi  = new float[size];
  f   = new float[size];
  amp = new float[size];

  for (i = 0; i < size; i++) {
    f[i] = (sample_rate * i) / (float)(size);
  }
}

fft_t::~fft_t()
{
  delete[] xr;
  delete[] xi;
  delete[] f;
  delete[] amp;
}

/* fft_apply - a fast Fourier transform routine
 *
 * xr   real part of data to be transformed
 * xi   imaginary part (normally zero, unless inverse transform in effect)
 * inv  flag for inverse
 */
void fft_t::apply(const float *x, bool inv)
{
  int n, n2, i, k, kn2, l, p;
  float ang, s, c, tr, ti;
  // double ds, dc;

  n = 1 << bits;
  n2 = n / 2;

  memcpy(xr, x, n * sizeof(float)); // XXX: check if this is working
  memset(xi, 0, n * sizeof(float));

  for (l = 0; l < bits; ++l) {
    for (k = 0; k < n; k += n2) {
      for (i = 0; i < n2; ++i, ++k) {
        p = bitreverse[k / n2];
        ang = 6.283185 * p / n;
        c = cos(ang);
        // s = sin(ang);
        s = inv ? -sin(ang) : sin(ang);
        kn2 = k + n2;
        // if (inv) s = -s;
        tr = xr[kn2] * c + xi[kn2] * s;
        ti = xi[kn2] * c - xr[kn2] * s;
        xr[kn2] = xr[k] - tr;
        xi[kn2] = xi[k] - ti;
        xr[k] += tr;
        xi[k] += ti;
      }
    }
    n2 /= 2;
  }

  for (k = 0; k < n; ++k) {
    i = bitreverse[k];
    if (i <= k) continue;
    tr = xr[k];
    ti = xi[k];
    xr[k] = xr[i];
    xi[k] = xi[i];
    xr[i] = tr;
    xi[i] = ti;
  }

  // Finally, multiply each value by 1/n, if this is the forward transform
  if (!inv) {
    float f = 1.0f / n;

    for (i = 0; i < n; ++i) {
      xr[i] *= f;
      xi[i] *= f;
    }
  }
}

const float *fft_t::compute_amplitude()
{
  int n = 1 << bits;

  peak.amp = -1.0;
  peak.idx = -1;

  for (int i = 0; i < n / 2; i++) {
    amp[i] = sqrt(xr[i] * xr[i] + xi[i] + xi[i]);

    if (amp[i] > peak.amp) {
      peak.amp = amp[i];
      peak.idx = i;
    }
  }

  assert(peak.idx >= 0);
  peak.frq = f[peak.idx];

  return amp;
}

const float *fft_t::get_amp_table()
{
  return amp;
}

const float *fft_t::get_frq_table()
{
  return f;
}

const peak_t *fft_t::get_peak()
{
  return &peak;
}

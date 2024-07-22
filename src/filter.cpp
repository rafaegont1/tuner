#include "filter.hpp"

#include <string.h>
#include <math.h>

filter_t::filter_t(double sample_rate, double cutoff_freq, int size)
  : size(size)
{
  setup(sample_rate, cutoff_freq);
}

filter_t::~filter_t() { }

void filter_t::setup(double sample_rate, double cutoff_freq)
{
  double a0;
  double w0 = 2.0f * M_PI * cutoff_freq / sample_rate;
  double cosw0 = cos(w0);
  double sinw0 = sin(w0);
  // double alpha = sinw0/2;
  double alpha = sinw0 / 2.0f * sqrtf(2.0f);

  a0 = 1.0f + alpha;
  a[0] = (-2.0f * cosw0) / a0;
  a[1] = (1.0f - alpha) / a0;
  b[0] = ((1.0f - cosw0) / 2.0f) / a0;
  b[1] = (1.0f - cosw0) / a0;
  b[2] = b[0];
}

double filter_t::apply(const double x)
{
  float y = b[0] * x + b[1] * mem[0] + b[2] * mem[1]
    - a[0] * mem[2] - a[1] * mem[3];

  mem[1] = mem[0];
  mem[0] = x;
  mem[3] = mem[2];
  mem[2] = y;

  return y;
}

void filter_t::reset()
{
  memset(mem, 0, sizeof(mem));
}

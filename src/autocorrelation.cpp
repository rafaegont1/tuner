#include "autocorrelation.hpp"

#include <string.h>

Autocorrelation::Autocorrelation(int size, float sample_rate)
: n(size), sample_rate(sample_rate)
{
  res = new float[n];
}

Autocorrelation::~Autocorrelation()
{
  delete[] res;
}

const float *Autocorrelation::apply(const float *input)
{
  memset(res, 0, n * sizeof(float));

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n - i; j++) {
      res[i] += input[j] * input[j + i];
    }
  }

  return res;
}

float Autocorrelation::get_freq()
{
  float sum = 0.0;
  float thresh = res[0] * 0.5;
  int period = 0;
  int state = 1;

  for (int i = 1; i < n; ++i) {
    if (state == 1 && res[i] > thresh && res[i] - res[i-1] > 0) {
      state = 2;
    } else if (state == 2 && res[i] - res[i-1] <= 0) {
      period = i;
      break;
    }
  }

  return sample_rate / period;
}

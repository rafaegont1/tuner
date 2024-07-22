#include "autocorrelation.hpp"

Autocorrelation::Autocorrelation(int size, float fs)
: fs(fs)
{
  res.resize(size);
}

Autocorrelation::~Autocorrelation() {}

float Autocorrelation::get_frequency(const float *input)
{
  const int n = res.size();
  int state = 0;
  int period = 0;
  float thresh;

  std::fill(res.begin(), res.end(), 0.0f);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n - i; j++) {
      res[i] += input[j] * input[j + i];
    }

    switch (state) {
      case 0:
        thresh = 0.5f * res[0];
        state = 1;
        break;
      case 1:
        if ((res[i] > thresh) && (res[i] - res[i-1] > 0)) {
          state = 2;
        }
        break;
      case 2:
        if (res[i] - res[i-1] <= 0) {
          period = i;
          state = 3;
        }
        break;
    };

    if (state == 3) break;
  }

  return fs / period;
}

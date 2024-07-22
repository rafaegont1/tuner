#include "window.hpp"

#include <math.h>

window_t::window_t() { }

window_t::window_t(int size, enum type t)
  : size(size)
{
  coeff = new float[size];
  setup(t);
}

window_t::~window_t()
{
  delete[] coeff;
}

void window_t::init(int size, enum type t)
{
  this->size = size;
  coeff = new float[size];
  setup(t);
}

void window_t::setup(enum type t)
{
  constexpr float bell_w = 0.4; // constant for gauss method

  switch (t) {
    case blackman_harris:
      for (int i = 0; i < size; i++)
        coeff[i] = 0.35875 - 0.48829 * cos(2.0 * M_PI * i / (size - 1))
          + 0.14128 * cos(4.0 * M_PI * i / (size - 1))
          - 0.01168 * cos(6.0 * M_PI * i / (size - 1));
      break;

    case gauss:
      for (int i = 0; i < size; i++)
        coeff[i] = pow(M_E, -0.5 * pow((i - (size - 1) * 0.5) /
          (bell_w * (size - 1) * 0.5), 2.0));
      break;

    case hamming:
      for (int i = 0; i < size; i++)
        coeff[i] = 0.54 - 0.46 * cos(2.0 * M_PI * i / (float)size);
      break;

    case hann:
      for (int i = 0; i < size; i++)
        coeff[i] = 0.5 * (1.0 - cos(2.0 * M_PI * i / (float)(size - 1)));
      break;

    case triangular:
      for (int i = 0; i < size; i++)
        coeff[i] = (2.0 / size) * ((size * 0.5) - fabs(i - (size - 1.0) * 0.5));
      break;
  }
}

void window_t::apply(const float *x, float *y)
{
  for (int i = 0; i < size; i++) {
    y[i] = x[i] * coeff[i];
  }
}

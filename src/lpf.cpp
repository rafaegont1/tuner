#include "lpf.hpp"

#include <cmath>

Lpf::Lpf() {}

Lpf::Lpf(double fs, double fc)
{
  this->setup(fs, fc);
}

Lpf::~Lpf() { }

void Lpf::setup(double fs, double fc)
{
  const double w0 = 2.0 * M_PI * fc / fs;
  const double cosw0 = std::cos(w0);
  const double sinw0 = std::sin(w0);
  const double alpha = sinw0 / 2.0 * sqrt(2.0);
  const double a0 = 1.0f + alpha;

  a[0] = (-2.0f * cosw0) / a0;
  a[1] = (1.0f - alpha) / a0;

  b[0] = ((1.0f - cosw0) / 2.0f) / a0;
  b[1] = (1.0f - cosw0) / a0;
  b[2] = b[0];

  old_x[0] = 0.0;
  old_x[1] = 0.0;

  old_y[0] = 0.0;
  old_y[1] = 0.0;
}

double Lpf::apply(const double x)
{
  double y = b[0] * x + b[1] * old_x[0] + b[2] * old_x[1]
    - a[0] * old_y[0] - a[1] * old_y[1];

  old_x[1] = old_x[0];
  old_x[0] = x;

  old_y[1] = old_y[0];
  old_y[0] = y;

  return y;
}

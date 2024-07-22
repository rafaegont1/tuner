#ifndef AUTOCORRELATION_HPP
#define AUTOCORRELATION_HPP

#include <vector>

class Autocorrelation
{
public:
  Autocorrelation(int size, float fs);
  virtual ~Autocorrelation();

  float get_frequency(const float *input);

private:
  float fs;
  std::vector<float> res;
};

#endif // AUTOCORRELATION_HPP

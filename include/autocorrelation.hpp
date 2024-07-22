#ifndef AUTOCORRELATION_HPP
#define AUTOCORRELATION_HPP

class Autocorrelation {
public:
  Autocorrelation(int size, float sample_rate);
  virtual ~Autocorrelation();

  const float *apply(const float *x);
  float get_freq();

private:
  int n;
  float sample_rate;
  float *res;
};

#endif // AUTOCORRELATION_HPP

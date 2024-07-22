#ifndef FILTER_HPP
#define FILTER_HPP

class filter_t {
public:
  filter_t(double sample_rate, double cutoff_freq, int size);
  virtual ~filter_t();

  void setup(double sample_rate, double cutoff_freq);
  double apply(const double x);
  void reset();

private:
  double a[2];
  double b[3];
  double mem[4];
  int size;
};

#endif // FILTER_HPP

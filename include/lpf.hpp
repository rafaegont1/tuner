#ifndef FILTER_HPP
#define FILTER_HPP

class Lpf {
public:
  Lpf();
  Lpf(double fs, double fc);
  virtual ~Lpf();

  void setup(double fs, double fc);
  double apply(const double x);

private:
  double a[2];
  double b[3];
  double old_x[2];
  double old_y[2];
};

#endif // FILTER_HPP

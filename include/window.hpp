#ifndef WINDOW_HPP
#define WINDOW_HPP

class window_t {
public:
  enum type { blackman_harris, gauss, hamming, hann, triangular };

  window_t();
  window_t(int size, enum type t = type::hamming);
  virtual ~window_t();

  void init(int size, enum type t = type::hamming);
  void setup(enum type t);
  void apply(const float *x, float *y);

private:
  float *coeff;
  int size;
};

#endif // WINDOW_HPP

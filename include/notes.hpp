#ifndef NOTES_HPP
#define NOTES_HPP

#include "util.hpp"

class PitchDetector {
public:
  struct Result {
    double f;
    const char *name;
    double cents_sharp;
    bool in_tune;
  };

  PitchDetector();
  virtual ~PitchDetector();

  const Result &find(const double pitch, const double accuracy);
  bool is_in_tune();

private:
  inline const char *note_name(const int idx);
  inline int get_closest(int lidx, int hidx, double target);
  int binary_search_idx(const double pitch, const double accuracy);
  inline double diff(const double a, const double b);

  static constexpr int n = 127;

  double notetb[n];
  Result res;
};

#endif // NOTES_HPP

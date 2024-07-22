#include "PitchDetector.hpp"

#include <math.h>

PitchDetector::PitchDetector()
{
  for (int i = 0; i < n; i++) {
    pitchtb[i] = (440.0 / 32.0) * std::pow(2.0, (i - 9.0) / 12.0);
  }
}

PitchDetector::~PitchDetector() { }

const PitchDetector::Result &PitchDetector::find(
  const double f, const double accuracy)
{
  const int idx = binary_search(f, accuracy);

  res.f = pitchtb[idx];
  res.note = NOTES[idx%12];
  res.octave = idx / 12 - 1;
  res.cents = 1'200.0 * std::log2(f / res.f);
  res.in_tune = (std::abs(f - pitchtb[idx]) < accuracy);

  return res;
}

int PitchDetector::binary_search(const double f, const double accuracy)
{
  if (f < pitchtb[0]) {
    return 0;
  } else if (f > pitchtb[n - 1]) {
    return n - 1;
  }

  int idx = -1; // nearest pitch index
  int left = 0;
  int right = n - 1;

  while (left < right) {
    int mid = (left + right) / 2;

    if (std::abs(f - pitchtb[mid]) < accuracy) {
      // The target is in the middle
      idx = mid;
      break;
    } else if (f < pitchtb[mid]) {
      // Go left
      if (mid > 0 && f > pitchtb[mid - 1]) {
        // The target is in the left
        idx = get_closest(mid - 1, mid, f);
        break;
      }
      right = mid;
    } else {
      // Go right
      if (mid < n - 1 && f < pitchtb[mid + 1]) {
        // The target is in the right
        idx = get_closest(mid, mid + 1, f);
        break;
      }
      left = mid + 1;
    }
  }

  return idx;
}

inline int PitchDetector::get_closest(int lidx, int hidx, double target)
{
  return (target - pitchtb[lidx] < pitchtb[hidx] - target) ? lidx : hidx;
}

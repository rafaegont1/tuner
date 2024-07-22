#include "notes.hpp"

#include <math.h>

PitchDetector::PitchDetector()
{
  for (int i = 0; i < n; i++) {
    notetb[i] = (440.0 / 32.0) * std::pow(2.0, (i - 9.0) / 12.0);
  }
}

PitchDetector::~PitchDetector() { }

inline const char *PitchDetector::note_name(const int idx)
{
  static constexpr const char * NOTES[] = {
    "C",  "C#", "D",  "D#", "E",  "F",  "F#", "G", "G#", "A",  "A#", "B"
  };
  return NOTES[idx % 12];
}

inline double PitchDetector::diff(const double a, const double b)
{
  return std::abs(a - b);
}

int PitchDetector::binary_search_idx(const double pitch, const double accuracy)
{
  if (pitch < notetb[0]) {
    return 0;
  } else if (pitch > notetb[n - 1]) {
    return n - 1;
  }

  int idx = -1; // nearest pitch index
  int left = 0;
  int right = n - 1;

  while (left < right) {
    int mid = (left + right) / 2;

    if (diff(pitch, notetb[mid]) < accuracy) { // TODO: check if this is working
      idx = mid;
      break;
    }

    if (pitch < notetb[mid]) {
      if (mid > 0 && pitch > notetb[mid - 1]) {
        idx = get_closest(mid - 1, mid, pitch);
        break;
      }
      right = mid;
    } else {
      if (mid < n - 1 && pitch < notetb[mid + 1]) {
        idx = get_closest(mid, mid + 1, pitch);
        break;
      }
      left = mid + 1;
    }
  }

  return idx;
}

const PitchDetector::Result &PitchDetector::find(
  const double pitch, const double accuracy)
{
  const int idx = binary_search_idx(pitch, accuracy);

  res.f = notetb[idx];
  res.name = this->note_name(idx);
  res.cents_sharp = 1'200.0 * log2(pitch / res.f);
  res.in_tune = (diff(pitch, notetb[idx]) < accuracy);

  return res;
}

inline int PitchDetector::get_closest(int lidx, int hidx, double target)
{
  return (target - notetb[lidx] < notetb[hidx] - target) ? lidx : hidx;
}

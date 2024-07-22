#ifndef NOTES_HPP
#define NOTES_HPP

class PitchDetector {
public:
  struct Result {
    double f;
    const char *note;
    int octave;
    double cents;
    bool in_tune;
  };

  PitchDetector();
  virtual ~PitchDetector();

  const Result &find(const double f, const double accuracy);
  bool is_in_tune();

private:
  static constexpr int n = 128;
  static constexpr const char * NOTES[] = {
    "C",  "C#", "D",  "D#", "E",  "F",  "F#", "G", "G#", "A",  "A#", "B"
  };

  int binary_search(const double pitch, const double accuracy);
  inline int get_closest(int lidx, int hidx, double target);

  double pitchtb[n];
  Result res;
};

#endif // NOTES_HPP

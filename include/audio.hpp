#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <vector>
#include "miniaudio.h"
#include "lpf.hpp"

class Audio {
public:
  struct Result {
    std::vector<float> raw; // raw audio frames
    std::vector<float> flt; // low-pass filtered audio frames
  };

  Audio(ma_uint32 frame_count, ma_uint32 srate, double cutoff);
  virtual ~Audio();

  void setup_filter(double fs, double fc);
  const Result *get_buffer();

private:
  void exit_with_error();

  Lpf lpf;
  ma_result ma_res;
  ma_device ma_dev;
  ma_device_config ma_conf;
  Result res;
};

#endif // AUDIO_HPP

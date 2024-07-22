#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "miniaudio.h"

namespace audio {

struct frames_t {
  float *x; // raw audio frames
  float *y; // low-pass filtered audio frames
};

void init(ma_uint32 frame_count, ma_uint32 srate, double cutoff);
void deinit();
const frames_t *get_buffer();
void setup_filter(double srate, double cutoff);
bool is_buffer_new();

} // namespace audio

#endif // AUDIO_HPP

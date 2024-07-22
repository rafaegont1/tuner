#ifndef PA_HPP
#define PA_HPP

#include <portaudio.h>

class pa_t {
public:
  pa_t(int buffer_sz, int sample_rate);
  virtual ~pa_t();

  int     buffer_sz;
  float   *read_stream();
  PaError get_error_code();
  void    exit_with_error();

private:
  float              *data;
  PaStream           *stream;
  PaStreamParameters config;
  const PaDeviceInfo *info;
  PaError            err;
};

#endif // PA_HPP

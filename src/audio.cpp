#define MINIAUDIO_IMPLEMENTATION
#include "audio.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "lpf.hpp"
// #include "ETFE.hpp"
// #include "window.hpp"

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

ma_result result;
ma_device device;
ma_device_config dev_conf;
Lpf lpf;

float *buffer;
audio::frames_t frames;
int buffer_size;
bool new_buffer;

void data_callback(
  ma_device* pDevice, void* pOutput,
  const void* pInput, ma_uint32 frameCount)
{
  pthread_mutex_lock(&mtx);
  memcpy(buffer, pInput, frameCount * sizeof(float));
  new_buffer = true;
  pthread_mutex_unlock(&mtx);

  (void)pOutput;
}

static void exit_with_error()
{
  fprintf(stderr,
    "There has been an error with miniaudio.\n"
    "miniaudio error code: %d\n", result
  );

  exit(EXIT_FAILURE);
}

void audio::init(ma_uint32 frame_count, ma_uint32 srate, double cutoff)
{
  buffer = new float[frame_count];
  frames.x = new float[frame_count];
  frames.y = new float[frame_count];

  buffer_size = frame_count;
  new_buffer = false;

  memset(frames.x, 0, frame_count * sizeof(float));

  dev_conf = ma_device_config_init(ma_device_type_capture);
  dev_conf.sampleRate = srate;
  dev_conf.periodSizeInFrames = frame_count;
  dev_conf.dataCallback = data_callback;
  dev_conf.capture.format = ma_format_f32;
  dev_conf.capture.channels = 1;

  result = ma_device_init(nullptr, &dev_conf, &device);
  if (result != MA_SUCCESS) exit_with_error();

  result = ma_device_start(&device);
  if (result != MA_SUCCESS) exit_with_error();

  lpf.setup(srate, cutoff);
}

void audio::deinit()
{
  delete[] buffer;
  delete[] frames.x;
  delete[] frames.y;

  ma_device_stop(&device);
  ma_device_uninit(&device);
}

const audio::frames_t *audio::get_buffer()
{
  if (new_buffer) {
    pthread_mutex_lock(&mtx);
    memcpy(frames.x, buffer, buffer_size * sizeof(float));
    new_buffer = false;
    pthread_mutex_unlock(&mtx);

    for (int i = 0; i < buffer_size; i++) {
      frames.y[i] = lpf.apply(frames.x[i]);
    }
  }

  return &frames;
}

void audio::setup_filter(double srate, double cutoff)
{
  lpf.setup(srate, cutoff);
}

bool audio::is_buffer_new()
{
  return new_buffer;
}

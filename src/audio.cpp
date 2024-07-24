#define MINIAUDIO_IMPLEMENTATION
#include "audio.hpp"

#include <cstdio>
#include <cstdlib>
#include <mutex>

// #include "ETFE.hpp"
// #include "window.hpp"

std::mutex mtx;
std::vector<float> buffer;
bool new_buffer;

void data_callback(
  ma_device* pDevice, void* pOutput,
  const void* pInput, ma_uint32 frameCount)
{
  const float* data = (float*)pInput;

  (void)pOutput; // prevent unused variable warning
  std::lock_guard<std::mutex> lock(mtx); // prevent data concurrency
  std::copy(&data[0], &data[frameCount], buffer.begin());
  new_buffer = true;
}

Audio::Audio(ma_uint32 frame_count, ma_uint32 srate, double cutoff)
{
  buffer.resize(frame_count);
  res.raw.resize(frame_count);
  res.flt.resize(frame_count);

  new_buffer = false;

  std::fill(res.raw.begin(), res.raw.end(), 0.0f);

  ma_conf = ma_device_config_init(ma_device_type_capture);
  ma_conf.sampleRate = srate;
  ma_conf.periodSizeInFrames = frame_count;
  ma_conf.dataCallback = data_callback;
  ma_conf.capture.format = ma_format_f32;
  ma_conf.capture.channels = 1;

  ma_res = ma_device_init(nullptr, &ma_conf, &ma_dev);
  if (ma_res != MA_SUCCESS) exit_with_error();

  ma_res = ma_device_start(&ma_dev);
  if (ma_res != MA_SUCCESS) exit_with_error();

  setup_filter(srate, cutoff);
}

Audio::~Audio()
{
  ma_device_stop(&ma_dev);
  ma_device_uninit(&ma_dev);
}

void Audio::setup_filter(double fs, double fc)
{
  lpf.setup(fs, fc);
}

const Audio::Result *Audio::get_buffer()
{
  if (new_buffer) {
    const std::size_t buffer_size = res.raw.size();

    std::lock_guard<std::mutex> lock(mtx); // prevent data concurrency

    std::copy(buffer.begin(), buffer.end(), res.raw.begin());
    new_buffer = false;
    for (int i = 0; i < buffer_size; i++) {
      res.flt[i] = lpf.apply(res.raw[i]);
    }
  }

  return &res;
}

void Audio::exit_with_error()
{
  fprintf(stderr,
    "There has been an error with miniaudio.\n"
    "miniaudio error code: %d\n", ma_res
  );
  exit(EXIT_FAILURE);
}

#include "pa.hpp"

#include <stdio.h>
#include <stdlib.h>

pa_t::pa_t(int buffer_sz, int sample_rate)
  : buffer_sz(buffer_sz)
{
  data = new float[buffer_sz];

  err = Pa_Initialize();
  if (err != paNoError) exit_with_error();

  config.device = Pa_GetDefaultInputDevice();

  info = Pa_GetDeviceInfo(config.device);

  config.channelCount = 1;
  config.sampleFormat = paFloat32;
  config.suggestedLatency = info->defaultHighInputLatency;
  config.hostApiSpecificStreamInfo = NULL;

  printf("Device parameters:\n");
  printf("\tchannel count: %d\n", config.channelCount);
  printf(
    "\tsample format: %s\n",
    config.sampleFormat == paFloat32 ? "F32" : "unknown"
  );
  printf("\tsuggested latency: %f sec\n", config.suggestedLatency);

  printf("Opening device: %s\n", info->name);
  err = Pa_OpenStream(
    &stream,
    &config,
    NULL, // no output
    sample_rate,
    buffer_sz,
    paClipOff,
    NULL,
    NULL
  );
  if (err != paNoError) exit_with_error();

  err = Pa_StartStream(stream);
  if (err != paNoError) exit_with_error();
}

pa_t::~pa_t()
{
  delete[] data;

  err = Pa_StopStream(stream);
  if (err != paNoError) exit_with_error();

  err = Pa_Terminate();
  if (err != paNoError) exit_with_error();
}

float *pa_t::read_stream()
{
  err = Pa_ReadStream(stream, data, buffer_sz);
  if (err != paNoError) exit_with_error();

  return data;
}

void pa_t::exit_with_error()
{
  if (stream) {
    Pa_AbortStream(stream);
    Pa_CloseStream(stream);
  }

  fprintf(stderr, "An err occured while using the portaudio stream\n");
  fprintf(stderr, "err number: %d\n", err);
  fprintf(stderr, "err message: %s\n", Pa_GetErrorText(err));

  exit(EXIT_FAILURE);
}

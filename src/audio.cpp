#define MINIAUDIO_IMPLEMENTATION
#include "audio.hpp"

#include <cstdio>
#include <cstdlib>
#include <mutex>

// #include "ETFE.hpp"
// #include "window.hpp"

static std::mutex s_mtx;
static struct {
    std::vector<float> input;
    bool need_update;
} s_buffer;

static void data_callback(
    ma_device* pDevice, void* pOutput,
    const void* pInput, ma_uint32 frameCount
) {
    const float* data = (float*)pInput;

    (void)pOutput; // prevent unused variable warning
    std::lock_guard<std::mutex> lock(s_mtx); // prevent data concurrency
    std::copy(&data[0], &data[frameCount], s_buffer.input.begin());
    s_buffer.need_update = true;
}

Audio::Audio(ma_uint32 frame_count, ma_uint32 srate, double cutoff)
: m_res(m_etfe.getResult(), m_pitch.get_result()) {
    s_buffer.input.resize(frame_count);
    m_res.raw.resize(frame_count);
    m_res.flt.resize(frame_count);

    s_buffer.need_update = false;

    std::fill(m_res.raw.begin(), m_res.raw.end(), 0.0f);

    ma.conf = ma_device_config_init(ma_device_type_capture);
    ma.conf.sampleRate = srate;
    ma.conf.periodSizeInFrames = frame_count;
    ma.conf.dataCallback = data_callback;
    ma.conf.capture.format = ma_format_f32;
    ma.conf.capture.channels = 1;

    ma.res = ma_device_init(nullptr, &ma.conf, &ma.dev);
    if (ma.res != MA_SUCCESS) exit_with_error();

    ma.res = ma_device_start(&ma.dev);
    if (ma.res != MA_SUCCESS) exit_with_error();

    setup_filter(srate, cutoff);
}

Audio::~Audio() {
    ma_device_stop(&ma.dev);
    ma_device_uninit(&ma.dev);
}

void Audio::setup_filter(double fs, double fc) {
    m_lpf.setup(fs, fc);
}

void Audio::setup_etfe(
    std::size_t nsamples, double fs, const std::vector<double>& window,
    std::size_t noverlap, std::size_t nfft
) {
    m_etfe.setup(nsamples, fs, window, noverlap, nfft);
}

const Audio::Result& Audio::get_buffer() {
    if (s_buffer.need_update) {
        const std::size_t buffer_size = m_res.raw.size();

        std::lock_guard<std::mutex> lock(s_mtx); // prevent data concurrency

        std::copy(s_buffer.input.begin(), s_buffer.input.end(), m_res.raw.begin());
        s_buffer.need_update = false;
        for (int i = 0; i < buffer_size; i++) {
            m_res.flt[i] = m_lpf.apply(m_res.raw[i]);
        }

        m_etfe.estimate(m_res.flt.data());
        m_pitch.find(m_res.etfe.f[m_res.etfe.pidx], m_res.etfe.df);
    }

  return m_res;
}

void Audio::exit_with_error() {
    fprintf(stderr,
        "There has been an error with miniaudio.\n"
        "miniaudio error code: %d\n", ma.res
    );
    exit(EXIT_FAILURE);
}

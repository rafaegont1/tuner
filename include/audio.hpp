#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <vector>
#include "miniaudio.h"
#include "lpf.hpp"
#include "ETFE.hpp"
#include "PitchDetector.hpp"

class Audio {
public:
    struct Result {
        std::vector<float> raw; // raw audio frames
        std::vector<float> flt; // low-pass filtered audio frames
        const etfe::ETFE::Result& etfe;
        const PitchDetector::Result& pd;

        Result(
            const etfe::ETFE::Result& etfe, const PitchDetector::Result& pd
        ) : etfe(etfe), pd(pd) {
        }
    };

    Audio(ma_uint32 frame_count, ma_uint32 srate, double cutoff);
    virtual ~Audio();

    void setup_filter(double fs, double fc);
    void setup_etfe(
        std::size_t nsamples, double fs, const std::vector<double>& window,
        std::size_t noverlap, std::size_t nfft
    );
    const Result& get_buffer();

private:
    void exit_with_error();

    struct {
        ma_result res;
        ma_device dev;
        ma_device_config conf;
    } ma;
    Lpf m_lpf;
    etfe::ETFE m_etfe;
    PitchDetector m_pitch;
    Result m_res;
};

#endif // AUDIO_HPP

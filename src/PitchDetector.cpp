#include "PitchDetector.hpp"

#include <math.h>

PitchDetector::PitchDetector() {
    for (int i = 0; i < n; i++) {
        m_pitchtb[i] = (440.0 / 32.0) * std::pow(2.0, (i - 9.0) / 12.0);
    }
}

PitchDetector::~PitchDetector() {
}

const PitchDetector::Result& PitchDetector::find(
    const double f, const double accuracy
) {
    const int idx = binary_search(f, accuracy);

    m_res.f = m_pitchtb[idx];
    m_res.note = NOTES[idx%12];
    m_res.octave = idx / 12 - 1;
    m_res.cents = 1'200.0 * std::log2(f / m_res.f);
    m_res.in_tune = (std::abs(f - m_pitchtb[idx]) < accuracy);

    return m_res;
}

const PitchDetector::Result& PitchDetector::get_result() {
    return m_res;
}

int PitchDetector::binary_search(const double f, const double accuracy) {
    if (f < m_pitchtb[0]) {
        return 0;
    } else if (f > m_pitchtb[n - 1]) {
        return n - 1;
    }

    int idx = -1; // nearest pitch index
    int left = 0;
    int right = n - 1;

    while (left < right) {
        int mid = (left + right) / 2;

        if (std::abs(f - m_pitchtb[mid]) < accuracy) {
            // The target is in the middle
            idx = mid;
            break;
        } else if (f < m_pitchtb[mid]) {
            // Go left
            if (mid > 0 && f > m_pitchtb[mid - 1]) {
                // The target is in the left
                idx = get_closest(mid - 1, mid, f);
                break;
            }
            right = mid;
        } else {
            // Go right
            if (mid < n - 1 && f < m_pitchtb[mid + 1]) {
                // The target is in the right
                idx = get_closest(mid, mid + 1, f);
                break;
            }
            left = mid + 1;
        }
    }

    return idx;
}

inline int PitchDetector::get_closest(int lidx, int hidx, double target) {
    return (target - m_pitchtb[lidx] < m_pitchtb[hidx] - target) ? lidx : hidx;
}

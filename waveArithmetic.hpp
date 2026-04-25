#ifndef WAVE_ARITHMETIC
#define WAVE_ARITHMETIC

#include <cmath>
#include "enum.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline float generateWaveSample(waveform wave, float phase)
{
    // phase should be 0.0 to 1.0

    switch (wave)
    {
        case SINE:
            return std::sin(2.0f * static_cast<float>(M_PI) * phase);

        case SQUARE:
            return phase < 0.5f ? 1.0f : -1.0f;

        case SAW:
            return 2.0f * phase - 1.0f;

        case TRIANGLE:
            return 4.0f * std::fabs(phase - 0.5f) - 1.0f;

        default:
            return 0.0f;
    }
}

inline float wrapPhase(float phase)
{
    while (phase >= 1.0f) phase -= 1.0f;
    while (phase < 0.0f) phase += 1.0f;
    return phase;
}

#endif
#ifndef ENUM_HPP
#define ENUM_HPP
#include <string>
enum waveform {
    SINE,
    SAW,
    SQUARE,
    TRIANGLE
};

enum effect {
    NONE,
    VOLUME,
    DISTORTION,
    TREMOLO,
    LOW_PASS
};
struct note {
    std::string key;
    float frequency;
    float gain = 1.0;
    waveform wave;
    float phase = 0.0;
    bool releasing = false;
};
#endif

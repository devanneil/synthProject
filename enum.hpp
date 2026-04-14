#ifndef ENUM_HPP
#define ENUM_HPP
#include <string>
enum waveform {
    SINE,
    SAW,
    SQUARE,
    TRIANGLE
};
struct note {
    std::string key;
    float frequency;
    float gain = 1.0;
    waveform wave;
};
#endif
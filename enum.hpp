#ifndef ENUM_HPP
#define ENUM_HPP
enum waveform {
    SINE,
    SAW,
    SQUARE,
    TRIANGLE
};
struct note {
    char key;
    float frequency;
    float gain = 1.0;
    waveform wave;
};
#endif
#ifndef TONE_GENERATOR
#define TONE_GENERATOR
#define MA_NO_DECODING
#define MA_NO_ENCODING
#include <miniaudio.h>
#include <stdio.h>
#include <math.h>

// Callback function to fill the audio buffer
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_waveform* pSineWave = (ma_waveform*)pDevice->pUserData;
    ma_waveform_read_pcm_frames(pSineWave, pOutput, frameCount, NULL);
}

int sine_wave_test() {
    ma_device device;
    ma_device_config deviceConfig;
    ma_waveform sineWave;
    ma_waveform_config sineWaveConfig;

    // 1. Initialize the Waveform (440Hz Sine)
    sineWaveConfig = ma_waveform_config_init(ma_format_f32, 2, 48000, ma_waveform_type_sine, 0.2f, 440);
    ma_waveform_init(&sineWaveConfig, &sineWave);

    // 2. Configure and Start the Device
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate        = 48000;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &sineWave;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) return -1;
    ma_device_start(&device);

    printf("Press Enter to quit...");
    getchar();

    ma_device_uninit(&device);
    return 0;
}
#endif
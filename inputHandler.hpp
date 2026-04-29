#ifndef INPUT_HANDLER
#define INPUT_HANDLER
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <mutex>
#include "enum.hpp"
#include "waveArithmetic.hpp"
#include <miniaudio.h>
#include <stdio.h>
#include <math.h>

std::mutex audioMutex;
void initializeAudio();

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

struct keybind {
    std::string key;
    SDL_Scancode scancode;
    char letter;
    float frequency;
};
struct waveKey {
    std::string key;
    SDL_Scancode scancode;
    waveform wave;
};
keybind keybinds[128];
waveKey wavebinds[8];
int keybindLength = 0;
int waveKeyLength = 0;
note activeNotes[128];
int activeNoteCount = 0;
waveform currentWaveForm = SINE;
/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{   
    // Read in config file
    std::ifstream file("keyboard.conf");

    if (!file.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return SDL_APP_FAILURE;
    }

    int ind = 0;
    int waveInd = 0;
    std::string line;

    while (std::getline(file, line)) {

        // Skip blank or comment lines
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream tokenizer(line);

        std::vector<std::string> tokens;
        std::string tok;

        // Tokenize safely
        while (tokenizer >> tok) {
            tokens.push_back(tok);
        }

        try {

            // ---------- WAVE BINDS ----------
            // Format: KEY WAVE
            if (tokens.size() == 2) {

                const std::string& key  = tokens[0];
                const std::string& wave = tokens[1];

                wavebinds[waveInd].key = key;
                wavebinds[waveInd].scancode =
                    SDL_GetScancodeFromName(key.c_str());

                if (wave == "SINE")
                    wavebinds[waveInd].wave = SINE;
                else if (wave == "SAW")
                    wavebinds[waveInd].wave = SAW;
                else if (wave == "SQUARE")
                    wavebinds[waveInd].wave = SQUARE;
                else if (wave == "TRIANGLE")
                    wavebinds[waveInd].wave = TRIANGLE;
                else {
                    std::cerr << "Unknown wave type: "
                            << wave << std::endl;
                    return SDL_APP_FAILURE;
                }

                waveInd++;
            }

            // ---------- KEY BINDS ----------
            // Format: KEY LETTER FREQUENCY
            else if (tokens.size() == 3) {

                const std::string& letterStr = tokens[0];
                const std::string& key       = tokens[1];
                const std::string& freqStr   = tokens[2];

                keybinds[ind].key = key;
                keybinds[ind].letter = letterStr[0];
                keybinds[ind].frequency = std::stof(freqStr);

                keybinds[ind].scancode =
                    SDL_GetScancodeFromName(key.c_str());

                if (keybinds[ind].scancode ==
                    SDL_SCANCODE_UNKNOWN) {

                    std::cerr << "Invalid key name: "
                            << key << std::endl;

                    return SDL_APP_FAILURE;
                }

                ind++;
            }

            // ---------- INVALID LINE ----------
            else {

                std::cerr
                    << "Invalid config line format:\n"
                    << line << std::endl;

                return SDL_APP_FAILURE;
            }

        }
        catch (const std::exception& e) {

            std::cerr
                << "Invalid keyboard configuration file!"
                << std::endl;

            std::cerr << e.what() << std::endl;
            std::cerr << "Line: " << line << std::endl;

            file.close();
            return SDL_APP_FAILURE;
        }
    }

    keybindLength = ind;
    waveKeyLength = waveInd;
    file.close();
    initializeAudio();
    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Hello World", 800, 600, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_KEY_DOWN)
    {
        SDL_KeyboardEvent *key = (SDL_KeyboardEvent *)event;

        if (key->key == SDLK_ESCAPE)
        {
            return SDL_APP_SUCCESS;  /* only quit on ESC */
        }
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{   
    /* Clear screen */
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    int w = 0, h = 0;
    float x, y;
    const float scale = 4.0f;

    SDL_GetRenderOutputSize(renderer, &w, &h);
    SDL_SetRenderScale(renderer, scale, scale);

    /* Starting position */
    x = 10;
    y = 10;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, x, y, "These are the configured keys: ");
    y+= 10;
    for (int i = 0; i < keybindLength; i++) {

        char line[64];

        snprintf(
            line,
            sizeof(line),
            "%s %c %.2f",
            keybinds[i].key.c_str(),
            keybinds[i].letter,
            keybinds[i].frequency
        );

        SDL_RenderDebugText(renderer, x, y, line);

        /* Move down one line */
        y += SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE + 2;
    }
    for (int i = 0; i < waveKeyLength; i++) {
        char line[64];
        std::string currentWave;
        switch (wavebinds[i].wave)
        {
        case SINE:
            currentWave = "SINE";
            break;
        case SAW:
            currentWave = "SAW";
            break;
        case SQUARE:
            currentWave = "SQUARE";
            break;
        case TRIANGLE:
            currentWave = "TRIANGLE";
            break;
        default:
            currentWave = "UNCONFIGURED";
            break;
        }

        snprintf(
            line,
            sizeof(line),
            "%s %s",
            wavebinds[i].key.c_str(),
            currentWave.c_str()
        );

        SDL_RenderDebugText(renderer, x, y, line);

        /* Move down one line */
        y += SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE + 2;
    }
    const bool *keys =
    (const bool *)SDL_GetKeyboardState(NULL);

    int yOffset = 10;
    for (int i = 0; i < waveKeyLength; i++) {
        if (keys[wavebinds[i].scancode]) {
            currentWaveForm = wavebinds[i].wave;
        }
    }

    {
        std::lock_guard<std::mutex> lock(audioMutex);
        for (int i = 0; i < activeNoteCount; i++) {
            activeNotes[i].releasing = true;
        }

        for (int i = 0; i < keybindLength; i++) {
            std::string activeLetters;
            if (keys[keybinds[i].scancode]) {
                int noteIndex = -1;
                for (int n = 0; n < activeNoteCount; n++) {
                    if (activeNotes[n].key == keybinds[i].key) {
                        noteIndex = n;
                        break;
                    }
                }

                if (noteIndex == -1 && activeNoteCount < 128) {
                    noteIndex = activeNoteCount;
                    activeNotes[noteIndex].frequency = keybinds[i].frequency;
                    activeNotes[noteIndex].gain = 1.0f;
                    activeNotes[noteIndex].key = keybinds[i].key;
                    activeNotes[noteIndex].wave = currentWaveForm;
                    activeNotes[noteIndex].phase = 0.0f;
                    activeNoteCount++;
                }

                if (noteIndex != -1) {
                    activeNotes[noteIndex].frequency = keybinds[i].frequency;
                    activeNotes[noteIndex].releasing = false;
                }

                activeLetters += keybinds[i].key;
                activeLetters += ' ';
                SDL_RenderDebugText(renderer, 280, yOffset, activeLetters.c_str());
                yOffset += 10;
            }
        }
    }
    for (int i = 0; i < waveKeyLength; i++) {
        std::string currentWave;
        switch (currentWaveForm)
        {
        case SINE:
            currentWave = "SINE";
            break;
        case SAW:
            currentWave = "SAW";
            break;
        case SQUARE:
            currentWave = "SQUARE";
            break;
        case TRIANGLE:
            currentWave = "TRIANGLE";
            break;
        default:
            currentWave = "UNCONFIGURED";
            break;
        }
        SDL_RenderDebugText(renderer, 280, yOffset, currentWave.c_str());
    }
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}


#define SAMPLE_RATE 48000
#define RELEASE_SECONDS 0.01f

ma_device device;
bool audioInitialized = false;

void data_callback(
    ma_device* device,
    void* output,
    const void* input,
    ma_uint32 frameCount)
{
    float* out = (float*)output;
    std::lock_guard<std::mutex> lock(audioMutex);

    for (ma_uint32 i = 0; i < frameCount; i++) {

        float sample = 0.0f;
        for (int n = 0; n < activeNoteCount; n++) {
            sample += generateWaveSample(activeNotes[n].wave, 
                activeNotes[n].phase) * activeNotes[n].gain;
            activeNotes[n].phase = wrapPhase(
                activeNotes[n].phase + activeNotes[n].frequency / SAMPLE_RATE);
            if (activeNotes[n].releasing) {
                activeNotes[n].gain -= 1.0f / (RELEASE_SECONDS * SAMPLE_RATE);
                if (activeNotes[n].gain < 0.0f) {
                    activeNotes[n].gain = 0.0f;
                }
            } else {
                activeNotes[n].gain = 1.0f;
            }
        }
        out[i] = sample * 0.2f;
    }

    int writeIndex = 0;
    for (int readIndex = 0; readIndex < activeNoteCount; readIndex++) {
        if (!activeNotes[readIndex].releasing || activeNotes[readIndex].gain > 0.0f) {
            if (writeIndex != readIndex) {
                activeNotes[writeIndex] = activeNotes[readIndex];
            }
            writeIndex++;
        }
    }
    activeNoteCount = writeIndex;
}

void initializeAudio() {
    if (audioInitialized)
        return;

    ma_device_config config;

    config = ma_device_config_init(
        ma_device_type_playback);

    config.playback.format   = ma_format_f32;
    config.playback.channels = 1;
    config.sampleRate        = SAMPLE_RATE;
    config.dataCallback      = data_callback;

    if (ma_device_init(
            NULL,
            &config,
            &device) != MA_SUCCESS)
    {
        printf("Failed to open audio device.\n");
        return;
    }

    if (ma_device_start(&device)
            != MA_SUCCESS)
    {
        printf("Failed to start playback.\n");
        ma_device_uninit(&device);
        return;
    }

    audioInitialized = true;
}
#endif

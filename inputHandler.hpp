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
keybind keybinds[128];
int keybindLength = 0;
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

    // Read file line by line and fill keybinds array
    int ind = 0;
    std::string line;
    // Ignore first line
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::istringstream tokenizer(line);
        std::string conf[3];

        std::getline(tokenizer, conf[0], ' '); // Key
        std::getline(tokenizer, conf[1], ' '); // Letter
        std::getline(tokenizer, conf[2]); // Frequency

        if(tokenizer) {
            try {
                keybinds[ind].key = conf[0];
                keybinds[ind].letter = conf[1][0];
                keybinds[ind].frequency = std::stof(conf[2]);

                keybinds[ind].scancode =
                    SDL_GetScancodeFromName(conf[1].c_str());
                if (keybinds[ind].scancode == SDL_SCANCODE_UNKNOWN) {
                    std::cerr << "Invalid key name: "
                            << keybinds[ind].key << std::endl;
                    return SDL_APP_FAILURE;
                }
            } catch (const std::exception& e) {
                std::cerr << "Invalid keyboard configuration file!" << std::endl;
                std::cerr << e.what() << std::endl;
                std::cerr << "At: " << conf[2] << std::endl;
                file.close();
                return SDL_APP_FAILURE;
                break;
            }
        } else {
            std::cerr << "Keyboard configuration file error at: " << ind << std::endl;
        }
        ind++;
    }
    keybindLength = ind;
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
    const bool *keys =
    (const bool *)SDL_GetKeyboardState(NULL);

    int yOffset = 10;
    activeNoteCount = 0;
    for (int i = 0; i < keybindLength; i++) {
        std::string activeLetters;
        if (keys[keybinds[i].scancode]) {
            audioMutex.lock();
            activeNotes[activeNoteCount].frequency = keybinds[i].frequency;
            activeNotes[activeNoteCount].gain = 1;
            activeNotes[activeNoteCount].key = keybinds[i].key;
            activeNotes[activeNoteCount].wave = currentWaveForm;
            activeNoteCount++;
            audioMutex.unlock();

            activeLetters += keybinds[i].key;
            activeLetters += ' ';
            SDL_RenderDebugText(renderer, 280, yOffset, activeLetters.c_str());
            yOffset += 10;
        }
    }
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}


#define SAMPLE_RATE 48000

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
                activeNotes[n].phase);
            activeNotes[n].phase += activeNotes[n].frequency / SAMPLE_RATE;
        }
        out[i] = sample * 0.2f;
    }
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
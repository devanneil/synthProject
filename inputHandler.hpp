#ifndef INPUT_HANDLER
#define INPUT_HANDLER
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
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
    /* Get current keyboard state (IMPORTANT: enables multi-key detection) */
    const bool *keys = (const bool *)SDL_GetKeyboardState(NULL);

    bool wKey = keys[SDL_SCANCODE_W];
    bool aKey = keys[SDL_SCANCODE_A];
    bool sKey = keys[SDL_SCANCODE_S];
    bool dKey = keys[SDL_SCANCODE_D];
    bool space = keys[SDL_SCANCODE_SPACE];

    /* Clear screen */
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    /* Title */
    SDL_RenderDebugText(renderer, 20, 20, "SDL3 Multi-Key Input Demo");

    /* Instructions */
    SDL_RenderDebugText(renderer, 20, 60, "Hold keys to see state:");

    SDL_RenderDebugText(renderer, 20, 100, "W - Forward");
    SDL_RenderDebugText(renderer, 20, 120, "A - Left");
    SDL_RenderDebugText(renderer, 20, 140, "S - Back");
    SDL_RenderDebugText(renderer, 20, 160, "D - Right");
    SDL_RenderDebugText(renderer, 20, 180, "SPACE - Jump");

     /* Live state display */
    char buffer[128];
    SDL_snprintf(buffer, sizeof(buffer),
                 "STATE -> W:%d A:%d S:%d D:%d SPACE:%d",
                 wKey, aKey, sKey, dKey, space);


    const char *message = buffer;
    int w = 0, h = 0;
    float x, y;
    const float scale = 4.0f;

    /* Center the message and scale it up */
    SDL_GetRenderOutputSize(renderer, &w, &h);
    SDL_SetRenderScale(renderer, scale, scale);
    x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
    y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

    /* Draw the message */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, x, y, message);
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}
#endif
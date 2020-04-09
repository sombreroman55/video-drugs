#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>

#define SCR_WIDTH  256
#define SCR_HEIGHT 256

int init();
void close();

SDL_Window* window = NULL;
SDL_Surface* screen_surface = NULL;

uint32_t pallette[16] = {0};
uint8_t canvas[256][256] = {0};

int main(int argc, char** argv)
{
    printf("Hi\n");
    if (!init())
    {
        printf("Failed to initialize!\n");
    }
    else
    {
        printf("Hello\n");
    }
    while(1) {}

    close();
    return 0;
}

int init()
{
    // Initialization flag
    int success = 1;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = 0;
    }
    else
    {
        window = SDL_CreateWindow("Video Drugs", 
                                  SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED, 
                                  SCR_WIDTH, SCR_HEIGHT, 
                                  (  SDL_WINDOW_SHOWN
                                   // | SDL_WINDOW_BORDERLESS 
                                   | SDL_WINDOW_RESIZABLE 
                                   | SDL_WINDOW_INPUT_FOCUS
                                   | SDL_WINDOW_ALWAYS_ON_TOP));
        if (window == NULL)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = 0;
        }
        else
        {
            // Get window surface
            screen_surface = SDL_GetWindowSurface(window);
        }
    }
    return success;
}

void close()
{
    // Destroy window
    SDL_DestroyWindow(window);
    window = NULL;

    // Quit SDL
    SDL_Quit();
}

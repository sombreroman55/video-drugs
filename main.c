#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include "images.h"

#define SCR_HEIGHT 256
#define SCR_WIDTH  256

typedef enum
{
    NONE                    = 0,
    PALETTE_CYCLING         = 1 << 0,
    BACKGROUND_SCROLLING    = 1 << 1,
    HORIZONTAL_OSCILLATION  = 1 << 2,
    INTERLEAVED_OSCILLATION = 1 << 3,
    VERTICAL_OSCILLATION    = 1 << 4,
    TRANSPARENCY            = 1 << 5,
} Effect;

void display_pixel_data();

uint32_t palette[16] = {0x051E3E, 0x251E3E, 0x451E3E, 0x651E3E, 
                        0x851E3E, 0xFF3377, 0xFF5588, 0xFFBBEE,
                        0xFF99CC, 0xFF77AA, 0xFF8B94, 0xFFAAA5,
                        0xFFD3B6, 0xDCEDC1, 0xA8E6CF, 0x88D8B0};

uint8_t pixel_data[SCR_HEIGHT][SCR_WIDTH] = {0};

int main(int argc, char** argv)
{
    // vertical_stripes((uint8_t*)pixel_data, SCR_HEIGHT, SCR_WIDTH);
    // horizontal_stripes((uint8_t*)pixel_data, SCR_HEIGHT, SCR_WIDTH);
    concentric_squares((uint8_t*)pixel_data, SCR_HEIGHT, SCR_WIDTH);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Video Drugs", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SCR_WIDTH*2, SCR_HEIGHT*2, 
                                          SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             SCR_WIDTH,
                                             SCR_HEIGHT);
    int interrupted = 0;
    int frame_counter = 0;
    int offset = 0;
    Effect effects = PALETTE_CYCLING;
    while (!interrupted)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    switch (ev.key.keysym.sym)
                    {
                        case 'q': 
                            interrupted = 1;
                            break;
                    }
                    break;
            }
        }
        uint32_t pixels[SCR_WIDTH*SCR_HEIGHT] = {0};
        int i, j;
        for (i = 0; i < SCR_HEIGHT; i++)
        {
            for (j = 0; j < SCR_WIDTH; j++)
            {
                int palette_index = pixel_data[i][j];
                if (effects & PALETTE_CYCLING)
                    palette_index = (palette_index + offset) % 16;

                pixels[i*SCR_HEIGHT+j] = palette[palette_index];
            }
        }

        SDL_UpdateTexture(texture, NULL, pixels, 4*SCR_WIDTH);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000/60);
        frame_counter = (frame_counter + 1) % 4;
        if (frame_counter == 0)
            offset = (offset + 1) % 16;
    }

    SDL_DestroyWindow(window);
    window = NULL;
    SDL_Quit();
    return 0;
}

void display_pixel_data()
{
    int i, j;
    for (i = 0; i < SCR_HEIGHT; i++)
    {
        for (j = 0; j < SCR_WIDTH; j++)
        {
            printf("%2d", pixel_data[i][j]);
        }
        printf("\n");
    }
}

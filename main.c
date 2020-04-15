#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include "images.h"

#define SCR_HEIGHT 256
#define SCR_WIDTH  256
#define LINE_TO_DEG 360 / 256

const double PI = atan(1) / 4;

int offset_from_angle (double degrees, double shift);

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
    int frame_speed = 4;
    int p_offset = 0;
    int h_sine_offset = 0;
    int v_sine_offset = 0;
    double sine_shift = 0;
    int h_scroll_offset = 0;
    int v_scroll_offset = 0;
    Effect effects =  /* PALETTE_CYCLING | */
                      /* BACKGROUND_SCROLLING | */
                         HORIZONTAL_OSCILLATION |
                      /* INTERLEAVED_OSCILLATION | */
                      /* VERTICAL_OSCILLATION | */ 
                      /* TRANSPARENCY | */ 
                         NONE;
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
                if (effects & HORIZONTAL_OSCILLATION)
                {
                    h_sine_offset = offset_from_angle(i * LINE_TO_DEG, sine_shift);
                }
                if (effects & VERTICAL_OSCILLATION)
                {
                    v_sine_offset = offset_from_angle(i * LINE_TO_DEG, sine_shift);
                }
                int y = i + v_sine_offset + v_scroll_offset; 
                if (y < 0) y += SCR_HEIGHT; y %= SCR_HEIGHT;
                int x = j + h_sine_offset + h_scroll_offset; 
                if (x < 0) x += SCR_WIDTH; x %= SCR_WIDTH;
                int palette_index = pixel_data[y][x];
                if (effects & PALETTE_CYCLING)
                    palette_index = (palette_index + p_offset) % 16;
                pixels[i*SCR_HEIGHT+j] = palette[palette_index];
            }
        }

        SDL_UpdateTexture(texture, NULL, pixels, 4*SCR_WIDTH);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000/250);
        frame_counter = (frame_counter + 1) % 6;
        sine_shift += 0.02; if (sine_shift >= 360) sine_shift -= 360;
        if (frame_counter == 0)
        {
            p_offset = (p_offset + 1) % 16;
        }
        // --v_scroll_offset;
        // v_scroll_offset += sine_push; v_scroll_offset %= SCR_HEIGHT;
        // if (v_scroll_offset < 0) v_scroll_offset += SCR_HEIGHT;
        if ((effects & BACKGROUND_SCROLLING) && frame_counter == 0)
        {
            v_scroll_offset += 4; v_scroll_offset %= SCR_HEIGHT;
            h_scroll_offset += 1; h_scroll_offset %= SCR_WIDTH;
        }
    }

    SDL_DestroyWindow(window);
    window = NULL;
    SDL_Quit();
    return 0;
}

int offset_from_angle (double degrees, double shift)
{
    double rad = (degrees * PI) / 180.0;
    return (int)(25*sin(25*rad+shift));
}

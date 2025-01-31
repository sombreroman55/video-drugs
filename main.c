#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "images.h"
#include "palettes.h"
#include "renderer.h"

#define SCR_HEIGHT 256
#define SCR_WIDTH  256
#define LINE_TO_DEG 360 / 256
#define PI (atan(1)/4)

double sine_table[256];

int offset_from_angle (double degrees, double amplitude, double period, double shift);

typedef struct
{
    uint8_t pval;
    uint8_t fixed;
} Pixel;

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

uint32_t* palette = something;
size_t palette_size;

uint8_t pixel_data[SCR_HEIGHT][SCR_WIDTH] = {0};

int main(int argc, char** argv)
{
    palette_size = palette[0];
    // vertical_stripes((uint8_t*)pixel_data, SCR_HEIGHT, SCR_WIDTH);
    //vertical_stripes2((uint8_t*)pixel_data, SCR_HEIGHT, SCR_WIDTH);
    // horizontal_stripes((uint8_t*)pixel_data, SCR_HEIGHT, SCR_WIDTH);
    concentric_squares((uint8_t*)pixel_data, SCR_HEIGHT, SCR_WIDTH);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Video Drugs", SCR_WIDTH*2, SCR_HEIGHT*2, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             SCR_WIDTH,
                                             SCR_HEIGHT);
    int interrupted = 0;
    int frame_counter = 0;
    int p_offset = 0;

    int h_scroll_counter = 0;
    int v_scroll_counter = 0;
    int h_scroll_rate = 1;
    int v_scroll_rate = 1;
    int h_scroll_offset = 0;
    int v_scroll_offset = 0;

    int h_sine_offset = 0;
    int v_sine_offset = 0;
    double sine_shift = 0;
    Effect effects =  PALETTE_CYCLING |
                      // BACKGROUND_SCROLLING |
                      HORIZONTAL_OSCILLATION |
                      // INTERLEAVED_OSCILLATION |
                      VERTICAL_OSCILLATION |
                      // TRANSPARENCY |
                         NONE;
    while (!interrupted)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                    switch (ev.key.key)
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
                    h_sine_offset = offset_from_angle(i * LINE_TO_DEG, 24, 16, sine_shift);
                }
                if (effects & INTERLEAVED_OSCILLATION)
                {
                    int off = offset_from_angle(i * LINE_TO_DEG, 32, 32, sine_shift);
                    if (i % 2 == 0)
                        h_sine_offset = off;
                    else
                        h_sine_offset = off * -1;
                }
                if (effects & VERTICAL_OSCILLATION)
                {
                    v_sine_offset = offset_from_angle(i * LINE_TO_DEG, 24, 16, sine_shift);
                }
                int y = i + v_sine_offset + v_scroll_offset;
                while (y < 0) y += SCR_HEIGHT; y %= SCR_HEIGHT;
                int x = j + h_sine_offset + h_scroll_offset;
                while (x < 0) x += SCR_WIDTH; x %= SCR_WIDTH;
                int palette_index = pixel_data[y][x];
                if (effects & PALETTE_CYCLING)
                    palette_index = (palette_index + p_offset) % palette_size;
                pixels[i*SCR_HEIGHT+j] = palette[palette_index+1] | 0xFF000000;
            }
        }

        SDL_UpdateTexture(texture, NULL, pixels, 4*SCR_WIDTH);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000/60);
        frame_counter = (frame_counter + 1) % 20;
        sine_shift += 0.03; if (sine_shift >= 360) sine_shift -= 360;
        if (frame_counter == 0)
        {
            p_offset = (p_offset + 1) % palette_size;
        }

        if ((effects & BACKGROUND_SCROLLING))
        {
            v_scroll_counter = (v_scroll_counter + 1) % v_scroll_rate;
            h_scroll_counter = (h_scroll_counter + 1) % h_scroll_rate;
            if (v_scroll_counter == 0)
            {
                v_scroll_offset += 1;
                v_scroll_offset %= SCR_HEIGHT;
            }

            if (h_scroll_counter == 0)
            {
                h_scroll_offset += 1;
                h_scroll_offset %= SCR_WIDTH;
            }
        }
    }

    SDL_DestroyWindow(window);
    window = NULL;
    SDL_Quit();
    return 0;
}

int offset_from_angle (double degrees, double amplitude, double period, double shift)
{
    double rad = (degrees * PI) / 180.0;
    return (int)(amplitude*sin(period*rad+shift));
}

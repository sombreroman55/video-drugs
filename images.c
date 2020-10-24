/* images.c
 *
 * Creates different, predefined images through the
 * filling of pixel data arrays with palette indices
 */

#include "images.h"

void vertical_stripes(uint8_t* pixel_data, int h, int w)
{
    int i, j;
    for (i = 0; i < h; i++)
        for (j = 0; j < w; j++)
            pixel_data[i*h+j] = i/palette_size;
}

void horizontal_stripes(uint8_t* pixel_data, int h, int w)
{
    int i, j;
    for (j = 0; j < w; j++)
        for (i = 0; i < h; i++)
            pixel_data[i*h+j] = j/palette_size;
}

void concentric_squares(uint8_t* pixel_data, int h, int w)
{
    int chunk = palette_size * 2;
    int i, j, k, h_off = 0, w_off = 0, h_inc = h/chunk, w_inc = w/chunk;
    for (k = 0; k < palette_size; k++)
    {
        for (i = h_off; i < h-h_off; i++)
        {
            for (j = w_off; j < w-w_off; j++)
            {
                pixel_data[i*h+j] = k;
            }
        }
        h_off += h_inc;
        w_off += w_inc;
    }
}

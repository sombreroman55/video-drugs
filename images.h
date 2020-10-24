/* images.h
 *
 * Contains function declarations for images.c functions
 */
#ifndef _IMAGES_H_
#define _IMAGES_H_

#include <stddef.h>

#include <stdint.h>

extern size_t palette_size;

void vertical_stripes(uint8_t* pixel_data, int h, int w);
void horizontal_stripes(uint8_t* pixel_data, int h, int w);
void concentric_squares(uint8_t* pixel_data, int h, int w);
void concentric_circles(uint8_t* pixel_data, int h, int w);
void triangles(uint8_t* pixel_data, int h, int w);

#endif /* _IMAGES_H_ */

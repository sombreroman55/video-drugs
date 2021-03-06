/* images.h
 *
 * Contains function declarations for images.c functions
 */
#include <stdint.h>

void vertical_stripes(uint8_t* pixel_data, int h, int w);
void horizontal_stripes(uint8_t* pixel_data, int h, int w);
void concentric_squares(uint8_t* pixel_data, int h, int w);
void concentric_circles(uint8_t* pixel_data, int h, int w);
void triangles(uint8_t* pixel_data, int h, int w);

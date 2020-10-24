#include <math.h>
#define LINE_TO_DEG 360 / 256

static const double PI = atan(1) / 4;

static int offset_from_angle (double degrees, double amplitude, double period, double shift);
static int c_offset_from_angle (double degrees, double amplitude, double period, double shift);

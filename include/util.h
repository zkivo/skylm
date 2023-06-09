#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

/*
 * Get the human-readable string from a DRM connector type.
 * This is compatible with Weston's connector naming.
 */
const char *conn_str(uint32_t conn_type);

/*
 * Calculate an accurate refresh rate from 'mode'.
 * The result is in mHz.
 */
int refresh_rate(drmModeModeInfo *mode);

int get_dpi(int mm_width, int num_pixels);

int get_num_pixel(int dpi, float mm_height);

void convert_RGBA_to_ARGB(unsigned char* img, int num_bytes);
void convert_RGB_to_ARGB(unsigned char* img, unsigned char* out, int num_bytes);

#endif

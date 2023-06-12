#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <stdint.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include "font_util.h"

struct dumb_framebuffer {
	uint32_t id;     // DRM object ID
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t handle; // driver-specific handle
	uint64_t size;   // size of mapping

	uint8_t *data;   // mmapped data we can write to
};

struct connector {
	uint32_t id;
	char name[16];
	bool connected;

	drmModeCrtc *saved;

	uint32_t crtc_id;
	drmModeModeInfo mode;

	uint32_t width;
	uint32_t height;
	uint32_t rate;

	uint32_t dpi;

	FontUtil font_util;

	struct dumb_framebuffer fb;

	struct connector *next;
};

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

// x_pos, y_pos are number between 0-1
// out_img is a ARGB little endian framebuffer
// width and height are the resolution of the frame buffer
void add_text_to_framebuffer(float x_pos, float y_pos, std::string text,
							 unsigned char* out_img, int out_width, int out_height, FontUtil& font_util);

void convert_RGBA_to_ARGB(unsigned char* img, int num_bytes);
//void convert_RGB_to_ARGB(unsigned char* img, unsigned char* out, int num_bytes);

#endif

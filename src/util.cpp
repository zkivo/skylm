#include <string>

#include "font_util.h"
#include "util.h"

#include <stdint.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

/*
 * Get the human-readable string from a DRM connector type.
 * This is compatible with Weston's connector naming.
 */
const char *conn_str(uint32_t conn_type)
{
	switch (conn_type) {
	case DRM_MODE_CONNECTOR_Unknown:     return "Unknown";
	case DRM_MODE_CONNECTOR_VGA:         return "VGA";
	case DRM_MODE_CONNECTOR_DVII:        return "DVI-I";
	case DRM_MODE_CONNECTOR_DVID:        return "DVI-D";
	case DRM_MODE_CONNECTOR_DVIA:        return "DVI-A";
	case DRM_MODE_CONNECTOR_Composite:   return "Composite";
	case DRM_MODE_CONNECTOR_SVIDEO:      return "SVIDEO";
	case DRM_MODE_CONNECTOR_LVDS:        return "LVDS";
	case DRM_MODE_CONNECTOR_Component:   return "Component";
	case DRM_MODE_CONNECTOR_9PinDIN:     return "DIN";
	case DRM_MODE_CONNECTOR_DisplayPort: return "DP";
	case DRM_MODE_CONNECTOR_HDMIA:       return "HDMI-A";
	case DRM_MODE_CONNECTOR_HDMIB:       return "HDMI-B";
	case DRM_MODE_CONNECTOR_TV:          return "TV";
	case DRM_MODE_CONNECTOR_eDP:         return "eDP";
	case DRM_MODE_CONNECTOR_VIRTUAL:     return "Virtual";
	case DRM_MODE_CONNECTOR_DSI:         return "DSI";
	default:                             return "Unknown";
	}
}

int get_dpi(int mm_width, int num_pixels) {
	//supposing dpi is equal between height and width we use one function
	float inch_width = mm_width * 0.03937008;
	return (int)(num_pixels / inch_width); 
}

int get_num_pixel(int dpi, float mm_height) {
	float inch_height = mm_height * 0.03937008;
	return (int)(dpi * inch_height);
}

void convert_RGBA_to_ARGB(unsigned char* img, int num_bytes) {
	// RGBA is big endian encoded so R first (stb image library)
	// ARGB is little endiand encord so B first (drm library)
	unsigned char t_r, t_g, t_b;
	for (int i = 0; i < num_bytes; i += 4) {
		t_r = img[i];
		t_g = img[i+1];
		t_b = img[i+2];
		//t_a = img[i+3];
		img[i]   = t_b;
		img[i+1] = t_g;
		img[i+2] = t_r;
		//img[i+3] = t_b;
	}
}

void add_text_to_framebuffer(float x_pos, float y_pos, std::string text,
		unsigned char* out_img, int width, int height, FontUtil& font_util) {
	int x = width * x_pos;
	int y = height * y_pos;
	int spacing = 5; // value in pixel
	unsigned char* background_value;
	unsigned char* foreground_value;
	//unsigned int* fb = (unsigned int*)buffer;
	int index_out_img = (y * width * 4) + (x * 4);
	for (int c = 0; c < text.length(); c++) {
		unsigned char* buff = font_util.getARGBBitmapCharcode(text[c]);
		for (int i = 0; i < font_util.height; i++) {
			for (int j = 0; i < font_util.width; j++) {
				int index_inside_img = (i * font_util.width * 4) + j * 4;
				for (int k = 0; k < 4; k++) {
					background_value = &out_img[index_out_img + j * font_util.width * 4 + k];
					foreground_value = &buff[index_inside_img + k];
					if ((int)(*background_value + *foreground_value) > (int) 255) {
						*background_value = 255;
					}
				}
			}
		}
		index_out_img += font_util.width * 4 + spacing * 4;
	}

}

/*
 * Calculate an accurate refresh rate from 'mode'.
 * The result is in mHz.
 */
int refresh_rate(drmModeModeInfo *mode)
{
	int res = (mode->clock * 1000000LL / mode->htotal + mode->vtotal / 2) / mode->vtotal;

	if (mode->flags & DRM_MODE_FLAG_INTERLACE)
		res *= 2;

	if (mode->flags & DRM_MODE_FLAG_DBLSCAN)
		res /= 2;

	if (mode->vscan > 1)
		res /= mode->vscan;

	return res;
}

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

void convert_RGBA_to_ARGB(unsigned char* img, int num_bytes) {
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

void convert_RGB_to_ARGB(unsigned char* img, unsigned char* out, int num_bytes) {
	unsigned char t_r, t_g, t_b;
	int temp = 0;
	for (int i = 0; i < num_bytes; i += 3, temp++) {
		t_r = img[i];
		t_g = img[i+1];
		t_b = img[i+2];
		out[i + temp]   = 255;
		out[i+1 + temp] = t_r;
		out[i+2 + temp] = t_g;
		out[i+3 + temp] = t_b;
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

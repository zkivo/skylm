#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <cstdio>
#include <cinttypes>
#include <sys/mman.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <fcntl.h>
#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#include "ppm.h"
#include "font_util.h"
#include "util.h"

static uint32_t find_crtc(int drm_fd, drmModeRes *res, drmModeConnector *conn,
		uint32_t *taken_crtcs)
{
	for (int i = 0; i < conn->count_encoders; ++i) {
		drmModeEncoder *enc = drmModeGetEncoder(drm_fd, conn->encoders[i]);
		if (!enc)
			continue;

		for (int i = 0; i < res->count_crtcs; ++i) {
			uint32_t bit = 1 << i;
			// Not compatible
			if ((enc->possible_crtcs & bit) == 0)
				continue;

			// Already taken
			if (*taken_crtcs & bit)
				continue;

			drmModeFreeEncoder(enc);
			*taken_crtcs |= bit;
			return res->crtcs[i];
		}

		drmModeFreeEncoder(enc);
	}

	return 0;
}

bool create_fb(int drm_fd, uint32_t width, uint32_t height, struct dumb_framebuffer *fb)
{
	int ret;
	struct drm_mode_map_dumb map;
	
	struct drm_mode_create_dumb create = {
		.height = height,
		.width = width,
		.bpp = 32,
	};

	ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
	if (ret < 0) {
		perror("DRM_IOCTL_MODE_CREATE_DUMB");
		return false;
	}

	fb->height = height;
	fb->width = width;
	fb->stride = create.pitch;
	fb->handle = create.handle;
	fb->size = create.size;

	uint32_t handles[4] = { fb->handle };
	uint32_t strides[4] = { fb->stride };
	uint32_t offsets[4] = { 0 };

	ret = drmModeAddFB2(drm_fd, width, height, DRM_FORMAT_ARGB8888, //not every format is supported by gpus. Use with confidence ARGB and XRGB (the X means the first 8 bit are ignored).
		handles, strides, offsets, &fb->id, 0);
	if (ret < 0) {
		perror("drmModeAddFB2");
		goto error_dumb;
	}

	map = { .handle = fb->handle };
	ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
	if (ret < 0) {
		perror("DRM_IOCTL_MODE_MAP_DUMB");
		goto error_fb;
	}

	fb->data = (uint8_t *)mmap(0, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED,
		drm_fd, map.offset);
	if (!fb->data) {
		perror("mmap");
		goto error_fb;
	}

	memset(fb->data, 0xff, fb->size);

	return true;

error_fb:
	drmModeRmFB(drm_fd, fb->id);
error_dumb:
	;
	struct drm_mode_destroy_dumb destroy = { .handle = fb->handle };
	drmIoctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
	return false;
}

int main(void)
{

	//-------------------------------
	// 		TESTING FONT UTIL
	//-------------------------------
	// std::cout << "test..." << std::endl; 
	// FontUtil font_util(108);
	// if (font_util.error) {
	// 	std::cout << "error " << font_util.error << std::endl; 
	// 	return 1;
	// }
	// unsigned char* buffer;
	// buffer = font_util.getARGBBitmapCharcode('A');
	// PPM ppm(font_util.width, font_util.height);
	// for (int i = 0; i < font_util.height * font_util.width * 4; i+=4) {
    //     int p = i / 4;
	// 	ppm.set_pixel(p % font_util.width, p / font_util.width, buffer[i + 2], buffer[i + 1], buffer[i]);
    // }
	// ppm.write_file("output.ppm");
	// return 0;
	//-------------------------------
	int ret;

	/* We just take the first GPU that exists. */
	int drm_fd = open("/dev/dri/card0", O_RDWR | O_NONBLOCK);
	if (drm_fd < 0) {
		perror("/dev/dri/card0");
		return 1;
	}

	drmModeRes *res = drmModeGetResources(drm_fd);
	if (!res) {
		perror("drmModeGetResources");
		return 1;
	}

	struct connector *conn_list = NULL;
	uint32_t taken_crtcs = 0;

	for (int i = 0; i < res->count_connectors; ++i) {
		drmModeConnector *drm_conn = drmModeGetConnector(drm_fd, res->connectors[i]);
		if (!drm_conn) {
			perror("drmModeGetConnector");
			continue;
		}

		struct connector *conn = (connector*)malloc(sizeof(connector));
		//struct connector* conn = new connector();
		if (!conn) {
			std::cerr << "creating connector" << std::endl;
			goto cleanup;
		}

		conn->id = drm_conn->connector_id;
		snprintf(conn->name, sizeof conn->name, "%s-" PRIu32,
			conn_str(drm_conn->connector_type),
			drm_conn->connector_type_id);
		conn->connected = drm_conn->connection == DRM_MODE_CONNECTED;

		conn->next = conn_list;
		conn_list = conn;

		std::cout << "Found display " << conn->name << std::endl;

		if (!conn->connected) {
			std::cout << "Disconnected" << std::endl;;
			goto cleanup;
		}

		if (drm_conn->count_modes == 0) {
			std::cout << "No valid modes" << std::endl;
			conn->connected = false;
			goto cleanup;
		}

		conn->crtc_id = find_crtc(drm_fd, res, drm_conn, &taken_crtcs);
		if (!conn->crtc_id) {
			fprintf(stderr, "Could not find CRTC for %s\n", conn->name);
			conn->connected = false;
			goto cleanup;
		}

		printf("  Using CRTC %" PRIu32"\n", conn->crtc_id);

		// [0] is the best mode, so we'll just use that.
		conn->mode = drm_conn->modes[0];

		conn->width = conn->mode.hdisplay;
		conn->height = conn->mode.vdisplay;
		conn->rate = refresh_rate(&conn->mode);

		printf("  Using mode %" PRIu32"x%" PRIu32"@%" PRIu32"\n",
			conn->width, conn->height, conn->rate);

		if (!create_fb(drm_fd, conn->width, conn->height, &conn->fb)) {
			conn->connected = false;
			goto cleanup;
		}

		printf("  Created frambuffer with ID %" PRIu32"\n", conn->fb.id);

		// Save the previous CRTC configuration
		conn->saved = drmModeGetCrtc(drm_fd, conn->crtc_id);

		// Perform the modeset
		ret = drmModeSetCrtc(drm_fd, conn->crtc_id, conn->fb.id, 0, 0,
			&conn->id, 1, &conn->mode);
		if (ret < 0) {
			perror("drmModeSetCrtc");
		}
		
		conn->dpi = get_dpi(drm_conn->mmWidth, conn->width);
		std::cout << "dpi: " << conn->dpi << std::endl;
		conn->font_util = FontUtil(conn->dpi);
cleanup:
		drmModeFreeConnector(drm_conn);
	}

	drmModeFreeResources(res);

	//font_util()

	// load image and see
	const int DESIRED_CHANNELS = 4;
	int width, height, channels;
	unsigned char *wallpaper = stbi_load("../images/wallpaper.jpg", &width, &height, &channels, DESIRED_CHANNELS);
    if(wallpaper == NULL) {
 		printf("Error in loading the image\n");
 		exit(1);
    }
	printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
	//unsigned char* wall2 = (unsigned char*)malloc(width * height * 4);
	convert_RGBA_to_ARGB(wallpaper, width * height * 4);
	//wallpaper = wall2;
	// printf("%lu", sizeof(wallpaper));

	// // Draw some colours for 5 seconds
	// uint8_t colour[4] = { 0x00, 0x00, 0xff, 0x00 }; // B G R X
	// int inc = 1, dec = 2;
	// for (int i = 0; i < 60 * 5; ++i) {
	// 	colour[inc] += 15;
	// 	colour[dec] -= 15;

	// 	if (colour[dec] == 0) {
	// 		dec = inc;
	// 		inc = (inc + 2) % 3;
	// 	}

		for (struct connector *conn = conn_list; conn; conn = conn->next) {
			if (!conn->connected)
				continue;

			struct dumb_framebuffer *fb = &conn->fb;
			unsigned char* img = (unsigned char *)malloc(fb->width * fb->height * DESIRED_CHANNELS);
			stbir_resize_uint8(wallpaper,width,height, width*DESIRED_CHANNELS,
							img, fb->width, fb->height, fb->stride, DESIRED_CHANNELS);
			add_text_to_framebuffer(0.5f,0.5f,"suca",img,fb->width,fb->height,conn->font_util);
			for (uint32_t y = 0; y < fb->height; ++y) {
				uint8_t *row = fb->data + fb->stride * y;

				for (uint32_t x = 0; x < fb->width; ++x) {
					row[x * 4 + 0] = img[x * 4 + 0 + fb->stride * y];
					row[x * 4 + 1] = img[x * 4 + 1 + fb->stride * y];
					row[x * 4 + 2] = img[x * 4 + 2 + fb->stride * y];
					row[x * 4 + 3] = img[x * 4 + 3 + fb->stride * y];
				}	
				// for (uint32_t x = 0; x < fb->width; ++x) {
				// 	row[x * 4 + 0] = colour[0];
				// 	row[x * 4 + 1] = colour[1];
				// 	row[x * 4 + 2] = colour[2];
				// 	row[x * 4 + 3] = colour[3];
				// }
			}
			free(img);
		}

		sleep(5);

		// Should give 60 FPS
		// struct timespec ts = { .tv_nsec = 16666667 };
		// nanosleep(&ts, NULL);
	// }

	// Cleanup
	struct connector *conn = conn_list;
	while (conn) {
		if (conn->connected) {
			// Cleanup framebuffer
			munmap(conn->fb.data, conn->fb.size);
			drmModeRmFB(drm_fd, conn->fb.id);
			struct drm_mode_destroy_dumb destroy = { .handle = conn->fb.handle };
			drmIoctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);

			// Restore the old CRTC
			drmModeCrtc *crtc = conn->saved;
			if (crtc) {
				drmModeSetCrtc(drm_fd, crtc->crtc_id, crtc->buffer_id,
					crtc->x, crtc->y, &conn->id, 1, &crtc->mode);
				drmModeFreeCrtc(crtc);
			}
		}

		struct connector *tmp = conn->next;
		free(conn);
		conn = tmp;
	}

	stbi_image_free(wallpaper); 
	close(drm_fd);
}


#ifndef _FONT_UTIL_H_
#define _FONT_UTIL_H_

#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "util.h"

class FontUtil {

    public:
        FontUtil(int monitor_dpi);
        unsigned char* getARGBBitmapCharcode(uint32_t char_code);
    public:
        FT_Error      error;
        FT_Face       face;      /* handle to face object */
        unsigned int height;     //bitmap dimension
        unsigned int width;
    private:
        FT_Library    library;   /* handle to library     */
        FT_GlyphSlot  slot;
        FT_UInt  glyph_index;
};

#endif
#ifndef _FONT_UTIL_H_
#define _FONT_UTIL_H_

#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "util.h"

class FontUtil {

    public:
        FontUtil(int monitor_dpi);

    private:
        FT_Library  library;   /* handle to library     */
        FT_Face     face;      /* handle to face object */
        FT_Error    error;

};

#endif
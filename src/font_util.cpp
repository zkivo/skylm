#include "font_util.h"
#include "util.h"

// return 0 if success, 1 error
FontUtil::FontUtil(int monitor_dpi) {
    error = FT_Init_FreeType(&library);
    if ( error ) {
        std::cout << "Error during initialization of freefont library" << std::endl;
        return;
    }

    error = FT_New_Face( library,
                        "/usr/share/fonts/truetype/arial.ttf",
                        0,
                        &face );
    if ( error == FT_Err_Unknown_File_Format ) {
        std::cout << "The font file could be opened and read, but it appears "
                  << "that its font format is unsupported." << std::endl;
        return;
    } else if ( error ) {
        std::cout << "The font file could not be opened and read." << std::endl;
        return;
    }

    error = FT_Set_Char_Size(
          face,    /* handle to face object         */
          get_num_pixel(monitor_dpi, 1000.0f)*64,     /* char_width in 1/64 of points  */
          get_num_pixel(monitor_dpi, 1000.0f)*64,   /* char_height in 1/64 of points */
          monitor_dpi,     /* horizontal device resolution  */
          monitor_dpi);   /* vertical device resolution    */
}

unsigned char* FontUtil::getARGBBitmapCharcode(uint32_t char_code) {
    FT_Bitmap ft_bitmap;

    unsigned char* out; // the output must be little endian so Blue first 
    unsigned char* gray_bitmap; //just one color per pixel
    
    unsigned int height;
    unsigned int width;

    slot = face->glyph;
    ft_bitmap = slot->bitmap;
    gray_bitmap = ft_bitmap.buffer;

    height = ft_bitmap.rows;
    width  = ft_bitmap.width;

    /* retrieve glyph index from character code */
    glyph_index = FT_Get_Char_Index(face, char_code);

    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
    if ( error )
        return NULL;
    
    /* convert to an anti-aliased bitmap */
    error = FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL );
    if ( error )
        return NULL;

    out = (unsigned char*) malloc(height * width * 4);
    for (int i = 0, p = 0 ; i < height * width; i++, p += 4) {
        out[p]     = gray_bitmap[i];
        out[p + 1] = gray_bitmap[i];
        out[p + 2] = gray_bitmap[i];
        out[p + 3] = 255;
    }
    return out;
}


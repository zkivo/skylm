#include "font_util.h"

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


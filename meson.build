project('skylm', 'cpp')
inc_dir = include_directories('include')
deps     = [dependency('libdrm'), dependency('freetype2')]
# libs     = [library('util', 'src/util.cpp', include_directories: inc_dir, dependencies: deps), library('font_util', 'src/font_util.cpp', include_directories: inc_dir, dependencies: deps)]
executable('skylm', 'src/main.cpp' , 'src/util.cpp' , 'src/font_util.cpp' , 'src/ppm.cpp' , 
    include_directories: inc_dir,
    dependencies: deps)

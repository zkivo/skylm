project('skylm', 'cpp')
inc_dir = include_directories('include')
dep     = dependency('libdrm')
executable('skylm', 'src/main.cpp', 'src/util.cpp' , 
    include_directories: inc_dir,
    dependencies: dep)

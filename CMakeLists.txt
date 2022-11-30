cmake_minimum_required(VERSION 3.1)
project(ThumbsVert)

add_compile_options( "-Wall" "-fexceptions" "-fopenmp" "-DFLTK_USE_SVG" )

#set (CMAKE_CXX_STANDARD 11)
#set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "blah" FORCE)
#set (CMAKE_CXX_FLAGS "-fpermissive")

INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} /home/kevin/fltk )

add_executable( ThumbsVert untitled.cpp Fl_Image_Browser.cxx )

find_library(FLTK fltk /home/kevin/fltk/build/lib)
find_library(FLTK_IMG fltk_images /home/kevin/fltk/build/lib)
find_library(FLTK_PNG fltk_png    /home/kevin/fltk/build/lib)
find_library(FLTK_JPG fltk_jpeg   /home/kevin/fltk/build/lib)

set(LINK_FLAGS -no-pie -fopenmp -lX11 -lXext -lm -ldl -lXinerama -lXcursor
        -lXrender -lXfixes -lz -lXft -lfontconfig -pthread
        -lpthread -ljpeg
        )

target_link_libraries(ThumbsVert LINK_PUBLIC ${FLTK} ${FLTK_IMG} ${FLTK_PNG} ${FLTK_JPEG} ${LINK_FLAGS} )

cmake_minimum_required(VERSION 3.11.0)
project(implot_demos VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
endif()

###############################################################################
# 3RD PARTY LIBS
###############################################################################

# FETCH CONTENT LIBS
# include(FetchContent)
# set(FETCHCONTENT_FULLY_DISCONNECTED ON)

# FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw)
# FetchContent_MakeAvailable(glfw)

# FetchContent_Declare(iir GIT_REPOSITORY https://github.com/berndporr/iir1)
# FetchContent_MakeAvailable(iir)

# LOCAL LIBS

# GLAD
add_subdirectory(libs/glad)

# NFD
add_subdirectory(libs/nativefiledialog-extended)

# KISS FFT
set(KISS_SRC
  "libs/kissfft/kiss_fft.h"
  "libs/kissfft/kiss_fftr.h"
  "libs/kissfft/kiss_fft.c"
  "libs/kissfft/kiss_fftr.c"
)

# MINIAUDIO
# add_library(miniaudio libs/miniaudio.h)

# # EVERYTHING ELSE
include_directories("include" "libs" "libs/kissfft")

###############################################################################
# IMGUI
###############################################################################

set(IMGUI_HEADERS
  ../imgui/imconfig.h
  ../imgui/imgui_internal.h
  ../imgui/imgui.h
  ../imgui/imstb_rectpack.h
  ../imgui/imstb_textedit.h
  ../imgui/imstb_truetype.h
  ../imgui/backends/imgui_impl_glfw.h
  ../imgui/backends/imgui_impl_opengl3.h
  ../imgui/backends/imgui_impl_opengl3_loader.h
  ../imgui/misc/cpp/imgui_stdlib.h
)

set(IMGUI_SRC
  ../imgui/imgui_demo.cpp
  ../imgui/imgui_draw.cpp
  ../imgui/imgui_tables.cpp
  ../imgui/imgui_widgets.cpp
  ../imgui/imgui.cpp
  ../imgui/backends/imgui_impl_glfw.cpp
  ../imgui/backends/imgui_impl_opengl3.cpp
  ../imgui/misc/cpp/imgui_stdlib.cpp
)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_library(imgui ${IMGUI_HEADERS} ${IMGUI_SRC})
target_link_libraries(imgui PUBLIC glfw glad OpenGL::GL)
target_compile_definitions(imgui PRIVATE IMGUI_DLL_EXPORT)

include_directories(../imgui/ ../imgui/backends ../imgui/misc/cpp)

# imnodes
# add_library(imnodes libs/imnodes/imnodes.h libs/imnodes/imnodes_internal.h libs/imnodes/imnodes.cpp)
# target_link_libraries(imnodes imgui)
# include_directories(libs/imnodes)

###############################################################################
# IMPLOT
###############################################################################

set(IMPLOT_HEADERS
  ../implot/implot.h
  ../implot/implot_internal.h
)

set(IMPLOT_SRC
  ../implot/implot.cpp
  ../implot/implot_items.cpp
  ../implot/implot_demo.cpp
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../implot/backends")
  list(APPEND IMPLOT_HEADERS ../implot/backends/implot_backend.h ../implot/backends/implot_impl_opengl3.h)
  list(APPEND IMPLOT_SRC ../implot/backends/implot_backend.h ../implot/backends/implot_impl_opengl3.cpp)
endif()

add_library(implot ${IMPLOT_HEADERS} ${IMPLOT_SRC})
target_link_libraries(implot PUBLIC imgui)
target_compile_definitions(implot PUBLIC IMPLOT_DEBUG IMPLOT_DLL_EXPORT IMPLOT_BACKEND_ENABLE_OPENGL3 IMGUI_IMPL_OPENGL_LOADER_GLAD)
set_property(TARGET implot PROPERTY CXX_STANDARD 11)
target_compile_options(implot PRIVATE -Wall -Wextra -pedantic -Werror -mavx2 -Ofast)

include_directories(../implot/)

###############################################################################
# APP FRAMEWORK
###############################################################################

add_library(app
  common/App.h
  common/App.cpp
  common/Shader.h
  common/Native.h
  common/Native.cpp
  common/Image.h
  common/Fonts/Fonts.h
  common/Fonts/FontAwesome5.cpp
  common/Fonts/RobotoRegular.cpp
  common/Fonts/RobotoBold.cpp
  common/Fonts/RobotoItalic.cpp
  common/Fonts/RobotoMonoRegular.cpp
  common/Fonts/RobotoMonoBold.cpp
  common/Fonts/RobotoMonoItalic.cpp
)
target_include_directories(app PUBLIC common)
target_link_libraries(app implot nfd)
target_compile_features(app PRIVATE cxx_std_17)

###############################################################################
# TUNER
###############################################################################

add_executable(tuner
  "src/main.cpp"
  "src/audio.cpp"
  # "src/filter.cpp"
  # "src/window.cpp"
  # "src/fft.cpp"
  "src/autocorrelation.cpp"
  "src/notes.cpp"
  ${KISS_SRC}
)
target_link_libraries(tuner app)
target_include_directories(tuner PRIVATE)
target_compile_definitions(tuner PRIVATE kiss_fft_scalar=double NOMINMAX)

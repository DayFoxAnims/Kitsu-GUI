cmake_minimum_required(VERSION 3.10)
project(KitsuGui VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== SDL2 =====
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# ===== SDL2_ttf (con pkg-config, más fiable en Termux) =====
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)
# ===== SDL2_image =====
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# ===== BIBLIOTECA =====
add_library(kitsugui STATIC
    src/window.cpp
    src/widget.cpp
    src/bg.cpp
    src/box.cpp
    src/button.cpp
    src/label.cpp
    src/renderer.cpp
    src/panel.cpp
    src/icon.cpp
    src/fps.cpp
    src/check_box.cpp
    src/switch.cpp
    src/slider.cpp
    src/progress_bar.cpp
    src/radio.cpp
    src/text_input.cpp
    src/scroll_view.cpp
    src/shapes.cpp
    src/viewport.cpp
    src/theme.cpp
    src/popup.cpp
    src/icon_theme.cpp
    src/icon.cpp
    src/context_menu.cpp
    src/config.cpp
    src/dropdown.cpp
    src/run.cpp
)

target_include_directories(kitsugui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_GFX_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}   # ← AÑADIR
)

target_link_libraries(kitsugui
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_GFX_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}      # ← AÑADIR
)

# ===== EJEMPLO =====
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_executable(kitsu_example examples/main.cpp)
    target_link_libraries(kitsu_example kitsugui ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})
endif()

cmake_minimum_required(VERSION 3.10)
project(KitsuGui VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== Instalar en ~/.local por defecto =====
if(NOT DEFINED CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install prefix" FORCE)
endif()

include(GNUInstallDirs)

# ===== SDL2 =====
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# ===== SDL2_ttf, SDL2_gfx, SDL2_image =====
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# ===== BIBLIOTECA =====
add_library(kitsugui STATIC
    src/utils.cpp           # ← NUEVO
    src/fonts.cpp           # ← NUEVO
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
    src/context_menu.cpp
    src/config.cpp
    src/dropdown.cpp
    src/run.cpp
)

target_include_directories(kitsugui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_GFX_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

target_link_libraries(kitsugui
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_GFX_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# ===== INSTALACIÓN =====
install(DIRECTORY include/kitsugui/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kitsugui
        FILES_MATCHING PATTERN "*.h")

install(FILES include/kitsugui.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS kitsugui
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Instalar temas junto a la librería
install(DIRECTORY data/themes/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/kitsugui/themes
        OPTIONAL)

# ===== EJEMPLO =====
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_executable(kitsu_example examples/main.cpp)
    target_link_libraries(kitsu_example kitsugui)
endif()

target_sources(app PRIVATE
    display_hardware.cpp
    display_thread.cpp
    display.cpp
    lvgl_manager.cpp
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
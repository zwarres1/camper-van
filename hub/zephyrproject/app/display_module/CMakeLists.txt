# 2. Add C/C++ Defines (Replaces Makefile DEFS)
# We throw away all the STM32 junk and keep the EVE config.
target_compile_definitions(app PRIVATE
    EVE_4
    EVE_4_INTERNAL_OSC
    IPS_101
    ZEPHYR_PLATFORM
)

# 3. Add Include Directories (Replaces Makefile INCDIRS)
# This tells the compiler where to look for .h files.
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/eve_layer
    ${CMAKE_CURRENT_SOURCE_DIR}/app_layer
    ${CMAKE_CURRENT_SOURCE_DIR}/riverdi_modules
    ${CMAKE_CURRENT_SOURCE_DIR}/host_layer
)

# 4. Add Source Files (Replaces Makefile SRC)
# We throw away the STM32 HAL and only compile the cross-platform Riverdi code.
target_sources(app PRIVATE
    host_layer/platform.c
    eve_layer/Gpu_Hal.c
    eve_layer/CoPro_Cmds.c
    eve_layer/Hal_Utils.c
    app_layer/App_Common.c
)

set(LVGL_EVE_DIR "$ENV{ZEPHYR_BASE}/../modules/lib/gui/lvgl/src/draw/eve")
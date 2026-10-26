target_sources(app PRIVATE
    network_thread.cpp
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
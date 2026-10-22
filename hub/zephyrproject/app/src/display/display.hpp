#pragma once

#include <lvgl_manager.hpp>

class Display
{
public:
    void run();

private:
    LVGLManager m_lvgl_manager;
};
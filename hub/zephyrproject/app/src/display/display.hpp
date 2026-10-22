#pragma once

#include <lvgl_manager.hpp>

class Display
{
public:
    Display();

    void run();

private:
    LVGLManager m_lvgl_manager;
};
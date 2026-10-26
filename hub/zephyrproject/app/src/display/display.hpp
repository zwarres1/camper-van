#pragma once

#include <lvgl_manager.hpp>

/* 
* Class that has the highest level display controls. It does things like setting the water levels
* or electricity levels in standard units from the network.
*/
class Display
{
public:
    void run();

private:
    LVGLManager m_lvgl_manager;
};
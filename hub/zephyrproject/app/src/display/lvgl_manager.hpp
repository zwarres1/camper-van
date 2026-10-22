#pragma once

#include <lvgl.h>

class LVGLManager
{
public:
    LVGLManager();

private:
    /* Private widget pointers */
    lv_obj_t* m_status_box;
    lv_obj_t* m_pump_btn;
    bool m_pump_active = false;

    /* The static C-compatible event bridge */
    static void btn_event_cb(lv_event_t * e);
    
    /* The actual C++ method that does the work */
    void toggle_pump();
};
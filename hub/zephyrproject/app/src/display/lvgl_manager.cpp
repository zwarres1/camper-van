#include <lvgl_manager.hpp>

LVGLManager::LVGLManager() {
    lv_obj_t * screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x222222), 0);

    /* 1. Create a status box (starts Red) */
    m_status_box = lv_obj_create(screen);
    lv_obj_set_size(m_status_box, 100, 100);
    lv_obj_align(m_status_box, LV_ALIGN_CENTER, 0, -80);
    lv_obj_set_style_bg_color(m_status_box, lv_color_hex(0xFF0000), 0);

    /* 2. Create the button */
    m_pump_btn = lv_btn_create(screen);
    lv_obj_set_size(m_pump_btn, 200, 80);
    lv_obj_align(m_pump_btn, LV_ALIGN_CENTER, 0, 80);

    lv_obj_t * btn_label = lv_label_create(m_pump_btn);
    lv_label_set_text(btn_label, "Toggle Pump");
    lv_obj_center(btn_label);

    /* 3. Attach the event callback AND pass 'this' as the user_data */
    lv_obj_add_event_cb(m_pump_btn, btn_event_cb, LV_EVENT_CLICKED, this);
}

/* The Static Bridge: LVGL calls this */
void LVGLManager::btn_event_cb(lv_event_t * e) {
    /* Extract the 'this' pointer we passed in build_ui() */
    LVGLManager* view = static_cast<LVGLManager*>(lv_event_get_user_data(e));
    if (view) {
        view->toggle_pump(); /* Route it back into the C++ instance */
    }
}

/* The Instance Method: Has full access to private variables */
void LVGLManager::toggle_pump() {
    m_pump_active = !m_pump_active;
    
    lv_color_t new_color = m_pump_active ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000);
    lv_obj_set_style_bg_color(m_status_box, new_color, 0);
}
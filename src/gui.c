#include <lvgl.h>

#undef LV_USE_DEMO_SCROLL
#define LV_USE_DEMO_SCROLL 1
#include "../lvgl/demos/scroll/lv_demo_scroll.c"

void create_lvgl_gui(void) {
    lv_demo_scroll();
}

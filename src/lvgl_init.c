#include <lvgl.h>
#include <display/lv_display_private.h>

#include "config.h"
#include "input.h"
#include "timer.h"

static uint32_t change_index;
static input_device_data_t* indev_data;
static lv_indev_t* rotary_encoder;

void* frame_buffer(void);

static void flush_cb(lv_display_t* disp, const lv_area_t*, uint8_t*) {
    ++change_index;
    lv_disp_flush_ready(disp);
}

static void init_display(void) {
    static lv_display_t* display;
    display = lv_display_create(FB_WIDTH, FB_HEIGHT);
    lv_display_set_flush_cb(display, flush_cb);
    const int fb_size = FB_WIDTH * FB_HEIGHT * 4;
    lv_display_set_buffers(display, frame_buffer(), NULL, fb_size, LV_DISPLAY_RENDER_MODE_DIRECT);
}

static void pointer_device_cb(lv_indev_t*, lv_indev_data_t* data) {
    data->point.x = indev_data->x;
    data->point.y = indev_data->y;
    data->state = indev_data->is_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void rotary_encoder_cb(lv_indev_t*, lv_indev_data_t* data) {
    static int32_t prev_pos;
    const int32_t pos = indev_data->encoder_pos;
    data->enc_diff = (int16_t)(pos - prev_pos);
    prev_pos = pos;
    data->state = indev_data->is_encoder_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void init_pointer_device(void) {
    static lv_indev_t* dev;
    dev = lv_indev_create();
    lv_indev_set_type(dev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(dev, pointer_device_cb);
}

static void init_rotary_encoder(void) {
    lv_indev_t* const dev = lv_indev_create();
    rotary_encoder = dev;
    lv_indev_set_type(dev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(dev, rotary_encoder_cb);
}

static void init_input(void) {
    indev_data = input_device_data();
    init_rotary_encoder();
    init_pointer_device();
}

void set_rotary_encoder_group(lv_group_t* group) {
    lv_indev_set_group(rotary_encoder, group);
}

bool pop_should_update(void) {
    static uint32_t prev_change_index;
    const bool res = change_index != prev_change_index;
    prev_change_index = change_index;
    return res;
}

void init_lvgl(void) {
    static bool is_initialized = false;
    if (is_initialized)
        return;

    lv_init();
    lv_tick_set_cb(mono_ms);
    init_display();
    init_input();
    is_initialized = true;
}

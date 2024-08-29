#ifndef UNICODE
#define UNICODE
#endif

#include <stdint.h>

#include <windows.h>

#include "config.h"
#include "input.h"
#include "timer.h"

LRESULT CALLBACK window_proc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
uint32_t lv_timer_handler(void);
bool pop_should_update(void);

static HBITMAP bitmap;
static void* bitmap_data;
static uint32_t next_frame_time;
static HWND hwnd;
static input_device_data_t indev_data;

static BITMAPINFOHEADER bitmap_header(void) {
    BITMAPINFOHEADER h = {};
    h.biSize = sizeof(BITMAPINFOHEADER);
    h.biWidth = FB_WIDTH;
    h.biHeight = -FB_HEIGHT;
    h.biPlanes = 1;
    h.biBitCount = 32;
    h.biCompression = BI_RGB;
    h.biXPelsPerMeter = 4000;
    h.biYPelsPerMeter = 4000;
    return h;
}

int init_window(HINSTANCE h_inst, int n_cmd) {
    const wchar_t class_name[]  = L"main window class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = window_proc;
    wc.hInstance = h_inst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = class_name;
    RegisterClass(&wc);

    const wchar_t window_title[] = L"LVGL-MinGW";
    const DWORD window_style = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    RECT window_rect = { 0, 0, FB_WIDTH, FB_HEIGHT };
    AdjustWindowRect(&window_rect, window_style, false);
    const int adjusted_width = window_rect.right - window_rect.left;
    const int adjusted_height = window_rect.bottom - window_rect.top;
    hwnd = CreateWindow(
        class_name, window_title, window_style,
        CW_USEDEFAULT, CW_USEDEFAULT, adjusted_width, adjusted_height,
        NULL, NULL, h_inst, NULL
        );

    if (hwnd == NULL)
        return -1;

    BITMAPINFO info = { .bmiHeader = bitmap_header() };
    HDC hdc = GetDC(hwnd);
    bitmap = CreateDIBSection(hdc, &info, DIB_RGB_COLORS, &bitmap_data, NULL, 0);
    ReleaseDC(hwnd, hdc);

    ShowWindow(hwnd, n_cmd);
    return 0;
}

int run_gui() {
    next_frame_time = mono_ms();
    MSG msg = {};
    while (true) {
        const uint32_t ms = mono_ms();
        if (ms >= next_frame_time) {
            const uint32_t ms_until_next = lv_timer_handler();
            next_frame_time = ms + ms_until_next;
        }
        const bool should_update = pop_should_update();
        const bool msg_received = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (!(msg_received || should_update)) {
            sleep_ms(3);
            continue;
        }
        if (msg.message == WM_QUIT)
            break;

        if (msg_received) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (should_update && (!msg_received || (msg.message != WM_PAINT)))
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
    }
    return 0;
}

void* frame_buffer(void) {
    return bitmap_data;
}

input_device_data_t* input_device_data(void) {
    return &indev_data;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    switch (umsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            HDC hdc_mem = CreateCompatibleDC(hdc);
            HGDIOBJ prev_bitmap = SelectObject(hdc_mem, bitmap);
            BitBlt(hdc, 0, 0, FB_WIDTH, FB_HEIGHT, hdc_mem, 0, 0, SRCCOPY);
            SelectObject(hdc_mem, prev_bitmap);
            DeleteDC(hdc_mem);

            EndPaint(hwnd, &ps);
        }
        return 0;
    case WM_MOUSEMOVE:
        const POINTS point = MAKEPOINTS(lparam);
        indev_data.x = point.x;
        indev_data.y = point.y;
        indev_data.is_pressed = (wparam & MK_LBUTTON) != 0;
        indev_data.is_encoder_pressed = (wparam & MK_MBUTTON) != 0;
        return 0;
    case WM_LBUTTONDOWN:
        indev_data.is_pressed = true;
        return 0;
    case WM_LBUTTONUP:
        indev_data.is_pressed = false;
        return 0;
    case WM_MOUSEWHEEL:
        const int wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam);
        indev_data.encoder_pos += wheel_delta / 120;
        return 0;
    case WM_MBUTTONDOWN:
        indev_data.is_encoder_pressed = true;
        return 0;
    case WM_MBUTTONUP:
        indev_data.is_encoder_pressed = false;
        return 0;
    }
    return DefWindowProc(hwnd, umsg, wparam, lparam);
}

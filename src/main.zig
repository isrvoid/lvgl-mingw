const std = @import("std");

const HINSTANCE = std.os.windows.HINSTANCE;
var timer: std.time.Timer = undefined;

extern fn init_window(?HINSTANCE, i32) i32;
extern fn run_gui() i32;
extern fn init_lvgl() void;
extern fn create_lvgl_gui() void;

export fn mono_ms() u32 {
    return @truncate(timer.read() / 1_000_000);
}

export fn sleep_ms(n: u32) void {
    std.time.sleep(@as(u64, n) *% 1_000_000);
}

pub export fn wWinMain(inst: ?HINSTANCE, _: ?HINSTANCE, _: ?std.os.windows.PWSTR, n_cmd: i32) i32 {
    timer = std.time.Timer.start() catch @panic("timer");
    if (init_window(inst, n_cmd) != 0)
        return -1;

    init_lvgl();
    create_lvgl_gui();
    return run_gui();
}

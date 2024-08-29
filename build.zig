const std = @import("std");
const mem = std.mem;

const mingw_prefix = "/usr/x86_64-w64-mingw32/";

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{ .default_target = .{ .os_tag = .windows }});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "lvgl-mingw",
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    const c_flags: []const []const u8 = &.{ "-std=c23", "-Wall", "-Wextra", "-Werror" };
    exe.addCSourceFile(.{ .file = b.path("src/window.c") });
    exe.addCSourceFile(.{ .file = b.path("src/lvgl_init.c"), .flags = c_flags });
    exe.addCSourceFile(.{ .file = b.path("src/gui.c"), .flags = c_flags });
    exe.subsystem = .Windows;
    exe.mingw_unicode_entry_point = true;
    addWindowsApi(b, exe);
    exe.addIncludePath(b.path("lvgl/src"));

    const lvgl = b.addStaticLibrary(.{
        .name = "lvgl",
        .target = target,
        .optimize = optimize,
    });
    lvgl.linkLibC();
    addLvgl(b, lvgl);
    exe.linkLibrary(lvgl);

    b.installArtifact(exe);

    const exe_unit_tests = b.addTest(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_exe_unit_tests.step);
}

fn addLvgl(b: *std.Build, a: *std.Build.Step.Compile) void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const lvgl_src_dir = "lvgl/src";
    a.addIncludePath(b.path(lvgl_src_dir));
    const lvgl_src = getFilesWithEnding(arena.allocator(), ".c", lvgl_src_dir) catch @panic("LVGL src");
    a.addCSourceFiles(.{ .root = b.path(lvgl_src_dir), .files = lvgl_src });
}

fn addWindowsApi(_: *std.Build, a: *std.Build.Step.Compile) void {
    a.addIncludePath(.{ .cwd_relative = mingw_prefix ++ "include" });
    a.addObjectFile(.{ .cwd_relative = mingw_prefix ++ "lib/libmincore.a" });
    a.addObjectFile(.{ .cwd_relative = mingw_prefix ++ "lib/libuser32.a" });
    a.addObjectFile(.{ .cwd_relative = mingw_prefix ++ "lib/libgdi32.a" });
}

// result is relative to sub_path
fn getFilesWithEnding(allocator: mem.Allocator, ending: []const u8, sub_path: []const u8) ![]const []const u8 {
    var res = std.ArrayList([]const u8).initCapacity(allocator, 0x200) catch unreachable;
    var dir = try std.fs.cwd().openDir(sub_path, .{ .iterate = true });
    defer dir.close();
    var walker = try dir.walk(allocator);
    defer walker.deinit();
    while (try walker.next()) |e| {
        if (!mem.endsWith(u8, e.basename, ending)) continue;
        const path = allocator.dupe(u8, e.path) catch @panic("OOM");
        res.appendAssumeCapacity(path);
    }
    return res.toOwnedSlice() catch unreachable;
}

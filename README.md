# lvgl-mingw

Template for Zig Windows app using LVGL as GUI framework.

Tested with the current development version of the Zig compiler, might work with the latest tagged version.

Requires MinGW. On Ubuntu:
```
sudo apt install mingw-w64-x86-64-dev
```

Clone with `--recursive` or `git submodule update --init` when already cloned to fetch LVGL.
`mingw_prefix` in `build.zig` might need to be changed for a different Linux distro.
```
cd lvgl-mingw
zig build
```

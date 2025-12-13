<br />
<div align="center">
  <a href="https://github.com/reuzdev/wavet">
    <img src="docs/images/wavet_logo_20x.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">wavet</h3>

  <p align="center">
    FlagWaver in terminal :)
  </p>
</div>

## About
<div align="center">
    <img src="docs/images/screenshot_turkey_centered_blue_bg.png" alt="Screenshot">
</div>

Wavet (stylized as wavet) is a terminal app for playing flag waving animation. It is intended for small pixel art flags and comes with flags designed by [R74n](https://www.youtube.com/@R74n) and used with their permission (more on that [here](assets/R74n/README.md)), and several flags I designed myself. It is written in C++17 using Win32 API for Windows and POSIX standard library for Linux for handling terminal-related stuff, and `stb_image.h` for loading images.

## Build and Install
If you have a:
* C++ compiler
* CMake

You can build wavet like this:

1. Create and `cd` into a `build` directory
```
mkdir build && cd build
```

2. Configure
```
cmake -DCMAKE_BUILD_TYPE=Release ..
```

3. Build
```
cmake --build . --config Release
```

4. Install (Optional)
```
cmake --install .
```

NOTE: Omit `-DCMAKE_BUILD_TYPE=Release` and `--config Release` if you want a Debug build. Also the former doesn't have an effect on MSVC and the latter doesn't have an effect on Linux/MinGW, so you can omit them on their corresponding platforms.

## Usage
Use `--help` to learn how to use wavet.

## Links
* R74n's Pixel Flags: https://r74n.com/pixelflags/
* `stb_image.h` Library: https://github.com/nothings/stb/blob/master/stb_image.h

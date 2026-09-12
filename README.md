# Mokosz

A C++ OpenGL renderer for physically based rendering, terrain generation, and editor-driven scene tweaks. The project is configured with CMake and pulls most dependencies automatically via `FetchContent`, while a few local utility files are expected in the `libs/` folder.

## Features

- Physically based rendering pipeline
- OpenGL 4.5-style rendering setup with GLFW, GLM, GLAD, and Assimp
- HDRI/IBL pipeline using cubemap generation, irradiance, prefiltering, and BRDF LUT
- Terrain generation with noise-based mesh creation and material blending
- ImGui-based editor panels for terrain editing
- Asset copying to the binary output directory so relative asset paths work reliably

## Project structure

- `src/` — main application and engine modules
  - `core/` — windowing, camera, input, object loading
  - `renderer/` — meshes, shaders, materials, textures, lighting
  - `editor/` — ImGui editor panels
  - `terrain/` — terrain noise and mesh generation
- `assets/` — shaders, textures, HDRI files, and other runtime assets
- `libs/` — local third-party dependencies
  - `glad/` — generated OpenGL loader headers and source
  - `stb/` — `stb_image.h`
- `CMakeLists.txt` — main project configuration

## Requirements

- CMake 3.20 or newer
- C++20 compatible compiler
- Visual Studio 2022 on Windows, or another C++20 toolchain with OpenGL support
- Git
- Internet access for fetching dependencies during the first configure/build

## Local setup

Before configuring the project, make sure the following files exist:

1. `libs/glad/include/glad/glad.h`
2. `libs/glad/include/KHR/khrplatform.h`
3. `libs/glad/src/glad.c`
4. `libs/stb/stb_image.h`

You can generate the GLAD files at https://glad.dav1d.de with:

- Language: C/C++
- API: OpenGL 4.5+
- Profile: Core
- Extensions: none required

Then place the generated files into the matching folders under `libs/glad/`.

For `stb_image.h`, download it from:

- https://github.com/nothings/stb/blob/master/stb_image.h

and save it as `libs/stb/stb_image.h`.

## Build

From the project root (`Mokosz`):

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

On Windows with Visual Studio generators, the resulting executable is typically:

```powershell
./build/Debug/PBRRenderer.exe
```

## Run

After a successful build, run the executable from the project root or from the build output folder:

```powershell
./build/Debug/PBRRenderer.exe
```

## Common CMake issue

If you see a CMake cache mismatch like:

```text
The current CMakeCache.txt directory ... is different than the directory ... where CMakeCache.txt was created
```

then the old `build` directory is stale. Delete it and reconfigure from the current project folder:

```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build
```

## Notes

- The project copies `assets/` next to the generated binary so shaders and textures can be loaded using relative paths.
- The generated `build/` folder is intentionally ignored by Git.
- `build_output.txt` appears to be a local log/output file and is not part of the source project logic.

## License

This project does not currently declare a license in the repository, so use it with caution unless the author has provided additional licensing details.


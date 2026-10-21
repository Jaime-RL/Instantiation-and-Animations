# OpenGL Scene: Instancing & Animation

Animated night-driving scene in **C++ / OpenGL / GLSL**: a textured **Cadillac** drives along an **endless road** lined with street lamps, rendered with **multiple spotlights**, switchable car headlights and first/third-person cameras. Models are loaded from Wavefront `.obj` files.

## Highlights

- **Endless road via tile recycling**: 11 road tiles (each with two lamps) are re-positioned every frame with `fmod`, so a finite set of instances produces an infinite scrolling world.
- **Multi-light shading**: all lamp positions are gathered first and sent to the shader as a uniform array (up to 22 spotlights with inner/outer cone falloff), plus optional car headlights.
- **Transform hierarchy** built from scratch (`algebra.cpp`): scaling, rotation, translation, view and projection matrices.
- **Cameras**: third-person orbit view and first-person (driver) view; perspective / orthographic projection.
- **Shaders**: basic, interlaced, Phong and cartoon, switchable at runtime.
- Skybox (cube map) and desert terrain.

## Controls

| Key | Action |
|---|---|
| `1`–`4` | Switch shader (basic / interlaced / Phong / cartoon) |
| `c` | Toggle first-person / third-person camera |
| `l` | Toggle car headlights |
| `x` `y` `z` (+Shift) | Move camera |
| `i` `j` `k` (+Shift) | Rotate camera |
| `f` / `F` | Field of view |
| `P` | Perspective / orthographic |
| `p` | Pause / resume animation |
| `q` | Quit |

## Building

Windows + Visual Studio 2022: open `DVA338_Lab2.vcxproj`; the **freeglut** and **GLEW** NuGet packages in `packages.config` are restored on the first build. Run with the project folder as the working directory so shaders and models are found through relative paths.

> **Note:** the `road.obj`, `lamp.obj` and `desert.obj` models are not included in the repository and must be placed in `lab3/RoadSkybox/`.

## Third-party code & assets

- [`obj.c`](lab3/obj-4.c) OBJ loader by Robert Kooima (MIT-style licence, see file header)
- Cadillac 3D model and textures (`lab3/Cadillac/`) provided with the course material

## Tech

C++ · OpenGL · GLSL · freeglut · GLEW · Visual Studio

*Lab from Fundamentals of Computer Graphics (DVA338) at Mälardalen University, Sweden.*

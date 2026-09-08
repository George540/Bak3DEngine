# The Bak3D Rendering Demo Engine
Copyright (c) 2022-2026 George Mavroeidis - GeoGraphics

<img width="1920" height="1030" alt="bak3d_v1 3_particles_demo" src="https://github.com/user-attachments/assets/5189969d-9a87-4791-bc38-b31ef2b73f27" />


# What is Bak3D

Bak3D (baked) is a real-time beginner-friendly 3D rendering engine that can be used as a playground, focusing on simplicity, continous learning, and understanding various rendering concepts. The engine contains simplified features that can be tuned in real time using the engine's editor. It is intended for early learners to use as a reference for their own projects, school assignments or just experimenting. It is also good for seasoned professionals who love simple things from simple times. So if you are a student or just starting to understand computer graphics and engine, this one is especially for you!

# Motivation

Bak3D engine started as a fun project back at a time where I just graduated from University in 2022. I had just started my first full-time opportunity as a Junior 3D Graphics Programmer and I wanted to make a project that will supplement my learning experience from that job. Based on an old sample project I made for a Computer Graphics course, Bak3D became an super-extension of it. It formed a playground for different rendering and engine concepts I always wanted to learn more about and introduce them to other beginners willing to learn or get help for their school assignments. Although it is a 4 year old engine based on junior knowledge, I occasionally still try to update it when I am looking to rapidly implement something I am interested in playing around with.

<img width="1919" height="1030" alt="bak3d_v1 3_postprocess" src="https://github.com/user-attachments/assets/cf834e07-bbb0-4891-ae6d-8bb5c84fa24d" />

*Bak3D engine v1.3 comes with a complete and more professional editor UI overhaul as well as a robust post processing stack, anti-aliasing and rendering optimizations.*

<br>

# This engine is for you if you are...

- :ballot_box_with_check: ...a rendering enthusiast and are interested how video games look like behind the hood.
- :ballot_box_with_check: ...an aspiring beginner who just got introduced to the concept of computer graphics.
- :ballot_box_with_check: ...a student currently taking a course in Computer Graphics in school, university or on even online.
- :ballot_box_with_check: ...completely lost about this topic, whether your instructor is not helping at all or just finding it hard to put the pieces together.
- :ballot_box_with_check: ...an experienced individual in making video games and/or engines and are willing to help newcomers explore the craft.
- :ballot_box_with_check: ...looking to get a starting point or reference for making your first engine.

This is (or planning to be) the perfect starting point for computer graphics and graphics programming. The purpose of this engine is to assist students and beginners on understanding the behind-the-scenes for different rendering phenomena. It is also a great starting point for creating their own project (under licensing conditions). It is made in OpenGL since it is usually the go-to library for most of introduction courses and to simplify most of the rendering processes. This helps with putting a focus on the actual learning material rather than the API itself.

---


## Engine Features

| Feature | Description |
|--------|---------|
| **Editor** | Overhauled ImGui-based editor with a more standard and professional structure, familiar to commercial engines. |
| **Lighting & Casters** | Simple Blinn-Phong lighting method implemented with different casters: spotlight, pointlight, directional light. Can tune properties in Environment Settings. |
| **Asset import** | Assimp for loading 40+ model formats, stbi_image for loading image-based formats, JSON for material loading and saving. |
| **Post-Processing** | Robust Post-Processing stack, focusing on color grading and kernel effects with convolution matrices. |
| **Anti-Aliasing** | Native MSAA implemented on the rasterization level with 2x2, 4x4 and even 8x8 if GPU vendor allows it. |
| **Input** | Keyboard and Mouse: Hold **Left Mouse Button** down and move around to orbit the scene's origin and **Scroll Wheel** to zoom in and out. |
| **Particles** | GPU instanced and culled particles with SoA (Structure of Arrays) CPU particle management and multiple particle emitters. |
| **Performance & Profiling Tools** | GPU tools ranging from timers, captures and runtime shader recompilation. Great for debugging and profiling. |

<img width="1920" height="1031" alt="bak3d_v1 3_particles" src="https://github.com/user-attachments/assets/b348b026-0a97-4e19-b0fb-56ad53bb91e4" />

*Robust Particle System with GPU instancing, GPU culling, CPU Structure of Arrays (SoA), all in dynamic and multiple particle emitters.*

<br>

## Third-Party libraries

Found inside the third_party directory, these are the current libraries being supported and setup locally using CMake.
| Library | Details |
|--------|---------|
| **[Assimp](https://github.com/assimp/assimp)** | Universal library for loading 40+ model formats in the engine. |
| **[Glad](https://glad.dav1d.de)** | Simple OpenGL generated header for linking core OpenGL GPU functions to the source code. |
| **[GLFW](https://www.glfw.org)** | Multi-platform support for creating windows, contexts, surfaces, receiving input and events. |
| **[GLM](https://github.com/g-truc/glm)** | OpenGL-based header-only math library. |
| **[ImGui](https://github.com/ocornut/imgui)** | Bloat-free immediate-mode graphical user interface for the engine's editor. |
| **[KHRPlatform](https://registry.khronos.org/EGL/api/KHR/khrplatform.h)** | Khronos group header file that defines platform-specific types and macros for OpenGL. |
| **[stb_image](https://github.com/nothings/stb)** | Lightweight header file for loading and creating textures in OpenGL, by stb. |
| **[Nlohmann JSON](https://github.com/nothings/stb)** | Modern JSON library with intuitive syntax and STL compatibility for C++. |

<img width="1918" height="1030" alt="bak3d_v1 3_model" src="https://github.com/user-attachments/assets/44acbbc3-e5a5-47da-9982-809839bced93" />

*Custom assimp loading with materials loaded and saved using JSON files and intuitive editor properties setup.*

<br>

## How to build
Simply clone the repo locally and run the setup.py script located in the root of the repository. Choose your .sln format and configuration and then open the solution once everything is done.

## How to launch a release build
Download and extract the latest .zip folder on the *Releases* section onto your personal computer. Enter the folder and launch the **Bak3D Engine** .exe application. The program will run in windowed version that covers the entire display. To close the program, just open the console window and close it.


## Roadmap

### [v1.4 - The Rendering Universe Expands](https://github.com/George540/Bak3DEngine/milestone/3)
Development will start Fall 2026
| Status | Feature | Description |
| :---: | :--- | :--- |
|☐| **Shadows** | With a complex environment scene, implementing CSM shadows will make it even more lively. |
|:gear:| **PBR Lighting** | A fundamental rendering concept I've been looking forward to implement. PBR Materials as well as HDR and IBL. |
|:ballot_box_with_check:| **Deferred Rendering** | Apply the possibility to integrate rendering features at scale and keep it industry standard. Opens up the possibility for multiple lights in scene. |
|:ballot_box_with_check:| **Compute Pipeline** | Add compute shaders in the Renderer for opening the possibility for more rendering features. |
|:ballot_box_with_check:| **Advanced GPU Particles** | Particle System is barely keeping up. A new and more capable approach is needed, like compute shaders, new buffer objects, culling, etc. |
|:ballot_box_with_check:| **Multiple Objects in scene** | Support multiple objects in scene, even from the same type. |
|:ballot_box_with_check:| **Scene Graph** | Add a scene graph for proper handling of multiple objects. |
|:gear:| **GPU Culling (Frustrum, Occlusion, Instance, Distance)** | As the scene grows, we need to handle it at scale with culling unecessary or unseen objects. |
|:ballot_box_with_check:| **Bonus: FPV movement** | make camera move around the scene without just being restricted in orbit. |

### [v1.5 - The Advanced Engine World](https://github.com/George540/Bak3DEngine/milestone/3)
Development will start 2027 TBD
| Status | Feature | Description |
| :---: | :--- | :--- |
|☐| **LUT Atmosphere** | Render realistic sky and atmosphere using LUT atmospheric scattering. |
|☐| **Terrain** | Simple terrain concept for loading a height map into a plane mesh, used for lighting purposes. |
|☐| **Rigidbodies** | Add physics for different shapes and rigidbodies using compute shaders. |
|☐| **Cloudscapes with Volumetric Raymarching** | If time and motivation, add a simple rigidbody structure for fun physics. |
|☐| **Full Post Processing** | Implement the full post process tack (Blur, DoF, Tonemapping & hdr, Reflections, Full Color Grading, Motion Blue, Lens Flare, Film Grain, GTAO, AA (MSAA, SMAA, DLSS, FSR, XeSS, TSR) |
|☐| **Post Process Volumes** | Implement global and local volumes with different shapes |
|☐| **Skeletal Animation and Physics** | Implement Skeletal Animation, physics, state machines and graph editors. |
|☐| **Level Editor** | Full ECS, play-in-viewport, multi-level editor with prefabs, expanding the current Scene Graph structure |

### [v1.6 - Project Tempest](https://github.com/George540/Bak3DEngine/milestone/3)
Development will start 2028 TBD (Big Project and just an idea. Taking the engine to the next level)
| Status | Feature | Description |
| :---: | :--- | :--- |
|☐| **Editor QoL** | Asset editor visualizers, shader editors, multi-entity editing, etc |
|☐| **Profile In-Editor Tools** | GPU/CPU timers, breadcrumbs, crash reports, more logs, etc |
|☐| **RHI Command List** | Implement an RHI to abstract GPU commands for different API's. |
|☐| **Vulkan** | Implement the Vulkan 1.4 API into the RHI with HLSL and/or Slang support. |
|☐| **SDL** | Implement SDL as the new input and window library. |
|☐| **Bindless Rendering** | Make all rendering bindless for all APIs. |
|☐| **Streaming** | Make textures virtual with texture streaming. |
|☐| **Geometry and Tessellation Shaders** | Add new types of shaders in the Shader pipeline. |
|☐| **Node Graphs** | Make and use Node graphs for animation, compositing, physics and more. |

I am open for requests as well as open to any support to help implement these features, although I am willing to try to implement most of these myself whenever I can. Just let me know!

See [project board](github.com/users/George540/projects/9) and [issue tracking](https://github.com/George540/Bak3DEngine/issues) for more details on what's currently being developed.

---


## Epilogue

In general, this project is aimed to be very simple with no commercial goal in mind. It is mostly a learning project and a demonstration of my knowledge. If anyone is using it for an academic assignment, project or even personal use, do not hesitate to credit this repository (see license).

I hope you enjoy using my little beginner engine and I hope you learn many more things with me as well.

## Acknowledgments

Special thanks to...

1. [Joe de Vries](http://joeydevries.com/#home) for his ebook ["Learn OpenGL"](https://learnopengl.com/About) for teaching me graphics when I was just a student. I initially made this project as a learning tool from his guide for one of my assignments. To attribute his work properly, here is [Joe's twitter](https://twitter.com/JoeyDeVriez.) as well. The code provided on this ebook is licensed under the [CC BY 4.0](https://creativecommons.org/licenses/by-nc/4.0/) license. Proper attribution was given as well as any direct reference of his work in my code. This work is non-commercial.

2. The cool experts at [Graphics Programming](https://discord.gg/DStdQSaN) discord server for answering my basic or even obscur questions when I needed help understanding something that was bothering me for days.

3. [Tom Looman](https://www.linkedin.com/in/tomlooman/) and his [Complete Game Optimization for Unreal Engine 5](https://courses.tomlooman.com/p/unrealperformance?coupon_code=COMMUNITY15) course that helped me advance my learning for optimizing games and graphics programs. A lot of his topics have been crucial to understanding numerous concepts I have encountered when designing this engine.

4. Finally, [vblanco20-1](https://github.com/vblanco20-1) for his thorough guide to transitioning to Vulkan a few years later after learning OpenGL. Although this is (currently) an OpenGL project, some of the Vulkan concepts in his guide have been an inspiration for this engine. Attributing to his github repository guide [vkguide.dev](https://vkguide.dev)

## License

**MIT License**, free to use with attribution.



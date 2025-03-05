
# PlazaEngine
3D Game Engine made with C++, supports OpenGL and Vulkan.

## Build Instructions
Clone the repository with: `git clone https://github.com/GiovaneBernardo/PlazaEngine.git --recursive`
Open the Repository folder with Visual Studio

## Features

### Cascaded Shadow Mapping
CSM using PCF for soft shadows and Multiview Rendering for better performance
### PBR Materials
Physically Based Rendering with Diffuse Irradiance and Specular Image Based Lighting
### Deferred Rendering
Drawing the Depth, Diffuse, Normal, and Others (Mettalic on Y, Roughness on Z) textures
### Tiled Deferred Lighting
Compute Shader for sorting which lights affects each tiles, then a fullscreen pass that uses this data to light the pixels
### Bloom
Bloom implementation used on Unity and Call of Duty: Advanced Warfare
### 3D Physics
Using Nvidia's Physx 5.4.0

# Assets
The Engine has an asset importer which gets models outside the engine and converts it to the engine's format. It also has an asset loader which loads the converted model file into the scene. Imported assets are stored in files inside the project and loaded assets are stored in memory, managed by the Assets Manager.
Below are the supported Formats:
### Models:
.obj .fbx
### Textures:
.png .jpg

# Shipping Games
1 - Use export button inside the engine.
2 - Copy the "Compiled Shaders" folder from the out folder of the engine's solution and paste into the game's folder.
3 - Copy all DLLs inside the "Shipping DLLs" folder and te them into the game's folder.



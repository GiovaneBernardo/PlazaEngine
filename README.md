
# PlazaEngine
3D Game Engine made with C++, supports OpenGL and Vulkan.

## Build Instructions
Clone the repository with: `git clone https://github.com/YlaxusB/PlazaEngine.git --recursive`  
Open the Repository folder with Visual Studio

## Features

### Cascaded Shadow Mapping
### PBR Materials
### Deferred Rendering
### Clustered Deferred Lighting
### Bloom
Based on Unity and Unreal 4 implementaiton
### 3D Physics
Using Nvidia's Physx 5.2.0

# Assets
The Engine has an asset importer which gets models outside the engine and converts it to the engine's format. It also has an asset loader which loads the converted model file into the scene. Imported assets are stored in files inside the project and loaded assets are stored in memory, managed by the Assets Manager.   
Below are the supported Formats:
### Models:
.obj .fbx
### Textures:
.png .jpg

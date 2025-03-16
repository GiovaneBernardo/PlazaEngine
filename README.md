# Plaza Engine
<img align="right" src="https://github.com/user-attachments/assets/7b921aee-0626-462c-a76c-82807438fb41" width="450" style="float: right; margin-left: 20px;">  
Plaza Engine is an open source 3D Game Engine + Editor, made for giving the game developer complete control over the engine.  

C++ Scripting allows for direct communication with engine code, without any interface.  

### Build Instructions  
Clone the repository with: `git clone --recursive https://github.com/GiovaneBernardo/PlazaEngine.git`  

Open the Repository folder with Visual Studio on windows or Visual Studio Code on linux (VsCode need cmake extension)  

You can build the Editor executable, the Game executable, or the Lib (PlazaEngineCore) used for the C++ game scripting. Both editor and game executable will build PlazaEngineCore lib.  

#### Requirements:
* Vulkan SDK: Can be downloaded [here](https://vulkan.lunarg.com/) (tested with 1.4.304.1, but other versions may work)  
##### Windows:  
* MSVC, C++ 20  
* Visual Studio 17 2022  
##### Linux
* Clang, C++ 20
* Ninja

### Scripting  
The C++ scripting works very similiary to Unity C# scripting, where you have a project that only builds a dll, the editor/game then loads the dll and execute its code.  

Entities have a scripting component that will hold your scripts.  

Scripts can derive from CppScriptComponent, that is used by entities, and will have its virtual functions for start, update, terminate, etc.  

Scripts may also derive from other classes for making custom editors in the future.  

Your game project will have a CMakeLists.txt where you will need to tell the path for the engine folder, you must clone the engine and build at least once the PlazaEngineCore lib.

### Shipping
* Use export button inside the editor for generating a more optmized version of your assets.  
* Build PlazaShippingGame in release mode and place it on the folder of the exported game, created by the editor.   
* Copy the "Compiled Shaders" folder from the out folder of the engine's solution and paste into the same folder as PlazaShippingGame executable.  
* If "Compiled Shaders" folder doesn't exists, you may need to run the PlazaShippingGame or PlazaEngineEditor at least once.  

### Features

#### Cascaded Shadow Mapping
CSM using PCF for soft shadows and Multiview Rendering for better performance
#### PBR Materials
Physically Based Rendering with Diffuse Irradiance and Specular Image Based Lighting
#### Deferred Rendering
Drawing the Depth, Diffuse, Normal, and Others (Mettalic on Y, Roughness on Z) textures
#### Tiled Deferred Lighting
Compute Shader for sorting which lights affects each tiles, then a fullscreen pass that uses this data to light the pixels
#### Bloom
Bloom implementation used on Unity and Call of Duty: Advanced Warfare
### 3D Physics
Using Nvidia's Physx 5.4.0

### Assets
The Engine has an asset importer which gets models outside the engine and converts it to the engine's format. It also has an asset loader which loads the converted model file into the scene. Imported assets are stored in files inside the project and loaded assets are stored in memory, managed by the Assets Manager.
Below are the supported Formats:  
#### Models:  
.obj .fbx  
#### Textures:  
.png .jpg .jpeg .tga .dds .hdr  
#### Audio:  
.mp3  

# Showcase
* [Editor](https://youtu.be/1siEjk5D_nw) (2024-12-23)  
* [The Summoners Path](https://youtu.be/7wsewy9MQMo), Game made for Ludum Dare 55 (2024-04-15)  

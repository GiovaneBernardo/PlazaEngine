#define PX_PHYSX_STATIC_LIB
#include <Engine/Core/PreCompiledHeaders.h>
#include "EntryPoint.h"
#include <ThirdParty/GLFW/include/GLFW/glfw3.h>

#include "Engine/Application/Application.h"
#include "Engine/Core/Time.h"
#include "Editor/SessionCache/Cache.h"

#define TRACY_NO_INVARIANT_CHECK 1
#include "Editor/DefaultAssets/Models/DefaultModels.h"

#include "Engine/Core/Physics.h"
#include "Editor/Project.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Audio/Audio.h"
#include "Editor/Settings/SettingsSerializer.h"
#include "Engine/Core/Scripting/CppScriptFactory.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <codecvt> 

//#include "Embed.h"

int main() {
	//EmbedResources::Init();
	Plaza::Log::Init();
	Plaza::Application::Init();
	return 0;
}

#ifdef WIN32
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
) {
	main();
}
#endif
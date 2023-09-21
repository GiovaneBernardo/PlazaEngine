#include "EntryPoint.h"
#include <GLFW/glfw3.h>

#include "Engine/Application/Application.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/Skybox.h"
#include "Editor/SessionCache/Cache.h"

ApplicationClass* Plaza::Application = new Plaza::ApplicationClass();

using namespace Plaza;


#include "Editor/DefaultAssets/Models/DefaultModels.h"

#include "Engine/Vendor/Tracy/tracy/Tracy.hpp"

#include "Engine/Core/Physics.h"
#include "Engine/Core/Scripting/Mono.h"
#ifdef TRACY_ENABLE
int main() {
	// Start
	Application->CreateApplication();
	Physics::Init();
	Application->activeScene->mainSceneEntity = new Entity("Scene");
	Editor::DefaultModels::Init();
	if (filesystem::exists(Application->enginePathAppData + "cache.yaml"))
		Editor::Cache::Load();
	Application->Loop();
	Application->Terminate();
	return 0;
}
#endif
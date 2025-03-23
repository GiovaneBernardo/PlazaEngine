#include "Engine/Core/PreCompiledHeaders.h"
#include "CppScriptComponent.h"
#include "Engine/Core/Scripting/CppScriptFactory.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	void CppScriptComponent::AddScript(CppScript* script) {
		script->mEntityUuid = this->mUuid;
		mScriptsUuid.push_back(script->mAssetUuid);
		mScripts.push_back(script);
	}

	CppScript* CppScriptComponent::AddScriptNewInstance(Scene* scene, uint64_t scriptUuid) {
		Script* assetScript = AssetsManager::GetScript(scriptUuid);
		CppScript* script = ScriptFactory::CreateScript(std::filesystem::path(assetScript->mAssetName).stem().string());
		if (!script) {
			PL_CORE_ERROR("Added Script is a nullptr");
			return nullptr;
		}
		script->mAssetUuid = assetScript->mAssetUuid;
		script->mEntityUuid = this->mUuid;
		mScriptsUuid.push_back(script->mAssetUuid);
		mScripts.push_back(script);
		if (scene->mRunning) {
			script->OnStart(scene);
		}
		return script;
	}
}

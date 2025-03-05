#pragma once
#include "DragDrop.h"
#include "Editor/GUI/FileExplorer/File.h"
#include "Engine/Core/AssetsManager/AssetsManager.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Engine/Components/Core/Prefab.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	namespace Editor {
		class DropFileScene {
		  public:
			static void Drop(File* file) {
				if (file->extension == Standards::modelExtName) {
					AssetsLoader::LoadAsset(AssetsManager::GetAsset(file->directory));
				}
				if (file->extension == Standards::prefabExtName) {
					AssetsManager::GetPrefab(AssetsManager::GetAsset(file->directory)->mAssetUuid)
						->LoadToScene(Scene::GetActiveScene());
					Scene::GetActiveScene()->RecalculateAddedComponents();
				}
			}
		};
	} // namespace Editor
} // namespace Plaza

#ifndef PLAZA_SCENE_H
#define PLAZA_SCENE_H
#include <vector>
#include <variant>
#include <unordered_map>


//#include "Engine/Components/Core/Entity.h"


#include "Engine/Components/Rendering/Material.h"
#include "Engine/Core/RenderGroup.h"


#include <unordered_set>
#include "Engine/Core/Standards.h"
#include "Engine/Core/ComponentMapStructure.h"

#include "Engine/Components/Core/Camera.h"
#include "Engine/Components/Rendering/MeshRenderer.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Components/Rendering/Light.h"
#include "Engine/Components/Physics/RigidBody.h"
#include "Engine/Components/Physics/CharacterController.h"
#include "Engine/Components/Physics/Collider.h"
#include "Engine/Components/Scripting/CppScriptComponent.h"
#include "Engine/Components/Drawing/UI/TextRenderer.h"
#include "Engine/Components/Drawing/UI/Gui.h"
#include "Engine/Components/Audio/AudioSource.h"
#include "Engine/Components/Audio/AudioListener.h"
#include "Engine/Components/Rendering/AnimationComponent.h"

#include "Engine/Core/Renderer/Vulkan/Renderer.h"
//#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"

using namespace std;
namespace Plaza {
	struct VectorHash {
		template <class T>
		std::size_t operator()(const std::vector<T>& vector) const {
			std::size_t seed = vector.size();
			for (const auto& i : vector) {
				seed ^= std::hash<T>()(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};
	template <class T>
	struct VectorEqual {
		bool operator()(const std::vector<T>& lhs, const std::vector<T>& rhs) const {
			return lhs == rhs;
		}
	};

	struct Uint64Hash {
		size_t operator()(uint64_t key) const {
			uint64_t v = key * 3935559000370003845 + 2691343689449507681;

			v ^= v >> 21;
			v ^= v << 37;
			v ^= v >> 4;

			v *= 4768777513237032717;

			v ^= v << 20;
			v ^= v >> 41;
			v ^= v << 5;

			return v;
		}
	};

	struct RenderGroupKey
	{
		uint64_t first;
		uint64_t second;

		bool operator==(const RenderGroupKey& other) const
		{
			return (first == other.first
				&& second == other.second);
		}
	};

	struct PairHash {
		std::size_t operator () (const std::pair<uint64_t, std::vector<uint64_t>>& p) const {
			// Combine the hash values of the two uint64_t values
			std::size_t h1 = std::hash<uint64_t>{}(p.first);
			std::size_t h2 = p.second.size();
			for (const auto& i : p.second) {
				h2 ^= std::hash<uint64_t>()(i) + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
			}//VectorHash{}//std::hash<std::vector<uint64_t>>{}(p.second);
			return h1 ^ h2;
		}
	};

	class GameObjectList : public std::vector<std::unique_ptr<Entity>> {
	public:
		void push_back(std::unique_ptr<Entity> obj);
		Entity* find(std::string findName);
		Entity* find(uint64_t findUuid);
		~GameObjectList() = default;
	};
	class Scene : public Asset {
	public:
		bool mIsDeleting = false;

		std::unordered_map<std::variant<uint64_t, std::string>, Entity*> gameObjectsMap;

		Entity* mainSceneEntity = nullptr;
		uint64_t mainSceneEntityUuid;

		std::map<uint64_t, Animation*> mPlayingAnimations = std::map<uint64_t, Animation*>();

		std::unordered_map<uint64_t, Entity> entities;
		ComponentMultiMap<uint64_t, Transform> transformComponents;
		ComponentMultiMap<uint64_t, Camera> cameraComponents;
		ComponentMultiMap<uint64_t, MeshRenderer> meshRendererComponents;
		ComponentMultiMap<uint64_t, RigidBody> rigidBodyComponents;
		ComponentMultiMap<uint64_t, Collider> colliderComponents;
		ComponentMultiMap<uint64_t, CsScriptComponent> csScriptComponents;
		ComponentMultiMap<uint64_t, Plaza::Drawing::UI::TextRenderer> UITextRendererComponents;
		ComponentMultiMap<uint64_t, AudioSource> audioSourceComponents;
		ComponentMultiMap<uint64_t, AudioListener> audioListenerComponents;
		ComponentMultiMap<uint64_t, Light> lightComponents;
		ComponentMultiMap<uint64_t, CharacterController> characterControllerComponents;
		ComponentMultiMap<uint64_t, AnimationComponent> animationComponentComponents;
		ComponentMultiMap<uint64_t, GuiComponent> guiComponents;

		std::unordered_map<std::string, void*> componentsMap;

		std::vector<MeshRenderer*> meshRenderers;
		std::unordered_map<std::string, uint64_t> materialsNames;

		std::unordered_map<uint64_t, RenderGroup> renderGroups;
		std::unordered_map<std::pair<uint64_t, std::vector<uint64_t>>, uint64_t, PairHash> renderGroupsFindMap;
		std::unordered_map<uint64_t, uint64_t> renderGroupsFindMapWithMeshUuid;
		std::unordered_map<std::vector<uint64_t>, uint64_t, VectorHash, VectorEqual<uint64_t>> renderGroupsFindMapWithMaterialUuid;

		std::unordered_map<std::string, std::unordered_set<uint64_t>> entitiesNames;

		RenderGroup* AddRenderGroup(Mesh* newMesh, std::vector<Material*> newMaterials, bool resizeBuffer = true) {
			if (!newMesh)
				return nullptr;
			bool foundNewRenderGroup = this->renderGroupsFindMap.find(std::pair<uint64_t, std::vector<uint64_t>>(newMesh->uuid, Material::GetMaterialsUuids(newMaterials))) != this->renderGroupsFindMap.end();
			if (foundNewRenderGroup) {
				uint64_t uuid = this->renderGroupsFindMap.at(std::pair<uint64_t, std::vector<uint64_t>>(newMesh->uuid, Material::GetMaterialsUuids(newMaterials)));
				RenderGroup* renderGroup = &this->renderGroups.at(uuid);
				//renderGroup->mCount++;
				//if (resizeBuffer && renderGroup->mBufferSize < renderGroup->mCount)
				//	renderGroup->ResizeInstanceBuffer(0);
				return renderGroup;
			}
			else if (!foundNewRenderGroup)
			{
				RenderGroup newRenderGroup = RenderGroup(newMesh, newMaterials);
				newRenderGroup.mCount++;
				this->renderGroups.emplace(newRenderGroup.uuid, newRenderGroup);
				this->renderGroupsFindMapWithMeshUuid.emplace(newRenderGroup.mesh->uuid, newRenderGroup.uuid);
				//;;this->renderGroupsFindMapWithMaterialUuid.emplace(newRenderGroup->material->uuid, newRenderGroup->uuid);
				//;;this->renderGroupsFindMap.emplace(std::make_pair(newRenderGroup->mesh->uuid, newRenderGroup->material->uuid), newRenderGroup->uuid);
				return &this->renderGroups.at(newRenderGroup.uuid);
			}
		}

		RenderGroup* AddRenderGroup(RenderGroup* renderGroup) {
			return AddRenderGroup(renderGroup->mesh, renderGroup->materials);
		}

		RenderGroup* GetRenderGroupWithUuids(uint64_t meshUuid, std::vector<uint64_t> materialsUuid) {
			const auto& renderGroupIt = renderGroupsFindMap.find(std::make_pair(meshUuid, materialsUuid));
			if (renderGroupIt != this->renderGroupsFindMap.end())
				return &renderGroups.at(renderGroupIt->second);
			return nullptr;
		}

		template <typename MapType>
		void EraseFoundMap(MapType& map, typename MapType::key_type value) {
			if (map.find(value) != map.end()) {
				map.erase(map.find(value));
			}
		}

		void RemoveRenderGroup(uint64_t uuid) {
			if (this->renderGroups.find(uuid) != this->renderGroups.end())
			{
				RenderGroup* renderGroup = &this->renderGroups.at(uuid);
				EraseFoundMap(this->renderGroups, renderGroup->uuid);
				if (renderGroup->mesh)
					EraseFoundMap(this->renderGroupsFindMapWithMeshUuid, renderGroup->mesh->uuid);
				//if (renderGroup->material)
				//	EraseFoundMap(this->renderGroupsFindMapWithMaterialUuid, renderGroup->material->uuid);
				//if (renderGroup->mesh && renderGroup->material)
				//	EraseFoundMap(this->renderGroupsFindMap, std::pair<uint64_t, uint64_t>(renderGroup->mesh->uuid, renderGroup->material->uuid));
			}
		}

		Scene();
		Scene(Scene& other) = default;

		void InitMainEntity() {
			mainSceneEntity = new Entity("Scene");
			mainSceneEntityUuid = mainSceneEntity->uuid;
			mainSceneEntity->parentUuid = Scene::GetActiveScene()->mainSceneEntity->uuid;
		};

		void RecalculateAddedComponents();

		//unordered_map<uint64_t, Transform*> meshRenderersComponents;

		void Copy(Scene* baseScene);
		static void Play(); // Starts the game
		static void Stop(); // Finishes the game
		static void Pause(); // Just pauses the game
		~Scene() {
			mIsDeleting = true;
		};

		void RemoveEntity(uint64_t uuid);
		Entity* GetEntity(uint64_t uuid);
		Entity* GetEntityByName(std::string name);
		template<typename T>
		T* GetComponent(uint64_t uuid) {
			return GetEntity(uuid)->GetComponent<T>();
		}
		template<typename T>
		bool HasComponent(uint64_t uuid) {
			return GetEntity(uuid)->HasComponent<T>();
			//return map.find(uuid) != map.end();
		}
		template<typename T>
		void RemoveComponent(uint64_t uuid) {
			return GetEntity(uuid)->RemoveComponent<T>();
		}

		void RemoveMeshRenderer(uint64_t uuid);

		void RegisterMaps() {
			componentsMap["class Plaza::Transform"] = &transformComponents;
			componentsMap["class Plaza::Camera"] = &cameraComponents;
			componentsMap["class Plaza::MeshRenderer"] = &meshRendererComponents;
			componentsMap["class Plaza::RigidBody"] = &rigidBodyComponents;
			componentsMap["class Plaza::Collider"] = &colliderComponents;
			componentsMap["class Plaza::CsScriptComponent"] = &csScriptComponents;
			componentsMap["class Plaza::Drawing::UI::TextRenderer"] = &UITextRendererComponents;
			componentsMap["class Plaza::AudioSource"] = &audioSourceComponents;
			componentsMap["class Plaza::AudioListener"] = &audioListenerComponents;
			componentsMap["class Plaza::Light"] = &lightComponents;
			componentsMap["class Plaza::CharacterController"] = &characterControllerComponents;
			componentsMap["class Plaza::AnimationComponent"] = &animationComponentComponents;
			componentsMap["class Plaza::GuiComponent"] = &guiComponents;
		}

		template <class Archive>
		void serialize(Archive& archive) {
			if (mainSceneEntity)
				mainSceneEntityUuid = mainSceneEntity->uuid;

			archive(PL_SER(mAssetUuid), PL_SER(mAssetName), PL_SER(mainSceneEntityUuid), PL_SER(entities), PL_SER(transformComponents), PL_SER(cameraComponents), PL_SER(meshRendererComponents),
				PL_SER(rigidBodyComponents), PL_SER(colliderComponents), PL_SER(csScriptComponents), PL_SER(UITextRendererComponents), PL_SER(audioSourceComponents),
				PL_SER(audioListenerComponents), PL_SER(lightComponents), PL_SER(characterControllerComponents), PL_SER(animationComponentComponents), PL_SER(guiComponents));

			if (!mainSceneEntity)
				mainSceneEntity = &entities.at(mainSceneEntityUuid);

			/*
						archive(mAssetUuid, entities, transformComponents, cameraComponents, meshRendererComponents,
				rigidBodyComponents, colliderComponents, csScriptComponents, UITextRendererComponents, audioSourceComponents,
				audioListenerComponents, lightComponents, characterControllerComponents, animationComponentComponents, materials,
				materialsNames, renderGroups, renderGroupsFindMap, renderGroupsFindMapWithMeshUuid, renderGroupsFindMapWithMaterialUuid,
				entitiesNames);
			*/
		}

		static void InitializeScenes() {
			sEditorScene = std::make_shared<Scene>();
			sRuntimeScene = std::make_shared<Scene>();
		}

		static Scene* GetEditorScene() {
			return sEditorScene.get();
		}
		static void SetEditorScene(std::shared_ptr<Scene> scene) {
			sEditorScene = scene;
		}
		static void ClearEditorScene() {
			sEditorScene.reset();
			sEditorScene = std::make_shared<Scene>();
		}
		static Scene* GetRuntimeScene() {
			return sRuntimeScene.get();
		}
		static Scene* GetActiveScene() {
			return sActiveScene;
		}
		static void SetActiveScene(Scene* scene) {
			sActiveScene = scene;
		}
		static void NewRuntimeScene(Scene* baseScene);

		static void Terminate() {
			sActiveScene = nullptr;
			sEditorScene.reset();
			sRuntimeScene.reset();
		}
	private:
		static inline std::shared_ptr<Scene> sEditorScene = nullptr;
		static inline std::shared_ptr<Scene> sRuntimeScene = nullptr;
		static inline Scene* sActiveScene = nullptr;
	};
}
#endif //PLAZA_SCENE_H
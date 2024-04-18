#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsConverter.h"

namespace Plaza {
	SerializableMeshRenderer* ComponentsConverter::ConvertMeshRenderer(MeshRenderer* meshRenderer) {
		SerializableMeshRenderer* serializedMeshRenderer = new SerializableMeshRenderer();
		serializedMeshRenderer->uuid = meshRenderer->uuid;
		serializedMeshRenderer->type = SerializableComponentType::MESH_RENDERER;
		serializedMeshRenderer->materialUuid = meshRenderer->material->mAssetUuid;
		serializedMeshRenderer->serializedMesh = ComponentsConverter::ConvertMesh(meshRenderer->mesh);
		return serializedMeshRenderer;
	}
}
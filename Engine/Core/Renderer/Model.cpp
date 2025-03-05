#include "Engine/Core/PreCompiledHeaders.h"
#include "Model.h"
#include "Engine/Core/Renderer/Mesh.h"

namespace Plaza {
	Mesh* Model::GetMesh(uint64_t uuid) {
		if (mMeshes.find(uuid) != mMeshes.end()) {
			return mMeshes.at(uuid).get();
		}
		return nullptr;
	}

	void Model::AddMeshes(const std::vector<std::shared_ptr<Mesh>>& newMeshes) {
		for (const std::shared_ptr<Mesh>& mesh : newMeshes) {
			mMeshes.emplace(mesh->uuid, mesh);
		}
	}
} // namespace Plaza

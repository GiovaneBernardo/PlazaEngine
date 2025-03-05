#pragma once

namespace Plaza {
	class Mesh;
	class Model : public Asset {
	  public:
		Model() : Asset() { mAssetUuid = Plaza::UUID::NewUUID(); }

		Mesh* GetMesh(uint64_t uuid);
		void AddMeshes(const std::vector<std::shared_ptr<Mesh>>& newMeshes);

		std::unordered_map<uint64_t, std::shared_ptr<Mesh>> mMeshes =
			std::unordered_map<uint64_t, std::shared_ptr<Mesh>>();

		template <class Archive> void serialize(Archive& archive) { archive(cereal::base_class<Asset>(this), mMeshes); }

	  private:
	};
} // namespace Plaza

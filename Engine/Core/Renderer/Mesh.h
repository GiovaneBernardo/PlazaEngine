#pragma once
#include <unordered_set>
#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/cereal/cereal/types/string.hpp>
#include <ThirdParty/cereal/cereal/types/vector.hpp>
#include <ThirdParty/cereal/cereal/cereal.hpp>
#include <vector>
#include <array>
#include "Engine/Core/Engine.h"
#include <limits>

namespace Plaza {
	enum MeshType {
		Triangle = 0,
		HeightField
	};
	const unsigned int MAX_BONE_INFLUENCE = 4;
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;
		unsigned int materialIndex = -1;

		~Vertex() = default;

		Vertex(const glm::vec3& pos) : position(pos), normal(0.0f), texCoords(0.0f), tangent(0.0f) {}

		Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex, const glm::vec3& tan,
			   const unsigned int materialIndex = 0)
			: position(pos), normal(norm), texCoords(tex), tangent(tan), materialIndex(materialIndex) {}

		Vertex() : position(0.0f), normal(0.0f), texCoords(0.0f), tangent(0.0f) {}
	};

	struct SkinnedVertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;
		unsigned int materialIndex = -1;
		std::array<int, MAX_BONE_INFLUENCE> boneIds;
		std::array<float, MAX_BONE_INFLUENCE> weights;

		~SkinnedVertex() = default;

		SkinnedVertex(const glm::vec3& pos) : position(pos), normal(0.0f), texCoords(0.0f), tangent(0.0f) {
			for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
				boneIds[i] = -1;
				weights[i] = 0.0f;
			}
		}

		SkinnedVertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex, const glm::vec3& tan,
					  const unsigned int materialIndex = 0,
					  const std::array<int, MAX_BONE_INFLUENCE> boneIds = std::array<int, MAX_BONE_INFLUENCE>(),
					  const std::array<float, MAX_BONE_INFLUENCE> weights = std::array<float, MAX_BONE_INFLUENCE>())
			: position(pos), normal(norm), texCoords(tex), tangent(tan), materialIndex(materialIndex), boneIds(boneIds),
			  weights(weights) {}

		SkinnedVertex() : position(0.0f), normal(0.0f), texCoords(0.0f), tangent(0.0f) {
			for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
				boneIds[i] = -1;
				weights[i] = 0.0f;
			}
		}
	};

	struct PLAZA_API Bone {
		uint64_t mId = -1;
		uint64_t mParentId = -1;
		uint64_t mHandlerIndex = -1;
		std::string mName = "bone";
		Bone(uint64_t id, std::string name) : mId(id), mName(name){};
		Bone(uint64_t id, uint64_t parentId, uint64_t handlerIndex, std::string name,
			 glm::mat4 offset = glm::mat4(1.0f))
			: mId(id), mParentId(parentId), mHandlerIndex(handlerIndex), mName(name), mOffset(offset){};
		float weight = 0.0f;
		glm::vec3 mPosition = glm::vec3(0.0f);
		glm::vec3 mRotation = glm::vec3(0.0f);
		glm::vec3 mScale = glm::vec3(1.0f);
		glm::mat4 mLocalTransform = glm::mat4(1.0f);
		glm::mat4 mTransform = glm::mat4(1.0f);
		glm::mat4 mOffset = glm::mat4(1.0f);
		std::vector<uint64_t> mChildren = std::vector<uint64_t>();
		Bone(){};

		struct PLAZA_API Keyframe {
			double time;
			glm::vec3 position;
			glm::quat orientation;
			glm::vec3 scale;

			template <class Archive> void serialize(Archive& archive) {
				archive(PL_SER(time), PL_SER(position), PL_SER(orientation), PL_SER(scale));
			}
		};

		float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
			float scaleFactor = 0.0f;
			float midWayLength = animationTime - lastTimeStamp;
			float framesDiff = nextTimeStamp - lastTimeStamp;
			scaleFactor = midWayLength / framesDiff;
			return scaleFactor;
		}

		glm::mat4 InterpolatePosition(std::vector<Keyframe>& keyframes, float animationTime) {
			if (1 == keyframes.size())
				return glm::translate(glm::mat4(1.0f), keyframes[0].position);

			int p0Index = GetPositionIndex(keyframes, animationTime);
			int p1Index = p0Index + 1;
			float scaleFactor = GetScaleFactor(keyframes[p0Index].time, keyframes[p1Index].time, animationTime);
			glm::vec3 finalPosition = glm::mix(keyframes[p0Index].position, keyframes[p1Index].position, scaleFactor);
			return glm::translate(glm::mat4(1.0f), finalPosition);
		}

		glm::mat4 InterpolateRotation(std::vector<Keyframe>& keyframes, float animationTime) {
			if (1 == keyframes.size()) {
				auto rotation = glm::normalize(keyframes[0].orientation);
				return glm::toMat4(rotation);
			}

			int p0Index = GetRotationIndex(keyframes, animationTime);
			int p1Index = p0Index + 1;
			float scaleFactor = GetScaleFactor(keyframes[p0Index].time, keyframes[p1Index].time, animationTime);
			glm::quat finalRotation =
				glm::slerp(keyframes[p0Index].orientation, keyframes[p1Index].orientation, scaleFactor);
			finalRotation = glm::normalize(finalRotation);
			return glm::toMat4(finalRotation);
		}

		glm::mat4 InterpolateScaling(std::vector<Keyframe>& keyframes, float animationTime) {
			if (1 == keyframes.size())
				return glm::scale(glm::mat4(1.0f), keyframes[0].scale);

			int p0Index = GetScaleIndex(keyframes, animationTime);
			int p1Index = p0Index + 1;
			float scaleFactor = GetScaleFactor(keyframes[p0Index].time, keyframes[p1Index].time, animationTime);
			glm::vec3 finalScale = glm::mix(keyframes[p0Index].scale, keyframes[p1Index].scale, scaleFactor);
			return glm::scale(glm::mat4(1.0f), finalScale);
		}

		void Update(std::vector<Keyframe>& keyframes, float animationTime) {
			glm::mat4 translation = InterpolatePosition(keyframes, animationTime);
			glm::mat4 rotation = InterpolateRotation(keyframes, animationTime);
			glm::mat4 scale = InterpolateScaling(keyframes, animationTime);
			mLocalTransform = translation * rotation * scale;
		}

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mId), PL_SER(mName), PL_SER(weight), PL_SER(mPosition), PL_SER(mRotation), PL_SER(mScale),
					PL_SER(mOffset), PL_SER(mParentId), PL_SER(mChildren));
		}

	  private:
		int GetPositionIndex(std::vector<Keyframe>& keyframes, float animationTime) {
			for (int index = 0; index < keyframes.size() - 1; ++index) {
				if (animationTime < keyframes[index + 1].time)
					return index;
			}
			return 0;
		}

		int GetRotationIndex(std::vector<Keyframe>& keyframes, float animationTime) {
			for (int index = 0; index < keyframes.size() - 1; ++index) {
				if (animationTime < keyframes[index + 1].time)
					return index;
			}
			return 0;
		}

		int GetScaleIndex(std::vector<Keyframe>& keyframes, float animationTime) {
			for (int index = 0; index < keyframes.size() - 1; ++index) {
				if (animationTime < keyframes[index + 1].time)
					return index;
			}
			return 0;
		}
	};

	struct PLAZA_API BonesHolder {
		std::vector<uint64_t> mBones = std::vector<uint64_t>();
		std::vector<float> mWeights = std::vector<float>();
		// std::vector<unsigned int> mBonesIds = std::vector<unsigned int>();

		std::array<int64_t, MAX_BONE_INFLUENCE> GetBoneIds() {
			std::array<int64_t, MAX_BONE_INFLUENCE> bonesArray = std::array<int64_t, MAX_BONE_INFLUENCE>();
			for (unsigned int i = 0; i < mBones.size() && i < MAX_BONE_INFLUENCE; ++i) {
				bonesArray[i] = this->mBones[i];
			}
			return bonesArray;
		}
		std::array<float, MAX_BONE_INFLUENCE> GetBoneWeights() const {
			std::array<float, MAX_BONE_INFLUENCE> weightArray = std::array<float, MAX_BONE_INFLUENCE>();
			for (unsigned int i = 0; i < mBones.size() && i < MAX_BONE_INFLUENCE; ++i) {
				weightArray[i] = this->mWeights[i];
			}
			return weightArray;
		}

		template <class Archive> void serialize(Archive& archive) { archive(PL_SER(mBones), PL_SER(mWeights)); }
	};

	struct PLAZA_API BoundingBox {
		glm::vec4 maxVector = glm::vec4(glm::vec3(std::numeric_limits<float>::min()), 1.0f);
		glm::vec4 minVector = glm::vec4(glm::vec3(std::numeric_limits<float>::max()), 1.0f);
	};

	class PLAZA_API Mesh {
	  public:
		BoundingBox mBoundingBox{};

		MeshType meshType = MeshType::Triangle;
		bool temporaryMesh = false;
		bool mImportedMesh = false;
		std::vector<glm::mat4> instanceModelMatrices = std::vector<glm::mat4>();
		uint64_t uuid;
		float farthestVertex = 0.0f;
		std::string id;
		std::string meshName;
		uint64_t modelUuid;

		std::vector<glm::vec3> vertices = std::vector<glm::vec3>();
		std::vector<unsigned int> indices = std::vector<unsigned int>();
		std::vector<glm::vec3> normals = std::vector<glm::vec3>();
		std::vector<glm::vec2> uvs = std::vector<glm::vec2>();
		std::vector<glm::vec3> tangent = std::vector<glm::vec3>();
		std::vector<unsigned int> materialsIndices = std::vector<unsigned int>();
		std::vector<BonesHolder> bonesHolder;
		std::map<uint64_t, Bone> uniqueBonesInfo = std::map<uint64_t, Bone>();

		// vector<std::array<int, MAX_BONE_INFLUENCE>> boneIds;
		// vector<std::array<float, MAX_BONE_INFLUENCE>> weights;

		uint64_t verticesCount = 0;
		uint64_t verticesOffset = 0;
		uint64_t indicesCount = 0;
		uint64_t indicesOffset = 0;
		uint64_t instanceOffset = 0;

		// vector<Texture> textures;
		// Material material = DefaultMaterial();
		bool usingNormal;
		glm::vec4 infVec = glm::vec4(INFINITY);

		virtual void setupMesh() {};
		virtual void Restart() {};
		virtual void Draw() {};
		virtual void DrawInstances() {};

		Mesh() { this->uuid = Plaza::UUID::NewUUID(); }

		void AddInstance(glm::mat4 model) { instanceModelMatrices.push_back(model); }

		void CalculateBoundingBox() {
			for (glm::vec3 vertex : this->vertices) {
				CalculateVertexInBoundingBox(vertex);
			}
		}

		void CalculateVertexInBoundingBox(const glm::vec3& vertex) {
			mBoundingBox.minVector.x = std::min(mBoundingBox.minVector.x, vertex.x);
			mBoundingBox.minVector.y = std::min(mBoundingBox.minVector.y, vertex.y);
			mBoundingBox.minVector.z = std::min(mBoundingBox.minVector.z, vertex.z);

			mBoundingBox.maxVector.x = std::max(mBoundingBox.maxVector.x, vertex.x);
			mBoundingBox.maxVector.y = std::max(mBoundingBox.maxVector.y, vertex.y);
			mBoundingBox.maxVector.z = std::max(mBoundingBox.maxVector.z, vertex.z);
		}

		void CalculateTangent() {
			for (size_t i = 0; i < indices.size(); i += 3) {
				// Get the indices of the triangle vertices
				const unsigned int& i0 = indices[i];
				const unsigned int& i1 = indices[i + 1];
				const unsigned int& i2 = indices[i + 2];

				// Get the vertices and UVs of the triangle
				glm::vec3 v0 = vertices[i0];
				glm::vec3 v1 = vertices[i1];
				glm::vec3 v2 = vertices[i2];

				glm::vec2 uv0 = uvs[i0];
				glm::vec2 uv1 = uvs[i1];
				glm::vec2 uv2 = uvs[i2];

				// Edges of the triangle
				glm::vec3 deltaPos1 = v1 - v0;
				glm::vec3 deltaPos2 = v2 - v0;

				// UV delta
				glm::vec2 deltaUV1 = uv1 - uv0;
				glm::vec2 deltaUV2 = uv2 - uv0;

				// Calculate the tangent
				float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
				glm::vec3 tangentVec = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

				// Store the tangent for each vertex of the triangle
				tangent[i0] += tangentVec;
				tangent[i1] += tangentVec;
				tangent[i2] += tangentVec;
			}

			// Normalize tangents
			for (size_t i = 0; i < tangent.size(); ++i) {
				tangent[i] = glm::normalize(tangent[i]);
			}
		}

		template <class Archive> void serialize(Archive& archive) {
			archive(uuid, mImportedMesh, id, meshName, modelUuid, vertices, indices, normals, uvs, materialsIndices,
					bonesHolder, uniqueBonesInfo);
			verticesCount = vertices.size();
			indicesCount = indices.size();
		}
	};
} // namespace Plaza

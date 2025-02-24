#pragma once
//#include "Engine/Core/ModelLoader/Model.h"

#ifdef _WIN32
#ifdef ENGINE_EXPORT
#define PLAZA_API __declspec(dllexport)
#define GAME_API __declspec(dllimport)
#elif ENGINE_IMPORT
#define PLAZA_API __declspec(dllimport)
#define GAME_API __declspec(dllexport)
#else
#define PLAZA_API 
#define GAME_API 
#endif
#else
#define PLAZA_API __attribute__((visibility("default")))
#define GAME_API __attribute__((visibility("default")))
#endif

#include <ThirdParty/cereal/cereal/archives/json.hpp>
#include <ThirdParty/cereal/cereal/types/string.hpp>
#include <ThirdParty/cereal/cereal/types/vector.hpp>
#include <ThirdParty/cereal/cereal/cereal.hpp>
#include <ThirdParty/cereal/cereal/archives/binary.hpp>
#include <ThirdParty/cereal/cereal/types/map.hpp>
#include <ThirdParty/cereal/cereal/types/polymorphic.hpp>
#include <ThirdParty/cereal/cereal/types/utility.hpp>
#include <unordered_map>
#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtx/quaternion.hpp>

namespace Plaza {
#define PL_SER(T) CEREAL_NVP(T)
#define PL_SER_REGISTER_TYPE(T) CEREAL_REGISTER_TYPE(T)
#define PL_SER_REGISTER_POLYMORPHIC_RELATION(A, B) CEREAL_REGISTER_POLYMORPHIC_RELATION(A, B)
}

namespace cereal {
	template <class Archive, typename T, std::size_t N>
	void serialize(Archive& archive, std::array<T, N>& arr) {
		for (auto& elem : arr) {
			archive(elem);
		}
	}

	template <class Archive> void serialize(Archive& archive, glm::vec2& v) {
		archive(v.x, v.y);
	}

	template <class Archive> void serialize(Archive& archive, glm::vec3& v) {
		archive(v.x, v.y, v.z);
	}

	template <class Archive> void serialize(Archive& archive, glm::vec4& v) {
		archive(v.x, v.y, v.z, v.w);
	}

	template <class Archive> void serialize(Archive& archive, glm::quat& q) {
		archive(q.w, q.x, q.y, q.z);
	}

	template <class Archive> void serialize(Archive& archive, glm::mat4& mat) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				archive(cereal::make_nvp("m" + std::to_string(i) + std::to_string(j), mat[i][j]));
			}
		}
	}
}

namespace Plaza {
	class EngineClass {
	public:
		//static std::unordered_map<uint64_t, std::unique_ptr<Model>> models;
	};
}
// inline std::unordered_map<uint64_t, std::unique_ptr<Model>> EngineClass::models = std::unordered_map<uint64_t, std::unique_ptr<Model>>();
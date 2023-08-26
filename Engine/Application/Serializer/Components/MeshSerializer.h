#pragma once
#include "ComponentSerializer.h"
namespace Plaza {
	class ComponentSerializer::MeshSerializer {
	public:
		static void Serialize(YAML::Emitter& out, Mesh& mesh);
		static void DeSerialize(std::string path);
	};
}
#pragma once
#include "Engine/Core/AssetsManager/AssetsType.h"
#include "Engine/Core/AssetsManager/AssetsManager.h"
#include <ThirdParty/cereal/cereal/archives/json.hpp>
#include <ThirdParty/cereal/cereal/types/string.hpp>
#include <ThirdParty/cereal/cereal/types/vector.hpp>
#include <ThirdParty/cereal/cereal/cereal.hpp>
#include <ThirdParty/cereal/cereal/archives/binary.hpp>
#include <ThirdParty/cereal/cereal/types/map.hpp>
#include <ThirdParty/cereal/cereal/types/polymorphic.hpp>
#include <ThirdParty/cereal/cereal/types/utility.hpp>
#include "Engine/Application/EngineSettings.h"
#include "Engine/Core/AssetsManager/Asset.h"
#include "Engine/Components/Rendering/AnimationComponent.h"
#include "Engine/Components/Rendering/Material.h"

namespace Plaza {
	class AssetsSerializer {
	  public:
		static void SerializeMaterial(Material* material, std::filesystem::path outPath,
									  SerializationMode serializationMode);
		static void SerializeMesh(Mesh* mesh, std::filesystem::path outPath, SerializationMode serializationMode);
		static void SerializeAnimation(Animation& animation, std::filesystem::path outPath,
									   SerializationMode serializationMode);

		static void SerializeAssetByExtension(Asset* asset);

		template <typename T>
		static inline void SerializeFile(T& file, std::string path, SerializationMode serializationMode) {
			switch (serializationMode) {
				case SerializationMode::SERIALIZE_BINARY:
					AssetsSerializer::SerializeAsBinary<T>(file, path);
					break;
				case SerializationMode::SERIALIZE_JSON:
					AssetsSerializer::SerializeAsJson<T>(file, path);
					break;
			}
		}

		template <typename T>
		static inline std::shared_ptr<T> DeSerializeFile(std::string path, SerializationMode serializationMode) {
			switch (serializationMode) {
				case SerializationMode::SERIALIZE_BINARY:
					return AssetsSerializer::DeSerializeAsBinary<T>(path);
					break;
				case SerializationMode::SERIALIZE_JSON:
					return AssetsSerializer::DeSerializeAsJson<T>(path);
					break;
			}
		}

	  private:
		template <typename T> static inline void SerializeAsBinary(T& file, const std::string& path) {
			std::ofstream os(path, std::ios::binary);
			cereal::BinaryOutputArchive archive(os);
			archive(file);
			os.close();
		}

		template <typename T> static inline std::shared_ptr<T> DeSerializeAsBinary(const std::string& path) {
			std::ifstream is(path, std::ios::binary);
			cereal::BinaryInputArchive archive(is);
			std::shared_ptr<T> file = std::make_shared<T>();
			archive(*file.get());
			is.close();
			static_cast<Asset*>(file.get())->mAssetPath = std::filesystem::path{path};
			return file;
		}

		template <typename T> static inline void SerializeAsJson(T& file, const std::string& path) {
			std::ofstream os(path);
			{
				cereal::JSONOutputArchive archive(os);
				archive(file);
			}
			os.close();
		}

		template <typename T> static inline std::shared_ptr<T> DeSerializeAsJson(const std::string& path) {
			std::ifstream is(path);
			cereal::JSONInputArchive archive(is);
			std::shared_ptr<T> file = std::make_shared<T>();
			archive(*file.get());
			is.close();
			static_cast<Asset*>(file.get())->mAssetPath = std::filesystem::path{path};
			return file;
		}
	};
} // namespace Plaza

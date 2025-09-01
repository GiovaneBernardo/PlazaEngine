#pragma once

namespace Plaza {
	class PlTextureSampler;
	enum PlRenderStage;
	class PlBuffer;
	class ShaderReflection {
	public:
		struct ReflectedBinding {
			uint32_t reflectedType;
			std::string name;
			uint32_t set;
			uint32_t binding;
			uint32_t descriptorType;
			uint32_t descriptorCount;
			uint32_t stageFlags;
			uint32_t offset;
			uint32_t size;
			uint32_t inputAttachmentIndex;
			bool read;
			bool write;
			std::shared_ptr<Texture> texture;
			std::shared_ptr<PlBuffer> buffer;
			std::shared_ptr<PlTextureSampler> sampler;
		};

		enum ShaderType {
			PL_VERTEX_SHADER,
			PL_PIXEL_SHADER,
			PL_GEOMETRY_SHADER,
			PL_TESSELATION_SHADER,
			PL_COMPUTE_SHADER
		};
		struct ShaderBinding {

		};
		struct Shader {
			std::string mEntryName;
			ShaderType mShaderType;
			std::vector<ReflectedBinding> mReflectedBindings;
			std::set<std::string> mExtensions;
			std::vector<std::vector<uint32_t>> mShadersData;
		};

		enum ReflectedType {
			PL_REFLECTED_TYPE_NONE,
			PL_REFLECTED_TYPE_UNIFORM_BUFFER,
			PL_REFLECTED_TYPE_STORAGE_BUFFER,
			PL_REFLECTED_TYPE_STAGE_INPUT,
			PL_REFLECTED_TYPE_STAGE_OUTPUT,
			PL_REFLECTED_TYPE_SUBPASS_INPUT,
			PL_REFLECTED_TYPE_SUBPASS_OUTPUT,
			PL_REFLECTED_TYPE_STORAGE_IMAGE,
			PL_REFLECTED_TYPE_SAMPLED_IMAGE,
			PL_REFLECTED_TYPE_ATOMIC_COUNTER,
			PL_REFLECTED_TYPE_ACCELERATION_STRUCTURE,
			PL_REFLECTED_TYPE_PLAIN_UNIFORMS,
			PL_REFLECTED_TYPE_TENSORS,
			PL_REFLECTED_TYPE_PUSH_CONSTANT_BUFFER,
			PL_REFLECTED_TYPE_SHADER_RECORD_BUFFER,
			PL_REFLECTED_TYPE_SEPARATE_IMAGE,
			PL_REFLECTED_TYPE_SEPARATE_SAMPLER,
			PL_REFLECTED_TYPE_BUILTIN_INPUT,
			PL_REFLECTED_TYPE_BUILTIN_OUTPUT,
			PL_REFLECTED_TYPE_MAX_ENUM
		};

		static std::filesystem::path CompileHlsl(std::filesystem::path path, const std::string& entryName,
			ShaderType shaderType, std::string outDirectory = "", const std::set<std::string>& extensions = {});
		static std::vector<uint32_t> ReadSpirVBinary(const std::filesystem::path& path);
		static void ReflectShaderBindings(ShaderReflection::Shader& shader, const std::vector<uint32_t>& spirVBinary, ShaderType stage);
		static std::vector<Shader> GetShadersFromSpirV(const std::vector<uint32_t>& spirVBinary);
		static std::vector<Shader> GetShadersFromHlsl(std::filesystem::path hlslPath);
		static std::set<std::string> ParsePragmaExtensions(const std::string& hlslSource);

		static PlRenderStage ShaderTypeToPlRenderStageFlags(ShaderType shaderType);
	};
}
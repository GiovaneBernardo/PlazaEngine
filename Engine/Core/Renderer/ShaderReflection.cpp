#include "Engine/Core/PreCompiledHeaders.h"
#include "ShaderReflection.h"
#include "Thirdparty/spirv-cross/spirv_cpp.hpp"
#include "Thirdparty/spirv-cross/spirv_cross.hpp"
#include "RendererTypes.h"

#include <regex>

namespace Plaza {
	std::string ExecCommand(const std::string& command) {
		std::array<char, 128> buffer;
		std::string result;

#ifdef _WIN32
		std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen((command + " 2>&1").c_str(), "r"), _pclose);
#else
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen((command + " 2>&1").c_str(), "r"), pclose);
#endif

		if (!pipe)
			throw std::runtime_error("Failed to open pipe");

		while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
			result += buffer.data();
		}

		return result;
	}

	std::filesystem::path ShaderReflection::CompileHlsl(std::filesystem::path path, const std::string& entryName,
														ShaderType shaderType, std::string outDirectory,
														const std::set<std::string>& extensions) {
		if (path.is_relative())
			path = FilesManager::sEngineFolder / "Shaders" / path;

		if (outDirectory.empty())
			outDirectory = FilesManager::sEngineExecutablePath.string() + "/CompiledShaders/";

		std::filesystem::path dxcPath = FilesManager::sEngineFolder / "../ThirdParty/dxc/dxc.exe";

		// Ensure output directory exists
		if (!std::filesystem::exists(std::filesystem::path{outDirectory}))
			std::filesystem::create_directory(outDirectory);
		std::string shadersName = std::filesystem::path{path}.filename().string();

		// Get the .spv path
		std::filesystem::path outputPath =
			std::filesystem::path(outDirectory + std::filesystem::path(shadersName).stem().string() + ".spv");

		// Only compile .hlsl to .spv only if in editor mode, games should ship the .spv and not .hlsl
#ifdef EDITOR_MODE
		std::string shaderVersionString;
		if (shaderType == ShaderType::PL_VERTEX_SHADER)
			shaderVersionString = "vs_6_9";
		else if (shaderType == ShaderType::PL_PIXEL_SHADER)
			shaderVersionString = "ps_6_9";
		else if (shaderType == ShaderType::PL_COMPUTE_SHADER)
			shaderVersionString = "cs_6_9";
		// TODO: Add other shader types

		std::string cmd = dxcPath.string() + " -T " + shaderVersionString + " -E " + entryName +
						  " -spirv -fspv-target-env=vulkan1.2 -fspv-reflect "
						  "-fspv-extension=SPV_GOOGLE_hlsl_functionality1 -fspv-extension=SPV_GOOGLE_user_type";
		cmd += " -I \"" + (FilesManager::sEngineFolder / "Shaders").string() + "\"";

		for (const auto& ext : extensions)
			cmd += " -fspv-extension=" + ext;

		cmd += " -Fo " + outputPath.string() + " " + path.string();
		std::string result = ExecCommand(cmd.c_str());

		if (!result.empty()) {
			std::cerr << "Shader compilation failed: " << result << std::endl;
		}
#endif EDITOR_MODE
		return outputPath;
	}

	std::vector<uint32_t> ShaderReflection::ReadSpirVBinary(const std::filesystem::path& path) {
		std::ifstream file(path.string(), std::ios::binary | std::ios::ate);
		if (!file.is_open())
			throw std::runtime_error("Failed to open file");
		size_t size = file.tellg();
		if (size % 4 != 0)
			throw std::runtime_error("SPIR-V size invalid");
		std::vector<uint32_t> data(size / 4);
		file.seekg(0);
		file.read(reinterpret_cast<char*>(data.data()), size);
		file.close();
		return data;
	}

	void ShaderReflection::ReflectShaderBindings(ShaderReflection::Shader& shader,
												 const std::vector<uint32_t>& spirVBinary, ShaderType stage) {
		try {
			spirv_cross::Compiler compiler(spirVBinary);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();

			std::vector<ReflectedBinding> reflectedBindings;

			auto GetStageFlag = [](ShaderType type) -> uint32_t {
				switch (type) {
					case ShaderType::PL_VERTEX_SHADER:
						return PL_STAGE_VERTEX;
					case ShaderType::PL_PIXEL_SHADER:
						return PL_STAGE_FRAGMENT;
					case ShaderType::PL_COMPUTE_SHADER:
						return PL_STAGE_COMPUTE;
					default:
						return 0;
				}
			};

			uint32_t stageFlag = GetStageFlag(stage);

			for (const auto& ubo : resources.uniform_buffers) {
				auto set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
				auto binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
				std::string name = compiler.get_name(ubo.id);

				reflectedBindings.push_back({PL_REFLECTED_TYPE_UNIFORM_BUFFER, name, set, binding,
											 PL_BUFFER_UNIFORM_BUFFER, 1, stageFlag, 0, 0, 0, true, false});
			}

			for (const auto& ssbo : resources.storage_buffers) {
				auto set = compiler.get_decoration(ssbo.id, spv::DecorationDescriptorSet);
				auto binding = compiler.get_decoration(ssbo.id, spv::DecorationBinding);
				std::string name = compiler.get_name(ssbo.id);
				auto flags = compiler.get_buffer_block_flags(ssbo.id);
				bool isWritable = !flags.get(spv::DecorationNonWritable);

				reflectedBindings.push_back({PL_REFLECTED_TYPE_STORAGE_BUFFER, name, set, binding,
											 PL_BUFFER_STORAGE_BUFFER, 1, stageFlag, 0, 0, 0, true, isWritable});
			}

			for (const auto& image : resources.storage_images) {
				auto set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
				auto binding = compiler.get_decoration(image.id, spv::DecorationBinding);
				std::string name = compiler.get_name(image.id);
				bool isWritable = !compiler.get_type(image.type_id).image.sampled;

				reflectedBindings.push_back({PL_REFLECTED_TYPE_STORAGE_IMAGE, name, set, binding,
											 PL_BUFFER_STORAGE_IMAGE, 1, stageFlag, 0, 0, 0, true, isWritable});
			}

			for (const auto& sampledImage : resources.sampled_images) {
				auto set = compiler.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
				auto binding = compiler.get_decoration(sampledImage.id, spv::DecorationBinding);
				std::string name = compiler.get_name(sampledImage.id);

				reflectedBindings.push_back({PL_REFLECTED_TYPE_SAMPLED_IMAGE, name, set, binding,
											 PL_BUFFER_SAMPLED_IMAGE, 1, stageFlag, 0, 0, 0, true, false});
			}

			for (const auto& image : resources.separate_images) {
				auto set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
				auto binding = compiler.get_decoration(image.id, spv::DecorationBinding);
				std::string name = compiler.get_name(image.id);

				reflectedBindings.push_back({PL_REFLECTED_TYPE_SEPARATE_IMAGE, name, set, binding,
											 PL_BUFFER_COMBINED_IMAGE_SAMPLER, 1, stageFlag, 0, 0, 0, true, false});
			}

			for (const auto& sampler : resources.separate_samplers) {
				auto set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);
				auto binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
				std::string name = compiler.get_name(sampler.id);

				reflectedBindings.push_back({PL_REFLECTED_TYPE_SEPARATE_SAMPLER, name, set, binding,
											 PL_BUFFER_SAMPLER, 1, stageFlag, 0, 0, 0, true, false});
			}

			if (stageFlag == PL_STAGE_FRAGMENT) {
				for (const auto& sampledImage : resources.stage_outputs) {
					auto set = compiler.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
					auto binding = compiler.get_decoration(sampledImage.id, spv::DecorationBinding);
					std::string name = compiler.get_name(sampledImage.id);

					reflectedBindings.push_back({PL_REFLECTED_TYPE_SAMPLED_IMAGE, name, set, binding,
												 PL_BUFFER_SAMPLED_IMAGE, 1, stageFlag, 0, 0, 0, false, true});
				}
			}

			shader.mReflectedBindings = reflectedBindings;

		} catch (const spirv_cross::CompilerError& e) {
			std::cerr << "SPIRV-Cross error: " << e.what() << std::endl;
		}
	}

	std::set<std::string> ShaderReflection::ParsePragmaExtensions(const std::string& hlslSource) {
		std::set<std::string> extensions;
		std::regex pragmaRegex(R"(^\s*#pragma\s+extension\s+([A-Za-z0-9_]+))", std::regex::icase);
		std::smatch match;

		auto begin = std::sregex_iterator(hlslSource.begin(), hlslSource.end(), pragmaRegex);
		auto end = std::sregex_iterator();

		for (auto it = begin; it != end; ++it) {
			std::string ext = (*it)[1].str();
			extensions.insert(ext);
		}
		return extensions;
	}

	std::vector<ShaderReflection::Shader> ShaderReflection::GetShadersFromSpirV(
		const std::vector<uint32_t>& spirVBinary) {
		std::vector<ShaderReflection::Shader> shaders;
		return shaders;
	}

	std::string ReadHlslWithIncludes(const std::filesystem::path& filePath, const std::vector<std::filesystem::path>& includeDirs, std::set<std::filesystem::path>& seenFiles)
	{
		if (!std::filesystem::exists(filePath))
			throw std::runtime_error("File not found: " + filePath.string());

		if (seenFiles.find(filePath) != seenFiles.end())
			return ""; // prevent cyclic includes

		seenFiles.insert(filePath);

		std::ifstream file(filePath);
		if (!file.is_open())
			throw std::runtime_error("Failed to open file: " + filePath.string());

		std::string result;
		std::string line;
		while (std::getline(file, line))
		{
			std::smatch match;
			static std::regex includeRegex("^\\s*#include\\s+\"(.+?)\"");
			if (std::regex_search(line, match, includeRegex))
			{
				std::filesystem::path includePath = match[1].str();
				// Try relative to current file
				std::filesystem::path fullInclude = filePath.parent_path() / includePath;
				if (!std::filesystem::exists(fullInclude))
				{
					// Try include directories
					bool found = false;
					for (auto& dir : includeDirs)
					{
						fullInclude = dir / includePath;
						if (std::filesystem::exists(fullInclude))
						{
							found = true;
							break;
						}
					}
					if (!found)
						throw std::runtime_error("Included file not found: " + includePath.string());
				}
				result += ReadHlslWithIncludes(fullInclude, includeDirs, seenFiles);
			}
			else
			{
				result += line + "\n";
			}
		}

		return result;
	}

	std::vector<ShaderReflection::Shader> ShaderReflection::GetShadersFromHlsl(std::filesystem::path hlslPath) {
		if (!std::filesystem::exists(hlslPath) && hlslPath.is_relative())
			hlslPath = FilesManager::sEngineFolder / "Shaders" / hlslPath;

		if (!std::filesystem::exists(hlslPath))
			throw std::runtime_error("Shader file does not exist: " + hlslPath.string());

		// Read full shader source with includes
		std::vector<std::filesystem::path> includeDirs = {
			FilesManager::sEngineFolder / "Shaders"
		};

		std::set<std::filesystem::path> seenFiles;
		std::string source = ReadHlslWithIncludes(hlslPath, includeDirs, seenFiles);

		std::vector<ShaderReflection::Shader> entries;

		// Look for lines like: float4 mainVS(...) : SV_Target
		std::regex functionRegex(R"(\b(\w+)\s+(\w+)\s*\(.*?\))"); // returnType name(args)
		std::smatch match;

		auto it = std::sregex_iterator(source.begin(), source.end(), functionRegex);
		auto end = std::sregex_iterator();

		std::set<std::string> extensions = ParsePragmaExtensions(source);
		std::set<std::string> addedFuncNames = std::set<std::string>();
		for (; it != end; ++it) {
			std::string funcName = (*it)[2].str();
			if (addedFuncNames.find(funcName) != addedFuncNames.end())
				continue;
			addedFuncNames.emplace(funcName);

			if (funcName.find("VS") != std::string::npos) {
				entries.emplace_back(ShaderReflection::Shader(funcName, ShaderType::PL_VERTEX_SHADER, {}, extensions));
			}
			else if (funcName.find("PS") != std::string::npos) {
				entries.emplace_back(ShaderReflection::Shader(funcName, ShaderType::PL_PIXEL_SHADER, {}, extensions));
			}
			else if (funcName.find("CS") != std::string::npos) {
				entries.emplace_back(ShaderReflection::Shader(funcName, ShaderType::PL_COMPUTE_SHADER, {}, extensions));
			}
			// Add more as needed (GS, HS, DS...)
		}

		return entries;
	}

	PlRenderStage ShaderReflection::ShaderTypeToPlRenderStageFlags(ShaderType shaderType) {
		switch (shaderType) {
			case PL_VERTEX_SHADER:
				return PL_STAGE_VERTEX;
				break;
			case PL_PIXEL_SHADER:
				return PL_STAGE_FRAGMENT;
				break;
			case PL_COMPUTE_SHADER:
				return PL_STAGE_COMPUTE;
				break;
			default:
				return PL_STAGE_ALL;
				break;
		}
	}

} // namespace Plaza
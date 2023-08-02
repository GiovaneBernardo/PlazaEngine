#include "Engine/Core/PreCompiledHeaders.h"
#include "ModelImporter.h"
#include "Engine/Core/ModelLoader/ModelLoader.h"
#include "Engine/Core/ModelLoader/Model.h"
#include "Engine/Core/Engine.h"
#include "Editor/GUI/FileExplorer/CreateFolder.h"
namespace Engine::Editor {
	vector<aiTextureType> types = { aiTextureType_DIFFUSE, aiTextureType_HEIGHT, aiTextureType_AMBIENT, aiTextureType_NORMALS, aiTextureType_SPECULAR };
	void ModelImporter::ImportModel(string const& path, std::string modelName, std::string extension, std::string modelPath) {
		vector<string> loadedTextures;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		aiNode* node = scene->mRootNode;
		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[i];
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			vector<string> textures;
			for (aiTextureType type : types) {
				for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
				{
					aiString str;
					material->GetTexture(type, i, &str);
					string fileName = std::filesystem::path(str.C_Str()).filename().string();
					bool skip = false;
					for (unsigned int j = 0; j < loadedTextures.size(); j++)
					{
						if (loadedTextures[j] == fileName) {
							textures.push_back(loadedTextures[j]);
							skip = true;
							break;
						}
					}
					if (!skip)
					{   // Load a new texture
						loadedTextures.push_back(fileName);
						textures.push_back(fileName);
					}

				}
			}
		}
		string modelPathInEngine = Editor::CreateFolder(path, modelName);
		string sourcesFolderPath = Editor::CreateFolder(modelPathInEngine, "sources");
		string texturesFolderPath = Editor::CreateFolder(modelPathInEngine, "textures");
		string texturesOriginalPath = std::filesystem::path(modelPath).parent_path().string();
		string isFbx = std::filesystem::path(modelPath).filename().extension() == ".fbx" ? "Textures\\" : "";
		for (string texturePath : loadedTextures) {
			//std::filesystem::copy(texturesOriginalPath + "\\" + isFbx + texturePath, texturesFolderPath + "\\" + texturePath);
		}
		//std::filesystem::copy(modelPath, sourcesFolderPath + "\\" + modelName + extension);
		ModelLoader::LoadModelToGame(modelPath, modelName, scene);
	}
}

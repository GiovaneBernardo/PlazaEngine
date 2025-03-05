#include "Engine/Core/PreCompiledHeaders.h"
#include "DefaultModels.h"
#include "Engine/Core/Physics.h"
#include "Engine/Core/Renderer/Renderer.h"
namespace Plaza::Editor {

	vector<Mesh*> DefaultModels::meshes = vector<Mesh*>();
	uint64_t DefaultModels::cubeUuid = 1;
	uint64_t DefaultModels::sphereUuid = 2;
	uint64_t DefaultModels::planeUuid = 3;
	uint64_t DefaultModels::cylinderUuid = 4;
	uint64_t DefaultModels::capsuleUuid = 5;

	void DefaultModels::Init() {
		InitCube();
		InitSphere();
		InitPlane();
		InitCylinder();
		InitCapsule(0.5f, 1.0f, 64, 64);

		Physics::InitDefaultGeometries();
		// Material defaultMaterial = Material();
		// defaultMaterial.albedo.rgba = glm::vec4(1.0f);
		// defaultMaterial.diffuse.rgba = glm::vec4(1.0f);
		// defaultMaterial.uuid = -1;
		////AssetsManager::mMaterials.emplace(-1, std::make_shared<Material>(defaultMaterial));
		// AssetsManager::AddMaterial(&defaultMaterial);
	}
	Mesh* DefaultModels::Cube() { return AssetsManager::GetMesh(cubeUuid); }
	Mesh* DefaultModels::Sphere() { return AssetsManager::GetMesh(sphereUuid); }
	Mesh* DefaultModels::Plane() { return AssetsManager::GetMesh(planeUuid); }
	Mesh* DefaultModels::Cylinder() { return AssetsManager::GetMesh(cylinderUuid); }
	Mesh* DefaultModels::Capsule() { return AssetsManager::GetMesh(capsuleUuid); }

	void DefaultModels::InitCube() {
		std::vector<unsigned int> indices = {
			0,	1,	2,	// Front face
			2,	1,	3,	// Front face
			4,	5,	6,	// Back face
			6,	5,	7,	// Back face
			8,	9,	10, // Top face
			10, 9,	11, // Top face
			12, 13, 14, // Bottom face
			14, 13, 15, // Bottom face
			16, 17, 18, // Left face
			18, 17, 19, // Left face
			20, 21, 22, // Right face
			22, 21, 23	// Right face
		};
		std::vector<glm::vec3> positions = {
			glm::vec3(-0.5f, -0.5f, 0.5f),	glm::vec3(0.5f, -0.5f, 0.5f),	glm::vec3(-0.5f, 0.5f, 0.5f),
			glm::vec3(0.5f, 0.5f, 0.5f),	glm::vec3(0.5f, -0.5f, -0.5f),	glm::vec3(-0.5f, -0.5f, -0.5f),
			glm::vec3(0.5f, 0.5f, -0.5f),	glm::vec3(-0.5f, 0.5f, -0.5f),	glm::vec3(-0.5f, 0.5f, 0.5f),
			glm::vec3(0.5f, 0.5f, 0.5f),	glm::vec3(-0.5f, 0.5f, -0.5f),	glm::vec3(0.5f, 0.5f, -0.5f),
			glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),	glm::vec3(-0.5f, -0.5f, 0.5f),
			glm::vec3(0.5f, -0.5f, 0.5f),	glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
			glm::vec3(-0.5f, 0.5f, -0.5f),	glm::vec3(-0.5f, 0.5f, 0.5f),	glm::vec3(0.5f, -0.5f, 0.5f),
			glm::vec3(0.5f, -0.5f, -0.5f),	glm::vec3(0.5f, 0.5f, 0.5f),	glm::vec3(0.5f, 0.5f, -0.5f)};

		std::vector<glm::vec3> normals = {
			glm::vec3(0, 0, 1),	 glm::vec3(0, 0, 1),  glm::vec3(0, 0, 1),  glm::vec3(0, 0, 1),	glm::vec3(0, 0, -1),
			glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0),	glm::vec3(0, 1, 0),
			glm::vec3(0, 1, 0),	 glm::vec3(0, 1, 0),  glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0),
			glm::vec3(0, -1, 0), glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0),
			glm::vec3(1, 0, 0),	 glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0)};

		std::vector<glm::vec2> texCoords = {
			glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(0, 0), glm::vec2(1, 0),
			glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(1, 1),
			glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(0, 0), glm::vec2(1, 0),
			glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(0, 1), glm::vec2(1, 1)};

		// new Mesh(positions, normals, texCoords, vector<glm::vec3>(), vector<glm::vec3>(), indices);

		std::vector<glm::vec3> tangents;
		std::vector<unsigned int> materials{0};
		Mesh* newMesh = Application::Get()->mRenderer->CreateNewMesh(positions, normals, texCoords, tangents, indices,
																	 materials, false);
		newMesh->uuid = cubeUuid;
		AssetsManager::AddMesh(newMesh);
	}

	void DefaultModels::InitSphere() {
		std::vector<unsigned int> indices;
		// std::vector<Vertex> vertices;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<glm::vec2> uvs;

		// Generate sphere vertices and indices
		const int stacks = 20;
		const int slices = 40;
		const float radius = 0.5f;
		float PI = 3.14159265359f;

		for (int i = 0; i <= stacks; ++i) {
			float stackAngle = static_cast<float>(i) * PI / stacks;
			float stackRatio = static_cast<float>(i) / stacks;
			float phi = stackAngle - PI / 2.0f;

			for (int j = 0; j <= slices; ++j) {
				float sliceAngle = static_cast<float>(j) * 2.0f * PI / slices;
				float sliceRatio = static_cast<float>(j) / slices;

				float x = radius * std::cos(phi) * std::cos(sliceAngle);
				float y = radius * std::sin(phi);
				float z = radius * std::cos(phi) * std::sin(sliceAngle);

				vertices.push_back(glm::vec3(x, y, z));
				normals.push_back(glm::normalize(glm::vec3(x, y, z)));
				uvs.push_back(glm::vec2(sliceRatio, stackRatio));
				// tangents.push_back(glm::vec3(0.0f));
				// bitangents.push_back(glm::vec3(0.0f));

				// vertices.push_back(Vertex(position, normal, texCoords, tangent, bitangent));
			}
		}

		// Generate sphere indices
		for (int i = 0; i < stacks; ++i) {
			int k1 = i * (slices + 1);
			int k2 = k1 + slices + 1;

			for (int j = 0; j < slices; ++j, ++k1, ++k2) {
				if (i != 0) {
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != stacks - 1) {
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}

		std::vector<unsigned int> materials{0};
		Mesh* newMesh =
			Application::Get()->mRenderer->CreateNewMesh(vertices, normals, uvs, tangents, indices, materials, false);
		newMesh->uuid = sphereUuid;
		AssetsManager::AddMesh(newMesh);
	}

	void DefaultModels::InitPlane() {
		std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};

		std::vector<glm::vec3> vertices = {glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(0.5f, 0.0f, 0.5f),
										   glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(-0.5f, 0.0f, -0.5f)};

		std::vector<glm::vec3> normals = {glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
										  glm::vec3(0, 1, 0)};

		std::vector<glm::vec2> texCoords = {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1)};

		std::vector<glm::vec3> tangents;
		std::vector<unsigned int> materials{0};
		Mesh* newMesh = Application::Get()->mRenderer->CreateNewMesh(vertices, normals, texCoords, tangents, indices,
																	 materials, false);
		newMesh->uuid = planeUuid;
		AssetsManager::AddMesh(newMesh);
	}

	void DefaultModels::InitCylinder() {
		float radius = 0.5f;
		float height = 1.0f;
		int slices = 20;
		int stacks = 20;
		vector<Vertex> vertices;
		vector<unsigned int> indices;

		float pi = 3.14159265359;
		// Vertex(glm::vec3(x, y, z), glm::vec3(0, 0, 1), glm::vec2(s, 1 - i / stacks), glm::vec3(0.0f),
		// glm::vec3(0.0f))
		for (unsigned int i = 1; i < slices; i++) {
			float x = glm::cos((slices / i)) * radius;
			float z = glm::sin((slices / i)) * radius;
			float y = 0;
			vertices.push_back(
				Vertex(glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f), glm::vec3(0.0f)));
			vertices.push_back(
				Vertex(glm::vec3(x, y + height, z), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f), glm::vec3(0.0f)));
		}

		for (unsigned int i = 0; i < slices; i += 2) {
			indices.push_back(i);
			indices.push_back(i + slices / 2);
			indices.push_back(i + 1);

			indices.push_back(i + 1);
			indices.push_back(i + slices / 2);
			indices.push_back(i + slices / 2 + 1);

			// indices.push_back(i);
			// indices.push_back(i + slices + 1);
			// indices.push_back(i + slices + 2);
		}

		// Create the cylinder mesh and add it to the scene
		// Mesh* newMesh = new Mesh(vertices, indices);
		// newMesh->usingNormal = false; // You probably want to set this to true for the cylinder
		// newMesh->meshId = cylinderUuid;
		// Scene::GetEditorScene()->meshes.emplace(newMesh->meshId, std::make_shared<Mesh>(*newMesh));
	}

	void DefaultModels::InitCapsule(float radius, float height, int stacks, int slices) {
		std::vector<unsigned int> indices;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;

		float halfHeight = height / 2.0f;
		float PI = 3.14159265359f;

		// 1. Generate the top hemisphere (like half of a sphere)
		for (int i = 0; i <= stacks; ++i) {
			float stackAngle = static_cast<float>(i) * (PI / 2.0f) / stacks; // Half-sphere angle
			float stackRatio = static_cast<float>(i) / stacks;
			float phi = stackAngle;

			for (int j = 0; j <= slices; ++j) {
				float sliceAngle = static_cast<float>(j) * 2.0f * PI / slices;
				float sliceRatio = static_cast<float>(j) / slices;

				float x = radius * std::cos(phi) * std::cos(sliceAngle);
				float y = radius * std::sin(phi) + halfHeight; // Shift by half the height
				float z = radius * std::cos(phi) * std::sin(sliceAngle);

				vertices.push_back(glm::vec3(x, y, z));
				normals.push_back(glm::normalize(glm::vec3(x, y - halfHeight, z))); // Corrected normal pointing outward
				uvs.push_back(glm::vec2(sliceRatio, stackRatio));
			}
		}

		// 2. Generate the cylindrical body (between the hemispheres)
		int baseCylinderStart = (stacks + 1) * (slices + 1); // Offset for cylindrical body start
		for (int i = 0; i <= stacks; ++i) {
			float stackRatio = static_cast<float>(i) / stacks;
			float y = halfHeight - stackRatio * height; // Move down along the Y-axis

			for (int j = 0; j <= slices; ++j) {
				float sliceAngle = static_cast<float>(j) * 2.0f * PI / slices;
				float sliceRatio = static_cast<float>(j) / slices;

				float x = radius * std::cos(sliceAngle);
				float z = radius * std::sin(sliceAngle);

				vertices.push_back(glm::vec3(x, y, z));
				normals.push_back(glm::normalize(glm::vec3(x, 0.0f, z))); // Normal is horizontal
				uvs.push_back(glm::vec2(sliceRatio, stackRatio));
			}
		}

		// 3. Generate the bottom hemisphere (flipped version of the top hemisphere)
		int baseBottomHemisphereStart =
			baseCylinderStart + (stacks + 1) * (slices + 1); // Offset for bottom hemisphere start
		for (int i = 0; i <= stacks; ++i) {
			float stackAngle = static_cast<float>(i) * (PI / 2.0f) / stacks;
			float stackRatio = static_cast<float>(i) / stacks;
			float phi = stackAngle;

			for (int j = 0; j <= slices; ++j) {
				float sliceAngle = static_cast<float>(j) * 2.0f * PI / slices;
				float sliceRatio = static_cast<float>(j) / slices;

				float x = radius * std::cos(phi) * std::cos(sliceAngle);
				float y = -radius * std::sin(phi) - halfHeight; // Shift down
				float z = radius * std::cos(phi) * std::sin(sliceAngle);

				vertices.push_back(glm::vec3(x, y, z));
				normals.push_back(glm::normalize(glm::vec3(x, y + halfHeight, z))); // Corrected normal pointing outward
				uvs.push_back(glm::vec2(sliceRatio, stackRatio));
			}
		}

		// 4. Generate indices for the capsule

		// Top hemisphere
		for (int i = 0; i < stacks; ++i) {
			int k1 = i * (slices + 1);
			int k2 = k1 + slices + 1;

			for (int j = 0; j < slices; ++j, ++k1, ++k2) {
				if (i != 0) {
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}
				if (i != stacks - 1) {
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}

		// Cylindrical body
		for (int i = 0; i < stacks; ++i) {
			int k1 = baseCylinderStart + i * (slices + 1);
			int k2 = k1 + slices + 1;

			for (int j = 0; j < slices; ++j, ++k1, ++k2) {
				indices.push_back(k1);
				indices.push_back(k1 + 1);
				indices.push_back(k2);

				indices.push_back(k2);
				indices.push_back(k1 + 1);
				indices.push_back(k2 + 1);
			}
		}

		// Bottom hemisphere (reversed winding order)
		for (int i = 0; i < stacks; ++i) {
			int k1 = baseBottomHemisphereStart + i * (slices + 1);
			int k2 = k1 + slices + 1;

			for (int j = 0; j < slices; ++j, ++k1, ++k2) {
				indices.push_back(k2);	   // Reverse order
				indices.push_back(k2 + 1); // Reverse order
				indices.push_back(k1);	   // Reverse order

				if (i != stacks - 1) {
					indices.push_back(k1);	   // Reverse order
					indices.push_back(k2 + 1); // Reverse order
					indices.push_back(k1 + 1); // Reverse order
				}
			}
		}

		// 5. Create mesh and add to AssetsManager
		std::vector<glm::vec3> tangents;
		std::vector<unsigned int> materials{0};
		Mesh* newMesh =
			Application::Get()->mRenderer->CreateNewMesh(vertices, normals, uvs, tangents, indices, materials, false);
		newMesh->uuid = capsuleUuid;
		AssetsManager::AddMesh(newMesh);
	}
} // namespace Plaza::Editor

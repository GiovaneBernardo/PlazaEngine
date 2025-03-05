#pragma once
namespace Plaza::Editor {
	class DefaultModels {
	  public:
		static vector<Mesh*> meshes;

		static void Init();

		static Mesh* Cube();
		static Mesh* Sphere();
		static Mesh* Plane();
		static Mesh* Cylinder();
		static Mesh* Capsule();

	  private:
		static uint64_t cubeUuid;
		static uint64_t sphereUuid;
		static uint64_t planeUuid;
		static uint64_t cylinderUuid;
		static uint64_t capsuleUuid;

		static void InitCube();
		static void InitSphere();
		static void InitPlane();
		static void InitCylinder();
		static void InitCapsule(float radius, float height, int stacks, int slices);
	};
} // namespace Plaza::Editor

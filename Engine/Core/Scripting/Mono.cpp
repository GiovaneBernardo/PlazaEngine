#include "Engine/Core/PreCompiledHeaders.h"
#include "Mono.h"
#include <functional>

#define PL_ADD_INTERNAL_CALL(name) mono_add_internal_call("Plaza.InternalCalls::" #name, (void*)InternalCalls::name)
namespace Plaza {

	MonoDomain* Mono::mAppDomain = nullptr;
	MonoAssembly* Mono::mCoreAssembly = nullptr;
	MonoDomain* Mono::mMonoRootDomain = nullptr;
	char* ReadBytes(const std::string& filepath, uint32_t* outSize)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return nullptr;
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = end - stream.tellg();

		if (size == 0)
		{
			// File is empty
			return nullptr;
		}

		char* buffer = new char[size];
		stream.read((char*)buffer, size);
		stream.close();

		*outSize = size;
		return buffer;
	}

	MonoAssembly* Mono::LoadCSharpAssembly(const std::string& assemblyPath)
	{
		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);

		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			printf(errorMessage);
			// Log some error message using the errorMessage data
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		// Don't forget to free the file data
		delete[] fileData;

		return assembly;
	}

	void PrintAssemblyTypes(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			printf("%s.%s\n", nameSpace, name);
		}
	}

	MonoClass* Mono::GetClassInAssembly(MonoAssembly* assembly, const char* namespaceName, const char* className)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		MonoClass* klass = mono_class_from_name(image, namespaceName, className);

		if (klass == nullptr)
		{
			// Log error here
			return nullptr;
		}

		return klass;
	}

	MonoObject* Mono::InstantiateClass(const char* namespaceName, const char* className, MonoAssembly* assembly, MonoDomain* appDomain)
	{
		// Get a reference to the class we want to instantiate
		MonoClass* testingClass = GetClassInAssembly(assembly, namespaceName, className);

		// Allocate an instance of our class
		MonoObject* classInstance = mono_object_new(appDomain, testingClass);

		if (classInstance == nullptr)
		{
			// Log error here and abort
		}

		// Call the parameterless (default) constructor
		mono_runtime_object_init(classInstance);
		return classInstance;
	}

	void Mono::CallMethod(MonoObject* objectInstance, std::string methodName)
	{
		// Get the MonoClass pointer from the instance
		MonoClass* instanceClass = mono_object_get_class(objectInstance);

		// Get a reference to the method in the class
		MonoMethod* method = mono_class_get_method_from_name(instanceClass, methodName.c_str(), 0);

		if (method == nullptr)
		{
			// No method called "PrintFloatVar" with 0 parameters in the class, log error or something
			return;
		}

		// Call the C# method on the objectInstance instance, and get any potential exceptions
		MonoObject* exception = nullptr;
		mono_runtime_invoke(method, objectInstance, nullptr, &exception);

		// Check if an exception occurred
		if (exception != nullptr) {
			// Get the MonoException type
			MonoClass* exceptionClass = mono_get_exception_class();
			MonoString* messageString = mono_object_to_string((MonoObject*)exception, nullptr);
			const char* message = mono_string_to_utf8(messageString);
			printf("Exception occurred: %s\n", message);

		}
	}

	static void CppFunction() {
		std::cout << "Writen in C++" << std::endl;
	}

	static void Vector3Log(glm::vec3* vec3) {
		std::cout << "X: " << vec3->x << std::endl;
		std::cout << "Y: " << vec3->y << std::endl;
		std::cout << "Z: " << vec3->z << std::endl;
	}

	void Mono::Init() {
		mono_set_assemblies_path("lib/mono");
		//mono_set_assemblies_path((Application->editorPath + "/lib/mono").c_str());
		if(Mono::mMonoRootDomain == nullptr)
		Mono::mMonoRootDomain = mono_jit_init("MyScriptRuntime");
		if (Mono::mMonoRootDomain == nullptr)
		{
			// Maybe log some error here
			return;
		}
		// Create an App Domain
		char appDomainName[] = "PlazaAppDomain";
		Mono::mAppDomain = mono_domain_create_appdomain(appDomainName, nullptr);
		mono_domain_set(mAppDomain, true);

		// Add all the internal calls
		//mono_add_internal_call("Plaza.InternalCalls::CppFunction", CppFunction);
		//mono_add_internal_call("Plaza.InternalCalls::Vector3Log", Vector3Log);

		InternalCalls::Init();

		// Load the PlazaScriptCore.dll assembly
		mCoreAssembly = mono_domain_assembly_open(mAppDomain, (Application->dllPath + "\\PlazaScriptCore.dll").c_str());
		if (!mCoreAssembly) {
			// Handle the error (assembly not found or failed to load)
			std::cout << "Didnt loaded assembly" << std::endl;
		}

		// Load all scripts
		for (auto& [key, value] : Application->activeProject->scripts) {
			std::string dllPath = filesystem::path{ key }.replace_extension(".dll").string();
			// Get a reference to the class we want to instantiate

			MonoClass* testingClass = GetClassInAssembly(LoadCSharpAssembly(dllPath), "", "Unnamed");

			// Allocate an instance of our class
			MonoObject* classInstance = mono_object_new(mAppDomain, testingClass);

			if (classInstance == nullptr)
			{
				// Log error here and abort
			}

			MonoObject* monoObject = InstantiateClass("", "Unnamed", LoadCSharpAssembly(dllPath), mAppDomain);
			//CallOnStart(monoObject);

			// Call the parameterless (default) constructor
			//mono_runtime_object_init(classInstance);
			Application->activeProject->monoObjects.emplace(dllPath, monoObject);
		}
	}

	// Execute OnStart on all scripts
	void Mono::OnStart() {
		for (auto [key, value] : Application->activeScene->cppScriptComponents) {
			CallMethod(value.monoObject, "OnStart");
		}
	}

	// Execute OnUpdate on all scripts
	void Mono::Update() {
		for (auto [key, value] : Application->activeScene->cppScriptComponents) {
			CallMethod(value.monoObject, "OnUpdate");
		}
	}
}
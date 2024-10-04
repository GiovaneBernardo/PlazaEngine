#pragma once
namespace Plaza {
	class Mono {
	public:
		static MonoDomain* mAppDomain;
		static MonoAssembly* mCoreAssembly;
		static MonoDomain* mMonoRootDomain;
		static MonoImage* mCoreImage;
		static MonoObject* mEntityObject;
		static MonoClass* mEntityClass;

		static MonoAssembly* mScriptAssembly;
		static MonoImage* mScriptImage;

		static std::unordered_map<MonoType*, std::function<bool(Entity)>> mEntityHasComponentFunctions;
		static std::unordered_map<MonoType*, std::function<Component* (Entity)>> mEntityAddComponentFunctions;
		static std::unordered_map<MonoType*, std::function<Component* (Entity)>> mEntityGetComponentFunctions;
		static void Init();
		static void OnStartAll(bool callOnStart = true);
		static void OnStart(MonoObject* monoObject);
		static void Update();
		static void RegisterComponents();
		static MonoObject* InstantiateClass(const char* namespaceName, const char* className, MonoAssembly* assembly, MonoDomain* appDomain, uint64_t uuid = 0);

		static void ReloadAppDomain();
		static MonoMethod* GetMethod(MonoObject* objectInstance, const std::string& methodName, int parameterCount = 0);
		static void CallMethod(MonoObject* objectInstance, std::string methodName);
		static void CallMethod(MonoObject* objectInstance, MonoMethod* method);
		static void CallMethod(MonoObject* objectInstance, MonoMethod* method, void* param);
		static void CallMethod(MonoObject* objectInstance, MonoMethod* method, void* params[]);
		static MonoClass* GetClassInAssembly(MonoAssembly* assembly, const char* namespaceName, const char* className, bool isCore = false);
		static MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath);
	};

	class InternalCalls {
	public:
		static void Init();
	};
}
#pragma once
#include "Script.h"

namespace Plaza {
	class PLAZA_API CppScript : public Script {
	  public:
		CppScript() { this->lastModifiedDate = std::chrono::system_clock::now(); }
		virtual ~CppScript(){};
		virtual void OnStart(Scene* scene) {};
		virtual void OnUpdate(Scene* scene) {};
		virtual void OnTerminate(Scene* scene) {};
		virtual void OnUpdateEditorGUI(Scene* scene) {};

		virtual void OnCollide(Scene* scene, const RaycastHit hit) {};
	};

	class PLAZA_API CppEditorScript : public Script {
	  public:
		CppEditorScript() { this->lastModifiedDate = std::chrono::system_clock::now(); }

		virtual void OnStart(Scene* scene) {};
		virtual void OnUpdate(Scene* scene) {};
		virtual void OnTerminate(Scene* scene) {};
	};
} // namespace Plaza

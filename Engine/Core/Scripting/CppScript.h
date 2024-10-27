#pragma once
#include "Script.h"

namespace Plaza {
	class PLAZA_API CppScript : public Script {
	public:
		CppScript() {
			this->lastModifiedDate = std::chrono::system_clock::now();
		}
		
		virtual void OnStart() {};
		virtual void OnUpdate() {};
		virtual void OnTerminate() {};
	};

	class PLAZA_API CppEditorScript : public Script {
	public:
		CppEditorScript() {
			this->lastModifiedDate = std::chrono::system_clock::now();
		}

		virtual void OnStart() {};
		virtual void OnUpdate() {};
		virtual void OnTerminate() {};
	};
}
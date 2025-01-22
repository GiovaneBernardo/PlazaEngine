#pragma once
#include "spdlog/spdlog.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API Log {
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger();
		static std::shared_ptr<spdlog::logger>& GetClientLogger();

	private:
		static std::shared_ptr<spdlog::logger> sCoreLogger;
		static std::shared_ptr<spdlog::logger> sClientLogger;
	};
}

#define PL_CORE_TRACE(...) ::Plaza::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define PL_CORE_INFO(...) ::Plaza::Log::GetCoreLogger()->info(__VA_ARGS__);
#define PL_CORE_WARN(...) ::Plaza::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define PL_CORE_ERROR(...) ::Plaza::Log::GetCoreLogger()->error(__VA_ARGS__);
#define PL_CORE_CRITICAL(...) ::Plaza::Log::GetCoreLogger()->critical(__VA_ARGS__);

#define PL_TRACE(...) ::Plaza::Log::GetClientLogger()->trace(__VA_ARGS__);
#define PL_INFO(...) ::Plaza::Log::GetClientLogger()->info(__VA_ARGS__);
#define PL_WARN(...) ::Plaza::Log::GetClientLogger()->warn(__VA_ARGS__);
#define PL_ERROR(...) ::Plaza::Log::GetClientLogger()->error(__VA_ARGS__);
#define PL_CRITICAL(...) ::Plaza::Log::GetClientLogger()->critical(__VA_ARGS__);

#ifdef IGNORE_LOG
#define PL_CORE_TRACE
#define PL_CORE_INFO
#define PL_CORE_WARN
#define PL_CORE_ERROR
#define PL_CORE_CRITICAL

#define PL_TRACE
#define PL_INFO
#define PL_WARN
#define PL_ERROR
#define PL_CRITICAL
#endif // IGNORE_LOG

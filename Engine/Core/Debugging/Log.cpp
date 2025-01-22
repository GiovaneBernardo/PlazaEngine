#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Plaza {
	std::shared_ptr<spdlog::logger> Log::sCoreLogger = nullptr;
	std::shared_ptr<spdlog::logger> Log::sClientLogger = nullptr;

	std::shared_ptr<spdlog::logger>& Log::GetCoreLogger() { return sCoreLogger; }
	std::shared_ptr<spdlog::logger>& Log::GetClientLogger() { return sClientLogger; }

	void Log::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");
		sCoreLogger = spdlog::stdout_color_mt("PLAZA");
		sCoreLogger->set_level(spdlog::level::trace);

		sClientLogger = spdlog::stdout_color_mt("APP");
		sClientLogger->set_level(spdlog::level::trace);
	}
}
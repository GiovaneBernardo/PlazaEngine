#include "Log.h"
#include "Engine/Core/Scripting/Script.h"
#include <memory>

namespace Plaza {
	std::shared_ptr<spdlog::logger> Log::sCoreLogger = nullptr;
	std::shared_ptr<spdlog::logger> Log::sClientLogger = nullptr;
	std::shared_ptr<PlazaVectorSink> Log::sVectorSink = nullptr;
	std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> Log::sConsoleSink = nullptr;
	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> Log::sFileSink = nullptr;

	std::shared_ptr<spdlog::logger>& Log::GetCoreLogger() { return sCoreLogger; }
	std::shared_ptr<spdlog::logger>& Log::GetClientLogger() { return sClientLogger; }
	std::shared_ptr<PlazaVectorSink>& Log::GetVectorSink() { return sVectorSink; };

	void Log::Init() {
		spdlog::flush_every(std::chrono::seconds(1));
		spdlog::set_pattern("%^[%T] %n: %v%$");

		Log::sVectorSink = std::make_shared<PlazaVectorSink>();
		Log::sConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		Log::sFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/plazaEngine.log", true);
		auto sinks = spdlog::sinks_init_list{sConsoleSink, sFileSink, sVectorSink};

		sCoreLogger = std::make_shared<spdlog::logger>("PLAZA", sinks.begin(), sinks.end());
		sCoreLogger->set_level(spdlog::level::trace);

		sClientLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
		sClientLogger->set_level(spdlog::level::trace);
	}

	void Log::Terminate() {
		sFileSink->flush();
		sVectorSink->flush();
		sConsoleSink->flush();
	}
} // namespace Plaza

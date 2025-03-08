#pragma once
#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/sinks/sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#include "Engine/Core/Engine.h"
#include <memory>
#include <mutex>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace spdlog::sinks {}

namespace Plaza {
	struct LogMessage {
		spdlog::level::level_enum mLevel;
		std::string mMessage;
		std::time_t mTime;
	};

	class PlazaVectorSink;
	class PLAZA_API Log {
	  public:
		static void Init();
		static void Terminate();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger();
		static std::shared_ptr<spdlog::logger>& GetClientLogger();

		static std::shared_ptr<PlazaVectorSink>& GetVectorSink();
	  private:
		static std::shared_ptr<spdlog::logger> sCoreLogger;
		static std::shared_ptr<spdlog::logger> sClientLogger;
		static std::shared_ptr<PlazaVectorSink> sVectorSink;
		static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> sConsoleSink;
		static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> sFileSink;
	};

	class PlazaVectorSink : public spdlog::sinks::base_sink<std::mutex> {
	  public:
		const std::vector<LogMessage>& GetLogs() { return mLogs; }

	  protected:
		void sink_it_(const spdlog::details::log_msg& msg) override {
			spdlog::memory_buf_t formatted;
			formatter_->format(msg, formatted);
			mLogs.push_back({msg.level, fmt::to_string(formatted), std::chrono::system_clock::to_time_t(msg.time)});
		}

		void flush_() override {}

	  private:
		std::vector<LogMessage> mLogs;
	};
} // namespace Plaza

#define PL_CORE_COMMON_MACRO()

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

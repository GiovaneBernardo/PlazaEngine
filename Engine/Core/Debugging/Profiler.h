#pragma once
#ifdef EDITOR_MODE
#include <ThirdParty/Tracy/public/tracy/Tracy.hpp>
#include <chrono>
// #include "ThirdParty/Tracy/tracy/Tracy.hpp"
#define PLAZA_PROFILE ZoneScoped
#define PLAZA_PROFILE_FRAME(x) FrameMark
#define PLAZA_PROFILE_SECTION(x)                                                                                       \
	static constexpr tracy::SourceLocationData __tracy_source_location385{x, __FUNCTION__, "", (uint32_t)385, 0};      \
	tracy::ScopedZone ___tracy_scoped_zone(&__tracy_source_location385, true)
#else
#define PLAZA_PROFILE
#define PLAZA_PROFILE_FRAME(x)
#define PLAZA_PROFILE_SECTION(x)
#endif

#define PLAZA_ASSERT assert

namespace Plaza {
	class SectionProfiler {
	  public:
		SectionProfiler(const std::string& name = "") {
			mStart = std::chrono::high_resolution_clock::now();
			mName = name;
		}

		std::chrono::microseconds Stop() {
			mEnd = std::chrono::high_resolution_clock::now();
			mDurationNanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(mEnd - mStart).count();
			return std::chrono::microseconds(mDurationNanoSeconds / 1000);
		}

		std::chrono::microseconds GetDuration() { return std::chrono::microseconds(mDurationNanoSeconds / 1000); }

		std::chrono::high_resolution_clock::time_point mStart;
		std::chrono::high_resolution_clock::time_point mEnd;
		std::int64_t mDurationNanoSeconds{0};
		std::string mName;
	};
	class Profiler {
	  public:
		void AddDuration(std::chrono::nanoseconds duration) {
			mTotalNanoSeconds.fetch_add(duration.count(), std::memory_order_relaxed);
		}

		std::chrono::nanoseconds GetNanoSeconds() const {
			return std::chrono::nanoseconds(mTotalNanoSeconds.load(std::memory_order_relaxed));
		}

		void Reset() { mTotalNanoSeconds = 0; }

		void AddSection(SectionProfiler section) {
			mSections.push_back(section);
			mDirtySectionsOrder = true;
		}

		void OrderSections() {
			if (!mDirtySectionsOrder)
				return;
			std::sort(mSections.begin(), mSections.end(), ([](SectionProfiler& a, SectionProfiler& b) {
						  return a.mDurationNanoSeconds > b.mDurationNanoSeconds;
					  }));
			mDirtySectionsOrder = false;
		}

		std::vector<SectionProfiler> mSections = std::vector<SectionProfiler>();
		bool mDirtySectionsOrder = false;

	  private:
		std::atomic<std::int64_t> mTotalNanoSeconds{0};

	  public:
		static std::chrono::high_resolution_clock::time_point StartSectionProfiling() {
			return std::chrono::high_resolution_clock::now();
		}
		static std::chrono::high_resolution_clock::time_point StopSectionProfiling() {
			return std::chrono::high_resolution_clock::now();
		}

		static std::shared_ptr<Profiler> NewProfiler(const std::string& profilerName) {
			if (sProfilers.find(profilerName) == sProfilers.end())
				sProfilers.emplace(profilerName, std::make_shared<Profiler>());
			return sProfilers.at(profilerName);
		}
		static std::shared_ptr<Profiler> GetProfiler(const std::string& profilerName) {
			PLAZA_ASSERT(sProfilers.find(profilerName) != sProfilers.end());
			return sProfilers.at(profilerName);
		}

	  public:
		static inline std::unordered_map<std::string, std::shared_ptr<Profiler>> sProfilers =
			std::unordered_map<std::string, std::shared_ptr<Profiler>>();
	};
} // namespace Plaza

/*

	static constexpr tracy::SourceLocationData __tracy_source_location385{ "a", __FUNCTION__,
   "C:/Users/Giovane/Desktop/Workspace/Plaza/Engine/Application/Application.cpp", (uint32_t)385, 0 }; tracy::ScopedZone
   ___tracy_scoped_zone(&__tracy_source_location385, true)

*/

#include "Engine/Core/PreCompiledHeaders.h"
#include "UUID.h"
#include <random>
#include <cstdint>
namespace Plaza {
	static std::random_device randomDevice;
	static std::mt19937_64 randomEngine(randomDevice());
	static std::uniform_int_distribution<uint64_t> uniformDistribution(0, std::numeric_limits<uint64_t>::max());
	uint64_t UUID::NewUUID() { return uniformDistribution(randomEngine); }
} // namespace Plaza

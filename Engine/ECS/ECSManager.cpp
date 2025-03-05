#include "Engine/Core/PreCompiledHeaders.h"
#include "ECSManager.h"
#include "Engine/Core/Engine.h"
#include <ThirdParty/cereal/cereal/archives/json.hpp>
#include <ThirdParty/cereal/cereal/types/string.hpp>
#include <ThirdParty/cereal/cereal/types/vector.hpp>
#include <ThirdParty/cereal/cereal/cereal.hpp>
#include <ThirdParty/cereal/cereal/archives/binary.hpp>
#include <ThirdParty/cereal/cereal/types/map.hpp>
#include <ThirdParty/cereal/cereal/types/polymorphic.hpp>
#include <ThirdParty/cereal/cereal/types/utility.hpp>
#include "Engine/Components/Core/Transform.h"
#include "Engine/Core/Scene.h"

#include "Engine/Components/Core/Camera.h"
#include "Engine/Components/Rendering/MeshRenderer.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Components/Rendering/Light.h"
#include "Engine/Components/Physics/RigidBody.h"
#include "Engine/Components/Physics/CharacterController.h"
#include "Engine/Components/Physics/Collider.h"
#include "Engine/Components/Scripting/CppScriptComponent.h"
#include "Engine/Components/Drawing/UI/TextRenderer.h"
#include "Engine/Components/Drawing/UI/Gui.h"
#include "Engine/Components/Audio/AudioSource.h"
#include "Engine/Components/Audio/AudioListener.h"
#include "Engine/Components/Rendering/AnimationComponent.h"

#define PL_REGISTER_COMPONENT(T)                                                                                       \
	CEREAL_REGISTER_TYPE(T);                                                                                           \
	CEREAL_REGISTER_POLYMORPHIC_RELATION(Plaza::Component, T);
namespace Plaza {

	void ECS::InstantiateComponent(ComponentPool* srcPool, ComponentPool* dstPool, uint64_t srcUuid, uint64_t dstUuid) {
		Component* component = static_cast<Component*>(
			sInstantiateComponentFactory[srcPool->mComponentMask](srcPool, dstPool, srcUuid, dstUuid));
		component->mUuid = dstUuid;
	}
} // namespace Plaza
PL_REGISTER_COMPONENT(Plaza::TransformComponent);
PL_REGISTER_COMPONENT(Plaza::MeshRenderer);
PL_REGISTER_COMPONENT(Plaza::Collider);
PL_REGISTER_COMPONENT(Plaza::RigidBody);
PL_REGISTER_COMPONENT(Plaza::Camera);
PL_REGISTER_COMPONENT(Plaza::Light);
PL_REGISTER_COMPONENT(Plaza::AudioSource);
PL_REGISTER_COMPONENT(Plaza::CppScriptComponent);
PL_REGISTER_COMPONENT(Plaza::AnimationComponent);

namespace cereal {
	namespace detail {
		template <> struct binding_name<Plaza::AudioListener> {
			static constexpr char const* name() { return "AudioListener"; }
		};
	} // namespace detail
} // namespace cereal
namespace cereal {
	namespace detail {
		template <> struct init_binding<Plaza::AudioListener> {
			static inline bind_to_archives<Plaza::AudioListener> const& b =
				::cereal::detail::StaticObject<bind_to_archives<Plaza::AudioListener>>::getInstance().bind();
			static void unused() { (void)b; }
		};
	} // namespace detail
}; // namespace cereal
namespace cereal {
	namespace detail {
		template <> struct PolymorphicRelation<Plaza::Component, Plaza::AudioListener> {
			static void bind() { RegisterPolymorphicCaster<Plaza::Component, Plaza::AudioListener>::bind(); }
		};
	} // namespace detail
}; // namespace cereal
;

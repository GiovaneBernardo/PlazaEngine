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
#include "Engine/Components/Scripting/CsScriptComponent.h"
#include "Engine/Components/Drawing/UI/TextRenderer.h"
#include "Engine/Components/Drawing/UI/Gui.h"
#include "Engine/Components/Audio/AudioSource.h"
#include "Engine/Components/Audio/AudioListener.h"
#include "Engine/Components/Rendering/AnimationComponent.h"

#define PL_REGISTER_COMPONENT(T) \
    CEREAL_REGISTER_TYPE(T) ;\
	CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, T) ;
namespace Plaza {

	void ECS::InstantiateComponent(ComponentPool* srcPool, ComponentPool* dstPool, uint64_t srcUuid, uint64_t dstUuid) {
		Component* component = static_cast<Component*>(sInstantiateComponentFactory[srcPool->mComponentMask](srcPool, dstPool, srcUuid, dstUuid));
		component->mUuid = dstUuid;
	}
}
PL_REGISTER_COMPONENT(TransformComponent);
PL_REGISTER_COMPONENT(MeshRenderer);
PL_REGISTER_COMPONENT(Collider);
PL_REGISTER_COMPONENT(RigidBody);
PL_REGISTER_COMPONENT(Camera);
PL_REGISTER_COMPONENT(Light);
PL_REGISTER_COMPONENT(AudioSource);
namespace cereal {
    namespace detail {
        template <> struct binding_name<AudioListener> {
            static constexpr char const* name() {
                return "AudioListener";
            }
        };
    }
} namespace cereal {
    namespace detail {
        template<> struct init_binding<AudioListener> {
            static inline bind_to_archives<AudioListener> const& b = ::cereal::detail::StaticObject< bind_to_archives<AudioListener> >::getInstance().bind(); static void unused() {
                (void)b;
            }
        };
    }
}; namespace cereal {
    namespace detail {
        template <> struct PolymorphicRelation<Component, AudioListener> {
            static void bind() {
                RegisterPolymorphicCaster<Component, AudioListener>::bind();
            }
        };
    }
};;
PL_REGISTER_COMPONENT(CppScriptComponent);
PL_REGISTER_COMPONENT(AnimationComponent);
PL_REGISTER_COMPONENT(CsScriptComponent);
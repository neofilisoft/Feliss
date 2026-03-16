#include "ecs/ComponentFactory.h"

#include "ecs/Component.h"
#include "ecs/World.h"

namespace Feliss::ComponentFactory {

namespace {

const std::vector<std::string_view> kAddableComponents = {
    "MeshRendererComponent",
    "SpriteComponent",
    "CameraComponent",
    "LightComponent",
    "RigidBodyComponent",
    "BoxColliderComponent",
    "SphereColliderComponent",
    "AudioSourceComponent",
    "AudioListenerComponent",
    "AnimatorComponent",
    "ParticleSystemComponent",
    "ScriptComponent",
};

} // namespace

ComponentBase* addComponentByTypeName(World& world, EntityID id, std::string_view typeName) {
    if (typeName == "MeshRendererComponent")   return &world.addComponent<MeshRendererComponent>(id);
    if (typeName == "SpriteComponent")         return &world.addComponent<SpriteComponent>(id);
    if (typeName == "CameraComponent")         return &world.addComponent<CameraComponent>(id);
    if (typeName == "LightComponent")          return &world.addComponent<LightComponent>(id);
    if (typeName == "RigidBodyComponent")      return &world.addComponent<RigidBodyComponent>(id);
    if (typeName == "BoxColliderComponent")    return &world.addComponent<BoxColliderComponent>(id);
    if (typeName == "SphereColliderComponent") return &world.addComponent<SphereColliderComponent>(id);
    if (typeName == "AudioSourceComponent")    return &world.addComponent<AudioSourceComponent>(id);
    if (typeName == "AudioListenerComponent")  return &world.addComponent<AudioListenerComponent>(id);
    if (typeName == "AnimatorComponent")       return &world.addComponent<AnimatorComponent>(id);
    if (typeName == "ParticleSystemComponent") return &world.addComponent<ParticleSystemComponent>(id);
    if (typeName == "ScriptComponent")         return &world.addComponent<ScriptComponent>(id);
    return nullptr;
}

ComponentBase* getComponentByTypeName(World& world, EntityID id, std::string_view typeName) {
    if (typeName == "TagComponent")            return world.getComponent<TagComponent>(id);
    if (typeName == "TransformComponent")      return world.getComponent<TransformComponent>(id);
    if (typeName == "MeshRendererComponent")   return world.getComponent<MeshRendererComponent>(id);
    if (typeName == "SpriteComponent")         return world.getComponent<SpriteComponent>(id);
    if (typeName == "CameraComponent")         return world.getComponent<CameraComponent>(id);
    if (typeName == "LightComponent")          return world.getComponent<LightComponent>(id);
    if (typeName == "RigidBodyComponent")      return world.getComponent<RigidBodyComponent>(id);
    if (typeName == "BoxColliderComponent")    return world.getComponent<BoxColliderComponent>(id);
    if (typeName == "SphereColliderComponent") return world.getComponent<SphereColliderComponent>(id);
    if (typeName == "AudioSourceComponent")    return world.getComponent<AudioSourceComponent>(id);
    if (typeName == "AudioListenerComponent")  return world.getComponent<AudioListenerComponent>(id);
    if (typeName == "AnimatorComponent")       return world.getComponent<AnimatorComponent>(id);
    if (typeName == "ParticleSystemComponent") return world.getComponent<ParticleSystemComponent>(id);
    if (typeName == "ScriptComponent")         return world.getComponent<ScriptComponent>(id);
    return nullptr;
}

bool hasComponentByTypeName(const World& world, EntityID id, std::string_view typeName) {
    if (typeName == "TagComponent")            return world.hasComponent<TagComponent>(id);
    if (typeName == "TransformComponent")      return world.hasComponent<TransformComponent>(id);
    if (typeName == "MeshRendererComponent")   return world.hasComponent<MeshRendererComponent>(id);
    if (typeName == "SpriteComponent")         return world.hasComponent<SpriteComponent>(id);
    if (typeName == "CameraComponent")         return world.hasComponent<CameraComponent>(id);
    if (typeName == "LightComponent")          return world.hasComponent<LightComponent>(id);
    if (typeName == "RigidBodyComponent")      return world.hasComponent<RigidBodyComponent>(id);
    if (typeName == "BoxColliderComponent")    return world.hasComponent<BoxColliderComponent>(id);
    if (typeName == "SphereColliderComponent") return world.hasComponent<SphereColliderComponent>(id);
    if (typeName == "AudioSourceComponent")    return world.hasComponent<AudioSourceComponent>(id);
    if (typeName == "AudioListenerComponent")  return world.hasComponent<AudioListenerComponent>(id);
    if (typeName == "AnimatorComponent")       return world.hasComponent<AnimatorComponent>(id);
    if (typeName == "ParticleSystemComponent") return world.hasComponent<ParticleSystemComponent>(id);
    if (typeName == "ScriptComponent")         return world.hasComponent<ScriptComponent>(id);
    return false;
}

bool removeComponentByTypeName(World& world, EntityID id, std::string_view typeName) {
    if (typeName == "MeshRendererComponent")   { world.removeComponent<MeshRendererComponent>(id); return true; }
    if (typeName == "SpriteComponent")         { world.removeComponent<SpriteComponent>(id); return true; }
    if (typeName == "CameraComponent")         { world.removeComponent<CameraComponent>(id); return true; }
    if (typeName == "LightComponent")          { world.removeComponent<LightComponent>(id); return true; }
    if (typeName == "RigidBodyComponent")      { world.removeComponent<RigidBodyComponent>(id); return true; }
    if (typeName == "BoxColliderComponent")    { world.removeComponent<BoxColliderComponent>(id); return true; }
    if (typeName == "SphereColliderComponent") { world.removeComponent<SphereColliderComponent>(id); return true; }
    if (typeName == "AudioSourceComponent")    { world.removeComponent<AudioSourceComponent>(id); return true; }
    if (typeName == "AudioListenerComponent")  { world.removeComponent<AudioListenerComponent>(id); return true; }
    if (typeName == "AnimatorComponent")       { world.removeComponent<AnimatorComponent>(id); return true; }
    if (typeName == "ParticleSystemComponent") { world.removeComponent<ParticleSystemComponent>(id); return true; }
    if (typeName == "ScriptComponent")         { world.removeComponent<ScriptComponent>(id); return true; }
    return false;
}

const std::vector<std::string_view>& addableComponentTypes() {
    return kAddableComponents;
}

} // namespace Feliss::ComponentFactory

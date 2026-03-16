#include "reflection/BuiltinComponentMetadata.h"

#include "ecs/Component.h"
#include "reflection/TypeInfo.h"

namespace Feliss::Reflection {

namespace {

void registerTagComponent() {
    TypeInfo info;
    info.name = "TagComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<TagComponent>("name", "string", PropertyKind::String, &TagComponent::name, Editable | Serializable | Scriptable));
    info.fields.push_back(makeField<TagComponent>("tag", "string", PropertyKind::String, &TagComponent::tag, Editable | Serializable | Scriptable));
    info.fields.push_back(makeField<TagComponent>("layer", "u32", PropertyKind::UInt, &TagComponent::layer, Editable | Serializable));
    info.fields.push_back(makeField<TagComponent>("active", "bool", PropertyKind::Bool, &TagComponent::active, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerTransformComponent() {
    TypeInfo info;
    info.name = "TransformComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<TransformComponent>("position", "Vec3", PropertyKind::Vec3, &TransformComponent::position, Editable | Serializable | Scriptable));
    info.fields.push_back(makeField<TransformComponent>("rotation", "Quat", PropertyKind::Quat, &TransformComponent::rotation, Editable | Serializable | Scriptable));
    info.fields.push_back(makeField<TransformComponent>("scale", "Vec3", PropertyKind::Vec3, &TransformComponent::scale, Editable | Serializable | Scriptable));
    Registry::instance().registerType(std::move(info));
}

void registerMeshRendererComponent() {
    TypeInfo info;
    info.name = "MeshRendererComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<MeshRendererComponent>("meshID", "AssetID", PropertyKind::AssetId, &MeshRendererComponent::meshID, Editable | Serializable));
    info.fields.push_back(makeField<MeshRendererComponent>("materialID", "AssetID", PropertyKind::AssetId, &MeshRendererComponent::materialID, Editable | Serializable));
    info.fields.push_back(makeField<MeshRendererComponent>("castShadow", "bool", PropertyKind::Bool, &MeshRendererComponent::castShadow, Editable | Serializable));
    info.fields.push_back(makeField<MeshRendererComponent>("receiveShadow", "bool", PropertyKind::Bool, &MeshRendererComponent::receiveShadow, Editable | Serializable));
    info.fields.push_back(makeField<MeshRendererComponent>("renderLayer", "u32", PropertyKind::UInt, &MeshRendererComponent::renderLayer, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerSpriteComponent() {
    TypeInfo info;
    info.name = "SpriteComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<SpriteComponent>("textureID", "AssetID", PropertyKind::AssetId, &SpriteComponent::textureID, Editable | Serializable));
    info.fields.push_back(makeField<SpriteComponent>("tint", "Color", PropertyKind::Color, &SpriteComponent::tint, Editable | Serializable));
    info.fields.push_back(makeField<SpriteComponent>("size", "Vec2", PropertyKind::Vec2, &SpriteComponent::size, Editable | Serializable));
    info.fields.push_back(makeField<SpriteComponent>("pivot", "Vec2", PropertyKind::Vec2, &SpriteComponent::pivot, Editable | Serializable));
    info.fields.push_back(makeEnumField<SpriteComponent>("blendMode", "BlendMode", &SpriteComponent::blendMode, Editable | Serializable, {"Opaque", "Alpha", "Additive", "Multiply", "Screen"}));
    info.fields.push_back(makeField<SpriteComponent>("sortOrder", "int", PropertyKind::Int, &SpriteComponent::sortOrder, Editable | Serializable));
    info.fields.push_back(makeField<SpriteComponent>("flipX", "bool", PropertyKind::Bool, &SpriteComponent::flipX, Editable | Serializable));
    info.fields.push_back(makeField<SpriteComponent>("flipY", "bool", PropertyKind::Bool, &SpriteComponent::flipY, Editable | Serializable));
    info.fields.push_back(makeField<SpriteComponent>("billboard", "bool", PropertyKind::Bool, &SpriteComponent::billboard, Editable | Serializable));
    info.fields.push_back(makeField<SpriteComponent>("pixelSnap", "bool", PropertyKind::Bool, &SpriteComponent::pixelSnap, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerCameraComponent() {
    TypeInfo info;
    info.name = "CameraComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeEnumField<CameraComponent>("projection", "CameraProjection", &CameraComponent::projection, Editable | Serializable, {"Perspective", "Orthographic"}));
    info.fields.push_back(makeField<CameraComponent>("fov", "float", PropertyKind::Float, &CameraComponent::fov, Editable | Serializable));
    info.fields.push_back(makeField<CameraComponent>("orthoSize", "float", PropertyKind::Float, &CameraComponent::orthoSize, Editable | Serializable));
    info.fields.push_back(makeField<CameraComponent>("nearClip", "float", PropertyKind::Float, &CameraComponent::nearClip, Editable | Serializable));
    info.fields.push_back(makeField<CameraComponent>("farClip", "float", PropertyKind::Float, &CameraComponent::farClip, Editable | Serializable));
    info.fields.push_back(makeField<CameraComponent>("clearColor", "Color", PropertyKind::Color, &CameraComponent::clearColor, Editable | Serializable));
    info.fields.push_back(makeField<CameraComponent>("isMain", "bool", PropertyKind::Bool, &CameraComponent::isMain, Editable | Serializable));
    info.fields.push_back(makeField<CameraComponent>("cullingMask", "u32", PropertyKind::UInt, &CameraComponent::cullingMask, Editable | Serializable));
    info.fields.push_back(makeField<CameraComponent>("renderTargetID", "AssetID", PropertyKind::AssetId, &CameraComponent::renderTargetID, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerLightComponent() {
    TypeInfo info;
    info.name = "LightComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeEnumField<LightComponent>("type", "LightType", &LightComponent::type, Editable | Serializable, {"Directional", "Point", "Spot", "Area"}));
    info.fields.push_back(makeField<LightComponent>("color", "Color", PropertyKind::Color, &LightComponent::color, Editable | Serializable));
    info.fields.push_back(makeField<LightComponent>("intensity", "float", PropertyKind::Float, &LightComponent::intensity, Editable | Serializable));
    info.fields.push_back(makeField<LightComponent>("range", "float", PropertyKind::Float, &LightComponent::range, Editable | Serializable));
    info.fields.push_back(makeField<LightComponent>("spotInner", "float", PropertyKind::Float, &LightComponent::spotInner, Editable | Serializable));
    info.fields.push_back(makeField<LightComponent>("spotOuter", "float", PropertyKind::Float, &LightComponent::spotOuter, Editable | Serializable));
    info.fields.push_back(makeField<LightComponent>("castShadow", "bool", PropertyKind::Bool, &LightComponent::castShadow, Editable | Serializable));
    info.fields.push_back(makeField<LightComponent>("shadowBias", "float", PropertyKind::Float, &LightComponent::shadowBias, Editable | Serializable));
    info.fields.push_back(makeField<LightComponent>("shadowResolution", "int", PropertyKind::Int, &LightComponent::shadowResolution, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerRigidBodyComponent() {
    TypeInfo info;
    info.name = "RigidBodyComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeEnumField<RigidBodyComponent>("type", "RigidBodyType", &RigidBodyComponent::type, Editable | Serializable, {"Static", "Kinematic", "Dynamic"}));
    info.fields.push_back(makeField<RigidBodyComponent>("mass", "float", PropertyKind::Float, &RigidBodyComponent::mass, Editable | Serializable));
    info.fields.push_back(makeField<RigidBodyComponent>("drag", "float", PropertyKind::Float, &RigidBodyComponent::drag, Editable | Serializable));
    info.fields.push_back(makeField<RigidBodyComponent>("angularDrag", "float", PropertyKind::Float, &RigidBodyComponent::angularDrag, Editable | Serializable));
    info.fields.push_back(makeField<RigidBodyComponent>("useGravity", "bool", PropertyKind::Bool, &RigidBodyComponent::useGravity, Editable | Serializable));
    info.fields.push_back(makeField<RigidBodyComponent>("isTrigger", "bool", PropertyKind::Bool, &RigidBodyComponent::isTrigger, Editable | Serializable));
    info.fields.push_back(makeField<RigidBodyComponent>("linearVelocity", "Vec3", PropertyKind::Vec3, &RigidBodyComponent::linearVelocity, Editable | Serializable));
    info.fields.push_back(makeField<RigidBodyComponent>("angularVelocity", "Vec3", PropertyKind::Vec3, &RigidBodyComponent::angularVelocity, Editable | Serializable));
    info.fields.push_back(makeField<RigidBodyComponent>("physicsLayer", "u32", PropertyKind::UInt, &RigidBodyComponent::physicsLayer, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerBoxColliderComponent() {
    TypeInfo info;
    info.name = "BoxColliderComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<BoxColliderComponent>("center", "Vec3", PropertyKind::Vec3, &BoxColliderComponent::center, Editable | Serializable));
    info.fields.push_back(makeField<BoxColliderComponent>("size", "Vec3", PropertyKind::Vec3, &BoxColliderComponent::size, Editable | Serializable));
    info.fields.push_back(makeField<BoxColliderComponent>("friction", "float", PropertyKind::Float, &BoxColliderComponent::friction, Editable | Serializable));
    info.fields.push_back(makeField<BoxColliderComponent>("restitution", "float", PropertyKind::Float, &BoxColliderComponent::restitution, Editable | Serializable));
    info.fields.push_back(makeField<BoxColliderComponent>("isTrigger", "bool", PropertyKind::Bool, &BoxColliderComponent::isTrigger, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerSphereColliderComponent() {
    TypeInfo info;
    info.name = "SphereColliderComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<SphereColliderComponent>("center", "Vec3", PropertyKind::Vec3, &SphereColliderComponent::center, Editable | Serializable));
    info.fields.push_back(makeField<SphereColliderComponent>("radius", "float", PropertyKind::Float, &SphereColliderComponent::radius, Editable | Serializable));
    info.fields.push_back(makeField<SphereColliderComponent>("friction", "float", PropertyKind::Float, &SphereColliderComponent::friction, Editable | Serializable));
    info.fields.push_back(makeField<SphereColliderComponent>("restitution", "float", PropertyKind::Float, &SphereColliderComponent::restitution, Editable | Serializable));
    info.fields.push_back(makeField<SphereColliderComponent>("isTrigger", "bool", PropertyKind::Bool, &SphereColliderComponent::isTrigger, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerAudioSourceComponent() {
    TypeInfo info;
    info.name = "AudioSourceComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<AudioSourceComponent>("clipID", "AssetID", PropertyKind::AssetId, &AudioSourceComponent::clipID, Editable | Serializable));
    info.fields.push_back(makeField<AudioSourceComponent>("volume", "float", PropertyKind::Float, &AudioSourceComponent::volume, Editable | Serializable));
    info.fields.push_back(makeField<AudioSourceComponent>("pitch", "float", PropertyKind::Float, &AudioSourceComponent::pitch, Editable | Serializable));
    info.fields.push_back(makeField<AudioSourceComponent>("minDist", "float", PropertyKind::Float, &AudioSourceComponent::minDist, Editable | Serializable));
    info.fields.push_back(makeField<AudioSourceComponent>("maxDist", "float", PropertyKind::Float, &AudioSourceComponent::maxDist, Editable | Serializable));
    info.fields.push_back(makeField<AudioSourceComponent>("loop", "bool", PropertyKind::Bool, &AudioSourceComponent::loop, Editable | Serializable));
    info.fields.push_back(makeField<AudioSourceComponent>("playOnAwake", "bool", PropertyKind::Bool, &AudioSourceComponent::playOnAwake, Editable | Serializable));
    info.fields.push_back(makeField<AudioSourceComponent>("spatial3D", "bool", PropertyKind::Bool, &AudioSourceComponent::spatial3D, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerAudioListenerComponent() {
    TypeInfo info;
    info.name = "AudioListenerComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<AudioListenerComponent>("isMain", "bool", PropertyKind::Bool, &AudioListenerComponent::isMain, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerAnimatorComponent() {
    TypeInfo info;
    info.name = "AnimatorComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<AnimatorComponent>("controllerID", "AssetID", PropertyKind::AssetId, &AnimatorComponent::controllerID, Editable | Serializable));
    info.fields.push_back(makeField<AnimatorComponent>("speed", "float", PropertyKind::Float, &AnimatorComponent::speed, Editable | Serializable));
    info.fields.push_back(makeField<AnimatorComponent>("currentState", "string", PropertyKind::String, &AnimatorComponent::currentState, Editable | Serializable));
    info.fields.push_back(makeField<AnimatorComponent>("playing", "bool", PropertyKind::Bool, &AnimatorComponent::playing, Editable | Serializable));
    info.fields.push_back(makeField<AnimatorComponent>("loop", "bool", PropertyKind::Bool, &AnimatorComponent::loop, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

void registerParticleSystemComponent() {
    TypeInfo info;
    info.name = "ParticleSystemComponent";
    info.baseName = "ComponentBase";
    info.fields.push_back(makeField<ParticleSystemComponent>("maxParticles", "u32", PropertyKind::UInt, &ParticleSystemComponent::maxParticles, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("emitRate", "float", PropertyKind::Float, &ParticleSystemComponent::emitRate, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("lifetime", "float", PropertyKind::Float, &ParticleSystemComponent::lifetime, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("startVelocity", "Vec3", PropertyKind::Vec3, &ParticleSystemComponent::startVelocity, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("gravity", "Vec3", PropertyKind::Vec3, &ParticleSystemComponent::gravity, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("startColor", "Color", PropertyKind::Color, &ParticleSystemComponent::startColor, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("endColor", "Color", PropertyKind::Color, &ParticleSystemComponent::endColor, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("startSize", "float", PropertyKind::Float, &ParticleSystemComponent::startSize, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("endSize", "float", PropertyKind::Float, &ParticleSystemComponent::endSize, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("materialID", "AssetID", PropertyKind::AssetId, &ParticleSystemComponent::materialID, Editable | Serializable));
    info.fields.push_back(makeField<ParticleSystemComponent>("playing", "bool", PropertyKind::Bool, &ParticleSystemComponent::playing, Editable | Serializable));
    Registry::instance().registerType(std::move(info));
}

} // namespace

void registerBuiltinComponentMetadata() {
    static bool registered = false;
    if (registered) {
        return;
    }

    registerTagComponent();
    registerTransformComponent();
    registerMeshRendererComponent();
    registerSpriteComponent();
    registerCameraComponent();
    registerLightComponent();
    registerRigidBodyComponent();
    registerBoxColliderComponent();
    registerSphereColliderComponent();
    registerAudioSourceComponent();
    registerAudioListenerComponent();
    registerAnimatorComponent();
    registerParticleSystemComponent();
    registered = true;
}

} // namespace Feliss::Reflection

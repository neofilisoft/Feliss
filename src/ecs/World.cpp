#include "ecs/World.h"
#include "renderer/RenderPipeline.h"
#include "core/Logger.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

namespace Feliss {

namespace {

Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 out{};
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            out.m[row][col] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                out.m[row][col] += a.m[row][k] * b.m[k][col];
            }
        }
    }
    return out;
}

Mat4 translation(const Vec3& position) {
    Mat4 out = Mat4::identity();
    out.m[0][3] = position.x;
    out.m[1][3] = position.y;
    out.m[2][3] = position.z;
    return out;
}

Mat4 scaling(const Vec3& scale) {
    Mat4 out = Mat4::identity();
    out.m[0][0] = scale.x;
    out.m[1][1] = scale.y;
    out.m[2][2] = scale.z;
    return out;
}

Mat4 rotation(const Quat& q) {
    const f32 xx = q.x * q.x;
    const f32 yy = q.y * q.y;
    const f32 zz = q.z * q.z;
    const f32 xy = q.x * q.y;
    const f32 xz = q.x * q.z;
    const f32 yz = q.y * q.z;
    const f32 wx = q.w * q.x;
    const f32 wy = q.w * q.y;
    const f32 wz = q.w * q.z;

    Mat4 out = Mat4::identity();
    out.m[0][0] = 1.0f - 2.0f * (yy + zz);
    out.m[0][1] = 2.0f * (xy - wz);
    out.m[0][2] = 2.0f * (xz + wy);
    out.m[1][0] = 2.0f * (xy + wz);
    out.m[1][1] = 1.0f - 2.0f * (xx + zz);
    out.m[1][2] = 2.0f * (yz - wx);
    out.m[2][0] = 2.0f * (xz - wy);
    out.m[2][1] = 2.0f * (yz + wx);
    out.m[2][2] = 1.0f - 2.0f * (xx + yy);
    return out;
}

Mat4 composeTransform(const TransformComponent& transform) {
    return multiply(translation(transform.position), multiply(rotation(transform.rotation), scaling(transform.scale)));
}

Mat4 transposeRotation(const Mat4& matrix) {
    Mat4 out = Mat4::identity();
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            out.m[row][col] = matrix.m[col][row];
        }
    }
    return out;
}

Vec3 extractTranslation(const Mat4& matrix) {
    return {matrix.m[0][3], matrix.m[1][3], matrix.m[2][3]};
}

Mat4 inverseCameraView(const Mat4& worldMatrix) {
    const Mat4 invRotation = transposeRotation(worldMatrix);
    const Vec3 position = extractTranslation(worldMatrix);
    Mat4 translationOnly = Mat4::identity();
    translationOnly.m[0][3] = -position.x;
    translationOnly.m[1][3] = -position.y;
    translationOnly.m[2][3] = -position.z;
    return multiply(invRotation, translationOnly);
}

Mat4 perspectiveProjection(f32 fovDegrees, f32 aspect, f32 nearClip, f32 farClip) {
    const f32 tanHalfFov = std::tan((fovDegrees * 0.01745329251994329577f) * 0.5f);
    Mat4 out {};
    out.m[0][0] = 1.0f / (aspect * tanHalfFov);
    out.m[1][1] = 1.0f / tanHalfFov;
    out.m[2][2] = -(farClip + nearClip) / (farClip - nearClip);
    out.m[2][3] = -(2.0f * farClip * nearClip) / (farClip - nearClip);
    out.m[3][2] = -1.0f;
    out.m[3][3] = 0.0f;
    return out;
}

Mat4 orthographicProjection(f32 orthoSize, f32 aspect, f32 nearClip, f32 farClip) {
    const f32 halfHeight = orthoSize * 0.5f;
    const f32 halfWidth = halfHeight * aspect;
    Mat4 out = Mat4::identity();
    out.m[0][0] = 1.0f / halfWidth;
    out.m[1][1] = 1.0f / halfHeight;
    out.m[2][2] = -2.0f / (farClip - nearClip);
    out.m[2][3] = -(farClip + nearClip) / (farClip - nearClip);
    return out;
}

std::string trim(std::string text) {
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(begin, end - begin + 1);
}

bool parseBool(const std::string& value) {
    return value == "1" || value == "true" || value == "on";
}

Vec2 parseVec2(const std::string& value) {
    std::istringstream stream(value);
    Vec2 out {};
    stream >> out.x >> out.y;
    return out;
}

Vec3 parseVec3(const std::string& value) {
    std::istringstream stream(value);
    Vec3 out {};
    stream >> out.x >> out.y >> out.z;
    return out;
}

Quat parseQuat(const std::string& value) {
    std::istringstream stream(value);
    Quat out {};
    stream >> out.x >> out.y >> out.z >> out.w;
    return out;
}

Color parseColor(const std::string& value) {
    std::istringstream stream(value);
    Color out {};
    stream >> out.r >> out.g >> out.b >> out.a;
    return out;
}

template<typename T>
std::string joinScalars(std::initializer_list<T> values) {
    std::ostringstream stream;
    bool first = true;
    for (const auto& value : values) {
        if (!first) {
            stream << ' ';
        }
        first = false;
        stream << value;
    }
    return stream.str();
}

struct SceneEntityData {
    EntityID savedId = NULL_ENTITY;
    EntityID parentId = NULL_ENTITY;
    std::string name = "Entity";
    std::string tag;
    bool active = true;

    bool hasTransform = false;
    Vec3 position = Vec3::zero();
    Quat rotation = Quat::identity();
    Vec3 scale = Vec3::one();

    bool hasMesh = false;
    MeshRendererComponent mesh {};

    bool hasSprite = false;
    SpriteComponent sprite {};

    bool hasCamera = false;
    CameraComponent camera {};

    bool hasLight = false;
    LightComponent light {};
};

} // namespace

const std::vector<EntityID> World::s_noChildren;

World::World()  { FLS_INFO("World", "ECS World created"); }
World::~World() { clear(); }

EntityID World::createEntity(const std::string& name) {
    EntityID id = generateID();
    EntityRecord rec;
    rec.id   = id;
    rec.name = name.empty() ? "Entity" : name;
    // Default components
    rec.comps[std::type_index(typeid(TagComponent))] =
        std::make_unique<TagComponent>(rec.name);
    rec.comps[std::type_index(typeid(TransformComponent))] =
        std::make_unique<TransformComponent>();
    m_entities[id] = std::move(rec);
    if (onEntityCreated) onEntityCreated(id);
    return id;
}

void World::detachFromParent(EntityID id) {
    EntityID par = getParent(id);
    if (par != NULL_ENTITY) {
        auto it = m_entities.find(par);
        if (it != m_entities.end()) {
            auto& ch = it->second.children;
            ch.erase(std::remove(ch.begin(), ch.end(), id), ch.end());
        }
    }
}

void World::destroyEntity(EntityID id) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) return;

    detachFromParent(id);

    // Recursively destroy children (copy first to avoid iterator invalidation)
    std::vector<EntityID> children = it->second.children;
    for (EntityID c : children) destroyEntity(c);

    if (onEntityDestroyed) onEntityDestroyed(id);
    m_entities.erase(id);
}

bool          World::isValid(EntityID id) const { return m_entities.count(id) > 0; }
void          World::setActive(EntityID id, bool v) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) return;
    it->second.active = v;
    if (auto* tag = getComponent<TagComponent>(id)) tag->active = v;
}
bool          World::isActive(EntityID id) const {
    auto it = m_entities.find(id); return it != m_entities.end() && it->second.active;
}
void          World::setName(EntityID id, const std::string& n) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) return;
    it->second.name = n;
    if (auto* t = getComponent<TagComponent>(id)) t->name = n;
}
std::string   World::getName(EntityID id) const {
    auto it = m_entities.find(id); return it != m_entities.end() ? it->second.name : "";
}
void          World::setTag(EntityID id, const std::string& t) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) return;
    it->second.tag = t;
    if (auto* tc = getComponent<TagComponent>(id)) tc->tag = t;
}
std::string   World::getTag(EntityID id) const {
    auto it = m_entities.find(id); return it != m_entities.end() ? it->second.tag : "";
}

void World::setParent(EntityID child, EntityID parent) {
    if (!isValid(child)) return;
    detachFromParent(child);
    m_entities[child].parent = parent;
    if (parent != NULL_ENTITY && isValid(parent))
        m_entities[parent].children.push_back(child);
}

EntityID World::getParent(EntityID child) const {
    auto it = m_entities.find(child);
    return it != m_entities.end() ? it->second.parent : NULL_ENTITY;
}

const std::vector<EntityID>& World::getChildren(EntityID id) const {
    auto it = m_entities.find(id);
    return it != m_entities.end() ? it->second.children : s_noChildren;
}

std::vector<EntityID> World::getRoots() const {
    std::vector<EntityID> roots;
    for (auto& [id, rec] : m_entities)
        if (rec.parent == NULL_ENTITY) roots.push_back(id);
    return roots;
}

EntityID World::findByName(const std::string& n) const {
    for (auto& [id, rec] : m_entities) if (rec.name == n) return id;
    return NULL_ENTITY;
}

std::vector<EntityID> World::findByTag(const std::string& t) const {
    std::vector<EntityID> res;
    for (auto& [id, rec] : m_entities) if (rec.tag == t) res.push_back(id);
    return res;
}

void World::update(f32 dt) {
    std::unordered_set<EntityID> resolved;
    std::unordered_set<EntityID> resolving;

    std::function<void(EntityID)> resolveTransform = [&](EntityID id) {
        if (resolved.count(id) > 0 || !isValid(id)) {
            return;
        }
        if (resolving.count(id) > 0) {
            FLS_WARNF("World", "Detected transform cycle while resolving entity " << id);
            return;
        }

        auto* transform = getComponent<TransformComponent>(id);
        if (!transform) {
            resolved.insert(id);
            return;
        }

        resolving.insert(id);
        const EntityID parent = getParent(id);
        const Mat4 local = composeTransform(*transform);
        if (parent != NULL_ENTITY && isValid(parent) && hasComponent<TransformComponent>(parent)) {
            resolveTransform(parent);
            transform->worldMatrix = multiply(getComponent<TransformComponent>(parent)->worldMatrix, local);
        } else {
            transform->worldMatrix = local;
        }
        transform->dirty = false;
        resolving.erase(id);
        resolved.insert(id);
    };

    each<TransformComponent>([&](EntityID id, TransformComponent&) {
        resolveTransform(id);
    });
}

void World::render(RenderPipeline& rp) {
    bool cameraBound = false;
    each<TransformComponent, CameraComponent>(
        [&](EntityID, TransformComponent& transform, CameraComponent& camera) {
            if (cameraBound || !camera.enabled || !camera.isMain) {
                return;
            }

            const f32 aspect = rp.activeRenderHeight() > 0
                ? static_cast<f32>(rp.activeRenderWidth()) / static_cast<f32>(rp.activeRenderHeight())
                : camera.aspectRatio;
            camera.aspectRatio = aspect;
            const Mat4 view = inverseCameraView(transform.worldMatrix);
            const Mat4 projection = camera.projection == CameraComponent::Projection::Perspective
                ? perspectiveProjection(camera.fov, aspect, camera.nearClip, camera.farClip)
                : orthographicProjection(camera.orthoSize, aspect, camera.nearClip, camera.farClip);
            rp.setCamera(multiply(projection, view), camera.clearColor);
            cameraBound = true;
        });
    if (!cameraBound) {
        rp.setCamera(Mat4::identity(), Color::black());
    }

    // Submit mesh renderers
    each<TransformComponent, MeshRendererComponent>(
        [&](EntityID, TransformComponent& t, MeshRendererComponent& mr) {
            if (!mr.enabled) return;
            rp.submitMesh(mr.meshID, mr.materialID, t.worldMatrix,
                          mr.castShadow, mr.renderLayer);
        });

    // Submit sprites
    each<TransformComponent, SpriteComponent>(
        [&](EntityID, TransformComponent& t, SpriteComponent& s) {
            if (!s.enabled) return;
            rp.submitSprite(s.textureID, s.tint, s.size,
                            t.worldMatrix, s.sortOrder);
        });

    // Submit lights
    each<TransformComponent, LightComponent>(
        [&](EntityID, TransformComponent& t, LightComponent& l) {
            if (!l.enabled) return;
            Vec3 dir = {0, -1, 0}; // TODO: derive from rotation
            rp.submitLight(static_cast<int>(l.type), l.color,
                           l.intensity, t.position, dir, l.range);
        });
}

void World::clear() {
    m_entities.clear();
    m_nextID = 1;
    FLS_INFO("World", "World cleared");
}

bool World::saveToFile(const std::string& path) const {
    std::ofstream f(path);
    if (!f.is_open()) {
        return false;
    }

    f << "feliss_scene 1\n";
    for (const auto& [id, rec] : m_entities) {
        f << "entity_begin\n";
        f << "id=" << id << '\n';
        f << "name=" << rec.name << '\n';
        f << "tag=" << rec.tag << '\n';
        f << "active=" << (rec.active ? 1 : 0) << '\n';
        f << "parent=" << rec.parent << '\n';

        if (const auto* transform = getComponent<TransformComponent>(id)) {
            f << "transform.position=" << joinScalars<f32>({transform->position.x, transform->position.y, transform->position.z}) << '\n';
            f << "transform.rotation=" << joinScalars<f32>({transform->rotation.x, transform->rotation.y, transform->rotation.z, transform->rotation.w}) << '\n';
            f << "transform.scale=" << joinScalars<f32>({transform->scale.x, transform->scale.y, transform->scale.z}) << '\n';
        }

        if (const auto* mesh = getComponent<MeshRendererComponent>(id)) {
            f << "mesh.material=" << mesh->materialID << '\n';
            f << "mesh.asset=" << mesh->meshID << '\n';
            f << "mesh.cast_shadow=" << (mesh->castShadow ? 1 : 0) << '\n';
            f << "mesh.receive_shadow=" << (mesh->receiveShadow ? 1 : 0) << '\n';
            f << "mesh.layer=" << mesh->renderLayer << '\n';
        }

        if (const auto* sprite = getComponent<SpriteComponent>(id)) {
            f << "sprite.texture=" << sprite->textureID << '\n';
            f << "sprite.tint=" << joinScalars<f32>({sprite->tint.r, sprite->tint.g, sprite->tint.b, sprite->tint.a}) << '\n';
            f << "sprite.size=" << joinScalars<f32>({sprite->size.x, sprite->size.y}) << '\n';
            f << "sprite.sort=" << sprite->sortOrder << '\n';
            f << "sprite.flip_x=" << (sprite->flipX ? 1 : 0) << '\n';
            f << "sprite.flip_y=" << (sprite->flipY ? 1 : 0) << '\n';
        }

        if (const auto* camera = getComponent<CameraComponent>(id)) {
            f << "camera.projection=" << static_cast<int>(camera->projection) << '\n';
            f << "camera.fov=" << camera->fov << '\n';
            f << "camera.ortho_size=" << camera->orthoSize << '\n';
            f << "camera.near=" << camera->nearClip << '\n';
            f << "camera.far=" << camera->farClip << '\n';
            f << "camera.aspect=" << camera->aspectRatio << '\n';
            f << "camera.clear_color=" << joinScalars<f32>({camera->clearColor.r, camera->clearColor.g, camera->clearColor.b, camera->clearColor.a}) << '\n';
            f << "camera.main=" << (camera->isMain ? 1 : 0) << '\n';
        }

        if (const auto* light = getComponent<LightComponent>(id)) {
            f << "light.type=" << static_cast<int>(light->type) << '\n';
            f << "light.color=" << joinScalars<f32>({light->color.r, light->color.g, light->color.b, light->color.a}) << '\n';
            f << "light.intensity=" << light->intensity << '\n';
            f << "light.range=" << light->range << '\n';
        }

        f << "entity_end\n";
    }

    FLS_INFOF("World", "Saved " << m_entities.size() << " entities -> " << path);
    return true;
}

bool World::loadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        FLS_ERRORF("World", "Failed to open scene: " << path);
        return false;
    }

    std::vector<SceneEntityData> pendingEntities;
    SceneEntityData current {};
    bool inEntity = false;
    bool sawHeader = false;
    std::string line;

    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        if (!sawHeader) {
            sawHeader = (line == "feliss_scene 1");
            if (!sawHeader) {
                FLS_ERRORF("World", "Unsupported scene format in " << path);
                return false;
            }
            continue;
        }
        if (line == "entity_begin") {
            current = SceneEntityData {};
            inEntity = true;
            continue;
        }
        if (line == "entity_end") {
            if (inEntity) {
                pendingEntities.push_back(current);
            }
            inEntity = false;
            continue;
        }
        if (!inEntity) {
            continue;
        }

        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            continue;
        }

        const std::string key = trim(line.substr(0, separator));
        const std::string value = trim(line.substr(separator + 1));

        if (key == "id") current.savedId = static_cast<EntityID>(std::stoull(value));
        else if (key == "name") current.name = value;
        else if (key == "tag") current.tag = value;
        else if (key == "active") current.active = parseBool(value);
        else if (key == "parent") current.parentId = static_cast<EntityID>(std::stoull(value));
        else if (key == "transform.position") { current.hasTransform = true; current.position = parseVec3(value); }
        else if (key == "transform.rotation") { current.hasTransform = true; current.rotation = parseQuat(value); }
        else if (key == "transform.scale") { current.hasTransform = true; current.scale = parseVec3(value); }
        else if (key == "mesh.material") { current.hasMesh = true; current.mesh.materialID = static_cast<AssetID>(std::stoull(value)); }
        else if (key == "mesh.asset") { current.hasMesh = true; current.mesh.meshID = static_cast<AssetID>(std::stoull(value)); }
        else if (key == "mesh.cast_shadow") { current.hasMesh = true; current.mesh.castShadow = parseBool(value); }
        else if (key == "mesh.receive_shadow") { current.hasMesh = true; current.mesh.receiveShadow = parseBool(value); }
        else if (key == "mesh.layer") { current.hasMesh = true; current.mesh.renderLayer = static_cast<u32>(std::stoul(value)); }
        else if (key == "sprite.texture") { current.hasSprite = true; current.sprite.textureID = static_cast<AssetID>(std::stoull(value)); }
        else if (key == "sprite.tint") { current.hasSprite = true; current.sprite.tint = parseColor(value); }
        else if (key == "sprite.size") { current.hasSprite = true; current.sprite.size = parseVec2(value); }
        else if (key == "sprite.sort") { current.hasSprite = true; current.sprite.sortOrder = std::stoi(value); }
        else if (key == "sprite.flip_x") { current.hasSprite = true; current.sprite.flipX = parseBool(value); }
        else if (key == "sprite.flip_y") { current.hasSprite = true; current.sprite.flipY = parseBool(value); }
        else if (key == "camera.projection") { current.hasCamera = true; current.camera.projection = static_cast<CameraComponent::Projection>(std::stoi(value)); }
        else if (key == "camera.fov") { current.hasCamera = true; current.camera.fov = std::stof(value); }
        else if (key == "camera.ortho_size") { current.hasCamera = true; current.camera.orthoSize = std::stof(value); }
        else if (key == "camera.near") { current.hasCamera = true; current.camera.nearClip = std::stof(value); }
        else if (key == "camera.far") { current.hasCamera = true; current.camera.farClip = std::stof(value); }
        else if (key == "camera.aspect") { current.hasCamera = true; current.camera.aspectRatio = std::stof(value); }
        else if (key == "camera.clear_color") { current.hasCamera = true; current.camera.clearColor = parseColor(value); }
        else if (key == "camera.main") { current.hasCamera = true; current.camera.isMain = parseBool(value); }
        else if (key == "light.type") { current.hasLight = true; current.light.type = static_cast<LightComponent::LightType>(std::stoi(value)); }
        else if (key == "light.color") { current.hasLight = true; current.light.color = parseColor(value); }
        else if (key == "light.intensity") { current.hasLight = true; current.light.intensity = std::stof(value); }
        else if (key == "light.range") { current.hasLight = true; current.light.range = std::stof(value); }
    }

    clear();
    std::unordered_map<EntityID, EntityID> entityMap;
    entityMap.reserve(pendingEntities.size());

    for (const auto& data : pendingEntities) {
        const EntityID newId = createEntity(data.name);
        entityMap[data.savedId] = newId;
        setActive(newId, data.active);
        setTag(newId, data.tag);

        if (auto* transform = getComponent<TransformComponent>(newId)) {
            if (data.hasTransform) {
                transform->position = data.position;
                transform->rotation = data.rotation;
                transform->scale = data.scale;
                transform->dirty = true;
            }
        }

        if (data.hasMesh) {
            auto& mesh = addComponent<MeshRendererComponent>(newId);
            mesh = data.mesh;
        }
        if (data.hasSprite) {
            auto& sprite = addComponent<SpriteComponent>(newId);
            sprite = data.sprite;
        }
        if (data.hasCamera) {
            auto& camera = addComponent<CameraComponent>(newId);
            camera = data.camera;
        }
        if (data.hasLight) {
            auto& light = addComponent<LightComponent>(newId);
            light = data.light;
        }
    }

    for (const auto& data : pendingEntities) {
        if (data.parentId == NULL_ENTITY) {
            continue;
        }
        const auto childIt = entityMap.find(data.savedId);
        const auto parentIt = entityMap.find(data.parentId);
        if (childIt != entityMap.end() && parentIt != entityMap.end()) {
            setParent(childIt->second, parentIt->second);
        }
    }

    FLS_INFOF("World", "Loaded " << pendingEntities.size() << " entities <- " << path);
    return true;
}

} // namespace Feliss

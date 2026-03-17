#include "physics/backends/Havok/HavokBackend.h"

#include "core/Logger.h"

namespace Feliss {

bool HavokBackend::init(const PhysicsConfig& config) {
    m_config = config;
    m_gravity = config.gravity;
    FLS_WARN("Physics", "Havok backend selected, but Havok bridge requires an external licensed SDK");
    return true;
}

void HavokBackend::shutdown() {
    FLS_INFO("Physics", "Havok backend shutdown");
}

void HavokBackend::step(f32) {
    // TODO: call Havok world step.
}

void HavokBackend::setGravity(Vec3 gravity) {
    m_gravity = gravity;
}

} // namespace Feliss

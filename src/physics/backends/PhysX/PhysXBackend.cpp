#include "physics/backends/PhysX/PhysXBackend.h"

#include "core/Logger.h"

namespace Feliss {

bool PhysXBackend::init(const PhysicsConfig& config) {
    m_config = config;
    m_gravity = config.gravity;
    FLS_WARN("Physics", "PhysX backend selected, but PhysX SDK bridge is not linked yet");
    return true;
}

void PhysXBackend::shutdown() {
    FLS_INFO("Physics", "PhysX backend shutdown");
}

void PhysXBackend::step(f32) {
    // TODO: call PhysX simulate/fetchResults.
}

void PhysXBackend::setGravity(Vec3 gravity) {
    m_gravity = gravity;
}

} // namespace Feliss

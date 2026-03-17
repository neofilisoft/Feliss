#include "physics/backends/AsterCore/AsterCoreBackend.h"

#include "core/Logger.h"

#include <filesystem>

namespace Feliss {

bool AsterCoreBackend::init(const PhysicsConfig& config) {
    m_config = config;
    m_gravity = config.gravity;

    const std::filesystem::path sdkPath("C:/Users/BEST/Desktop/Repo/AsterCorePhysics");
    if (std::filesystem::exists(sdkPath)) {
        FLS_INFOF("Physics", "AsterCore backend selected using SDK reference at " << sdkPath.string());
    } else {
        FLS_INFO("Physics", "AsterCore backend selected (SDK integration stub)");
    }
    return true;
}

void AsterCoreBackend::shutdown() {
    FLS_INFO("Physics", "AsterCore backend shutdown");
}

void AsterCoreBackend::step(f32) {
    // TODO: call AsterCore simulation once SDK wrapper is wired in.
}

void AsterCoreBackend::setGravity(Vec3 gravity) {
    m_gravity = gravity;
}

} // namespace Feliss

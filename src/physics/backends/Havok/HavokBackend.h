#pragma once

#include "physics/IPhysicsBackend.h"

namespace Feliss {

class HavokBackend final : public IPhysicsBackend {
public:
    const char* name() const override { return "Havok"; }
    bool init(const PhysicsConfig& config) override;
    void shutdown() override;
    void step(f32 dt) override;
    void setGravity(Vec3 gravity) override;

private:
    PhysicsConfig m_config;
    Vec3 m_gravity = {0.0f, -9.81f, 0.0f};
};

} // namespace Feliss

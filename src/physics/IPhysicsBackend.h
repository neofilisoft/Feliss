#pragma once

#include "physics/PhysicsTypes.h"

namespace Feliss {

class IPhysicsBackend {
public:
    virtual ~IPhysicsBackend() = default;

    virtual const char* name() const = 0;
    virtual bool init(const PhysicsConfig& config) = 0;
    virtual void shutdown() = 0;
    virtual void step(f32 dt) = 0;
    virtual void setGravity(Vec3 gravity) = 0;
};

} // namespace Feliss

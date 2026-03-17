#pragma once

#include "feliss/Types.h"

#include <string>

namespace Feliss {

enum class PhysicsBackendType : u32 {
    None = 0,
    AsterCore,
    PhysX,
    Havok,
};

inline const char* PhysicsBackendTypeToString(PhysicsBackendType backend) {
    switch (backend) {
        case PhysicsBackendType::AsterCore: return "AsterCore";
        case PhysicsBackendType::PhysX:     return "PhysX";
        case PhysicsBackendType::Havok:     return "Havok";
        default:                            return "None";
    }
}

inline PhysicsBackendType PhysicsBackendTypeFromString(const std::string& value) {
    if (value == "astercore") return PhysicsBackendType::AsterCore;
    if (value == "physx")     return PhysicsBackendType::PhysX;
    if (value == "havok")     return PhysicsBackendType::Havok;
    return PhysicsBackendType::None;
}

struct PhysicsConfig {
    Vec3  gravity       = {0, -9.81f, 0};
    int   subSteps      = 1;
    bool  debugDraw     = false;
    PhysicsBackendType backend = PhysicsBackendType::AsterCore;
};

} // namespace Feliss

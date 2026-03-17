#include "physics/PhysicsBackendFactory.h"

#include "physics/backends/AsterCore/AsterCoreBackend.h"
#include "physics/backends/Havok/HavokBackend.h"
#include "physics/backends/PhysX/PhysXBackend.h"

namespace Feliss::PhysicsBackendFactory {

Scope<IPhysicsBackend> create(PhysicsBackendType backendType) {
    switch (backendType) {
        case PhysicsBackendType::AsterCore: return MakeScope<AsterCoreBackend>();
        case PhysicsBackendType::PhysX:     return MakeScope<PhysXBackend>();
        case PhysicsBackendType::Havok:     return MakeScope<HavokBackend>();
        default:                            return nullptr;
    }
}

} // namespace Feliss::PhysicsBackendFactory

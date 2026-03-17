#pragma once

#include "feliss/Types.h"
#include "physics/PhysicsTypes.h"

namespace Feliss {

class IPhysicsBackend;

namespace PhysicsBackendFactory {

Scope<IPhysicsBackend> create(PhysicsBackendType backendType);

} // namespace PhysicsBackendFactory

} // namespace Feliss

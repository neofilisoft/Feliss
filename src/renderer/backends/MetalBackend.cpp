#include "renderer/backends/MetalBackend.h"

#include "core/Logger.h"

namespace Feliss {

bool MetalBackend::init(Window&) {
#ifdef __APPLE__
    FLS_WARN("Metal", "Metal backend skeleton is wired, but device/command queue implementation is not finished yet");
#else
    FLS_ERROR("Metal", "Metal backend is only available on Apple platforms");
#endif
    return false;
}

void MetalBackend::shutdown() {}

} // namespace Feliss

#include "feliss/Types.h"

#include <iomanip>
#include <random>
#include <sstream>

namespace Feliss {

UUID UUID::generate() {
    static thread_local std::mt19937_64 rng(std::random_device{}());
    UUID uuid;
    uuid.hi = rng();
    uuid.lo = rng();
    return uuid;
}

std::string UUID::toString() const {
    std::ostringstream out;
    out << std::hex << std::setfill('0')
        << std::setw(16) << hi
        << std::setw(16) << lo;
    return out.str();
}

} // namespace Feliss

#pragma once

#include "feliss/Types.h"

#include <string_view>
#include <vector>

namespace Feliss {

class World;
struct ComponentBase;

namespace ComponentFactory {

ComponentBase* addComponentByTypeName(World& world, EntityID id, std::string_view typeName);
ComponentBase* getComponentByTypeName(World& world, EntityID id, std::string_view typeName);
bool hasComponentByTypeName(const World& world, EntityID id, std::string_view typeName);
bool removeComponentByTypeName(World& world, EntityID id, std::string_view typeName);
const std::vector<std::string_view>& addableComponentTypes();

} // namespace ComponentFactory

} // namespace Feliss

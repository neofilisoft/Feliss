#include "reflection/TypeInfo.h"

namespace Feliss::Reflection {

Registry& Registry::instance() {
    static Registry registry;
    return registry;
}

void Registry::registerType(TypeInfo typeInfo) {
    m_types[typeInfo.name] = std::move(typeInfo);
}

const TypeInfo* Registry::find(std::string_view name) const {
    const auto it = m_types.find(std::string(name));
    return it != m_types.end() ? &it->second : nullptr;
}

std::vector<const TypeInfo*> Registry::all() const {
    std::vector<const TypeInfo*> types;
    types.reserve(m_types.size());
    for (const auto& [_, typeInfo] : m_types) {
        types.push_back(&typeInfo);
    }
    return types;
}

void* fieldPointer(void* instance, const FieldInfo& field) {
    return field.mutableAccessor ? field.mutableAccessor(instance) : nullptr;
}

const void* fieldPointer(const void* instance, const FieldInfo& field) {
    return field.constAccessor ? field.constAccessor(instance) : nullptr;
}

} // namespace Feliss::Reflection

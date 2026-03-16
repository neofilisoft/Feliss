#pragma once

#include "feliss/Types.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Feliss::Reflection {

enum class PropertyKind : u32 {
    Unknown = 0,
    Bool,
    Int,
    UInt,
    Float,
    String,
    Vec2,
    Vec3,
    Vec4,
    Quat,
    Color,
    AssetId,
    EntityId,
    Enum,
};

enum MetadataFlags : u32 {
    Editable     = 1u << 0,
    Serializable = 1u << 1,
    Scriptable   = 1u << 2,
};

struct FieldInfo {
    std::string name;
    std::string typeName;
    PropertyKind kind = PropertyKind::Unknown;
    u32 flags = 0;
    std::vector<std::string> enumLabels;
    std::function<void*(void*)> mutableAccessor;
    std::function<const void*(const void*)> constAccessor;
};

struct TypeInfo {
    std::string name;
    std::string baseName;
    std::vector<FieldInfo> fields;
};

class Registry {
public:
    static Registry& instance();

    void registerType(TypeInfo typeInfo);
    const TypeInfo* find(std::string_view name) const;
    std::vector<const TypeInfo*> all() const;

private:
    std::unordered_map<std::string, TypeInfo> m_types;
};

void* fieldPointer(void* instance, const FieldInfo& field);
const void* fieldPointer(const void* instance, const FieldInfo& field);

template<typename TOwner, typename TField>
FieldInfo makeField(const char* name,
                    const char* typeName,
                    PropertyKind kind,
                    TField TOwner::* member,
                    u32 flags) {
    FieldInfo field;
    field.name = name;
    field.typeName = typeName;
    field.kind = kind;
    field.flags = flags;
    field.mutableAccessor = [member](void* instance) -> void* {
        auto* owner = static_cast<TOwner*>(instance);
        return static_cast<void*>(&(owner->*member));
    };
    field.constAccessor = [member](const void* instance) -> const void* {
        auto* owner = static_cast<const TOwner*>(instance);
        return static_cast<const void*>(&(owner->*member));
    };
    return field;
}

template<typename TOwner, typename TEnum>
FieldInfo makeEnumField(const char* name,
                        const char* typeName,
                        TEnum TOwner::* member,
                        u32 flags,
                        std::initializer_list<const char*> labels) {
    FieldInfo field = makeField<TOwner, TEnum>(name, typeName, PropertyKind::Enum, member, flags);
    field.enumLabels.reserve(labels.size());
    for (const char* label : labels) {
        field.enumLabels.emplace_back(label);
    }
    return field;
}

} // namespace Feliss::Reflection

#include "editor/InspectorPanel.h"

#include "core/Engine.h"
#include "ecs/Component.h"
#include "ecs/ComponentFactory.h"
#include "ecs/World.h"
#include "reflection/TypeInfo.h"

#ifdef FELISS_HAS_IMGUI
#  include <imgui.h>
#endif

#include <algorithm>
#include <array>
#include <cstring>
#include <string>
#include <vector>

namespace Feliss::EditorPanels {

#ifdef FELISS_HAS_IMGUI
namespace {

bool isCoreComponent(std::string_view typeName) {
    return typeName == "TagComponent" || typeName == "TransformComponent";
}

void drawEntityHeader(World& world, EntityID entityId) {
    bool active = world.isActive(entityId);
    if (ImGui::Checkbox("##entity_active", &active)) {
        world.setActive(entityId, active);
    }
    ImGui::SameLine();

    std::array<char, 256> nameBuffer {};
    const std::string currentName = world.getName(entityId);
    std::strncpy(nameBuffer.data(), currentName.c_str(), nameBuffer.size() - 1);
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("##entity_name", nameBuffer.data(), nameBuffer.size())) {
        world.setName(entityId, nameBuffer.data());
    }

    std::array<char, 128> tagBuffer {};
    const std::string currentTag = world.getTag(entityId);
    std::strncpy(tagBuffer.data(), currentTag.c_str(), tagBuffer.size() - 1);
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("Tag", tagBuffer.data(), tagBuffer.size())) {
        world.setTag(entityId, tagBuffer.data());
    }
    ImGui::Separator();
}

bool drawFieldValue(const Reflection::FieldInfo& field, void* instance) {
    void* data = Reflection::fieldPointer(instance, field);
    if (!data) {
        ImGui::TextDisabled("%s", field.name.c_str());
        return false;
    }

    switch (field.kind) {
        case Reflection::PropertyKind::Bool:
            return ImGui::Checkbox(field.name.c_str(), static_cast<bool*>(data));
        case Reflection::PropertyKind::Int:
            return ImGui::DragInt(field.name.c_str(), static_cast<int*>(data), 1.0f);
        case Reflection::PropertyKind::UInt: {
            auto* value = static_cast<u32*>(data);
            int temp = static_cast<int>(*value);
            const bool changed = ImGui::DragInt(field.name.c_str(), &temp, 1.0f, 0);
            if (changed && temp >= 0) {
                *value = static_cast<u32>(temp);
            }
            return changed;
        }
        case Reflection::PropertyKind::Float:
            return ImGui::DragFloat(field.name.c_str(), static_cast<float*>(data), 0.05f);
        case Reflection::PropertyKind::String: {
            auto* value = static_cast<std::string*>(data);
            std::array<char, 256> buffer {};
            std::strncpy(buffer.data(), value->c_str(), buffer.size() - 1);
            if (ImGui::InputText(field.name.c_str(), buffer.data(), buffer.size())) {
                *value = buffer.data();
                return true;
            }
            return false;
        }
        case Reflection::PropertyKind::Vec2:
            return ImGui::DragFloat2(field.name.c_str(), &static_cast<Vec2*>(data)->x, 0.05f);
        case Reflection::PropertyKind::Vec3:
            return ImGui::DragFloat3(field.name.c_str(), &static_cast<Vec3*>(data)->x, 0.05f);
        case Reflection::PropertyKind::Vec4:
            return ImGui::DragFloat4(field.name.c_str(), &static_cast<Vec4*>(data)->x, 0.05f);
        case Reflection::PropertyKind::Quat:
            return ImGui::DragFloat4(field.name.c_str(), &static_cast<Quat*>(data)->x, 0.05f);
        case Reflection::PropertyKind::Color:
            return ImGui::ColorEdit4(field.name.c_str(), &static_cast<Color*>(data)->r);
        case Reflection::PropertyKind::AssetId: {
            auto* value = static_cast<AssetID*>(data);
            unsigned long long temp = static_cast<unsigned long long>(*value);
            const bool changed = ImGui::InputScalar(field.name.c_str(), ImGuiDataType_U64, &temp);
            if (changed) {
                *value = static_cast<AssetID>(temp);
            }
            return changed;
        }
        case Reflection::PropertyKind::Enum: {
            if (field.enumLabels.empty()) {
                int value = 0;
                std::memcpy(&value, data, (std::min)(sizeof(value), sizeof(int)));
                const bool changed = ImGui::DragInt(field.name.c_str(), &value, 1.0f);
                if (changed) {
                    std::memcpy(data, &value, (std::min)(sizeof(value), sizeof(int)));
                }
                return changed;
            }
            int currentIndex = 0;
            std::memcpy(&currentIndex, data, (std::min)(sizeof(currentIndex), sizeof(int)));
            std::string items;
            for (const auto& label : field.enumLabels) {
                items += label;
                items.push_back('\0');
            }
            items.push_back('\0');
            const bool changed = ImGui::Combo(field.name.c_str(), &currentIndex, items.c_str());
            if (changed) {
                std::memcpy(data, &currentIndex, (std::min)(sizeof(currentIndex), sizeof(int)));
            }
            return changed;
        }
        default:
            ImGui::TextDisabled("%s (%s)", field.name.c_str(), field.typeName.c_str());
            return false;
    }
}

void drawComponentSection(World& world, EntityID entityId, ComponentBase& component) {
    const char* typeName = component.typeName();
    const Reflection::TypeInfo* typeInfo = Reflection::Registry::instance().find(typeName);
    ImGuiTreeNodeFlags headerFlags = isCoreComponent(typeName) ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    const bool open = ImGui::CollapsingHeader(typeName, headerFlags);

    if (!isCoreComponent(typeName)) {
        ImGui::SameLine();
        const std::string removeLabel = std::string("Remove##") + typeName;
        if (ImGui::SmallButton(removeLabel.c_str())) {
            ComponentFactory::removeComponentByTypeName(world, entityId, typeName);
            return;
        }
    }

    if (!open) {
        return;
    }

    if (!isCoreComponent(typeName)) {
        ImGui::Checkbox((std::string("Enabled##") + typeName).c_str(), &component.enabled);
    }

    if (!typeInfo) {
        ImGui::TextDisabled("No metadata registered");
        return;
    }

    for (const auto& field : typeInfo->fields) {
        if ((field.flags & Reflection::Editable) == 0) {
            continue;
        }
        drawFieldValue(field, &component);
    }
}

} // namespace
#endif

void DrawInspectorPanel(Engine& engine, EntityID selected) {
#ifdef FELISS_HAS_IMGUI
    if (!ImGui::Begin("Inspector")) {
        ImGui::End();
        return;
    }

    World& world = engine.world();
    if (selected == NULL_ENTITY || !world.isValid(selected)) {
        ImGui::TextDisabled("No entity selected");
        ImGui::End();
        return;
    }

    drawEntityHeader(world, selected);

    const auto entityIt = world.all().find(selected);
    if (entityIt != world.all().end()) {
        std::vector<ComponentBase*> components;
        components.reserve(entityIt->second.comps.size());
        for (const auto& [_, component] : entityIt->second.comps) {
            components.push_back(component.get());
        }
        std::sort(components.begin(), components.end(), [](const ComponentBase* lhs, const ComponentBase* rhs) {
            return std::strcmp(lhs->typeName(), rhs->typeName()) < 0;
        });

        for (ComponentBase* component : components) {
            if (component) {
                drawComponentSection(world, selected, *component);
            }
        }
    }

    ImGui::Spacing();
    if (ImGui::Button("Add Component", ImVec2(-1.0f, 0.0f))) {
        ImGui::OpenPopup("AddComponentPopup");
    }
    if (ImGui::BeginPopup("AddComponentPopup")) {
        for (const auto typeName : ComponentFactory::addableComponentTypes()) {
            const bool alreadyAdded = ComponentFactory::hasComponentByTypeName(world, selected, typeName);
            if (ImGui::MenuItem(typeName.data(), nullptr, false, !alreadyAdded)) {
                ComponentFactory::addComponentByTypeName(world, selected, typeName);
            }
        }
        ImGui::EndPopup();
    }

    ImGui::End();
#else
    (void)engine;
    (void)selected;
#endif
}

} // namespace Feliss::EditorPanels

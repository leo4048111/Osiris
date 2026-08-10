#pragma once

#include <Features/Visuals/EquipmentModelChanger/EquipmentModelChangerConfigVariables.h>
#include <Features/Visuals/EquipmentModelChanger/EquipmentModelPresets.h>
#include <HookContext/HookContextMacros.h>

template <typename HookContext, EquipmentModelPresetCategory category>
struct EquipmentModelPresetDropdownSelectionChangeHandler {
    explicit EquipmentModelPresetDropdownSelectionChangeHandler(HookContext& hookContext) noexcept
        : hookContext{hookContext}
    {
    }

    void onSelectionChanged(int selectedIndex)
    {
        const auto preset = equipment_model_presets::fromIndex<category>(selectedIndex);
        if (!preset)
            return;

        if constexpr (category == EquipmentModelPresetCategory::Knife) {
            SET_CONFIG_VAR(equipment_model_changer_vars::KnifeModelId,
                equipment_model_changer_vars::KnifeModelId::ValueType{preset->itemDefinitionIndex});
            SET_CONFIG_VAR(equipment_model_changer_vars::KnifePaintKitId,
                equipment_model_changer_vars::KnifePaintKitId::ValueType{preset->paintKitIndex});
        } else if constexpr (category == EquipmentModelPresetCategory::Gun) {
            SET_CONFIG_VAR(equipment_model_changer_vars::GunModelId,
                equipment_model_changer_vars::GunModelId::ValueType{preset->itemDefinitionIndex});
            SET_CONFIG_VAR(equipment_model_changer_vars::GunPaintKitId,
                equipment_model_changer_vars::GunPaintKitId::ValueType{preset->paintKitIndex});
        } else {
            SET_CONFIG_VAR(equipment_model_changer_vars::GloveModelId,
                equipment_model_changer_vars::GloveModelId::ValueType{preset->itemDefinitionIndex});
            SET_CONFIG_VAR(equipment_model_changer_vars::GlovePaintKitId,
                equipment_model_changer_vars::GlovePaintKitId::ValueType{preset->paintKitIndex});
        }
    }

private:
    HookContext& hookContext;
};

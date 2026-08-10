#pragma once

#include <cstdint>
#include <GameClient/Econ/FauxItemId.h>
#include <Features/Visuals/EquipmentModelChanger/EquipmentModelChangerConfigVariables.h>

template <typename HookContext>
class ViewmodelModPreviewPanel {
public:
    using RawType = cs2::CUI_Item3dPanel;

    ViewmodelModPreviewPanel(HookContext& hookContext, cs2::CUI_Item3dPanel* item3dPanel) noexcept
        : hookContext{hookContext}
        , item3dPanel{item3dPanel}
    {
    }

    void setupPreviewModel() const
    {
        auto&& previewPanel = panel();
        const auto desiredItemDefinitionIndex = previewItemDefinitionIndex();
        if (state().previewItemDefinitionIndex != desiredItemDefinitionIndex && previewPanel.portraitWorld().isMapLoaded()) {
            previewPanel.template as<UiItem3dPanel>().createItem(previewItemId(desiredItemDefinitionIndex));
            state().previewItemDefinitionIndex = desiredItemDefinitionIndex;
            state().hadPreviewWeaponHandle = false;
            state().recreatedPreviewWeapon = true;
        }
        if (hookContext.template make<EntitySystem>().getEntityFromHandle2(state().previewWeaponHandle)) {
            state().hadPreviewWeaponHandle = true;
            if (state().recreatedPreviewWeapon) {
                previewPanel.template as<UiItem3dPanel>().startWeaponLookAt();
                state().recreatedPreviewWeapon = false;
            }
            return;
        }

        auto&& portraitWorld = previewPanel.portraitWorld();
        if (state().hadPreviewWeaponHandle && portraitWorld.isMapLoaded()) {
            previewPanel.template as<UiItem3dPanel>().createItem(previewItemId(desiredItemDefinitionIndex));
            state().hadPreviewWeaponHandle = false;
            state().recreatedPreviewWeapon = true;
        }
        state().previewWeaponHandle = portraitWorld.findPreviewWeapon().baseEntity().handle();
    }

    void setFov() const
    {
        panel().setFov(fovForPreview());
    }

private:
    [[nodiscard]] decltype(auto) panel() const
    {
        return hookContext.template make<Ui3dPanel>(item3dPanel);
    }

    [[nodiscard]] float fovForPreview() const
    {
        auto&& viewmodelMod = hookContext.template make<ViewmodelMod>();
        if (viewmodelMod.fovModificationActive())
            return viewmodelMod.viewmodelFov();
        return viewmodelFovFromConVar();
    }

    [[nodiscard]] cs2::ItemDefinitionIndex previewItemDefinitionIndex() const noexcept
    {
        if (GET_CONFIG_VAR(equipment_model_changer_vars::Enabled)) {
            const auto configuredId = static_cast<std::uint16_t>(GET_CONFIG_VAR(equipment_model_changer_vars::KnifeModelId));
            if (configuredId != 0)
                return static_cast<cs2::ItemDefinitionIndex>(configuredId);
        }
        return cs2::ItemDefinitionIndex::M9Bayonet;
    }

    [[nodiscard]] static constexpr FauxItemId previewItemId(cs2::ItemDefinitionIndex itemDefinitionIndex) noexcept
    {
        if (itemDefinitionIndex == cs2::ItemDefinitionIndex::M9Bayonet)
            return {itemDefinitionIndex, cs2::PaintKitIndex::MarbleFade};
        return {itemDefinitionIndex};
    }

    [[nodiscard]] float viewmodelFovFromConVar() const
    {
        return GET_CONVAR_VALUE(cs2::viewmodel_fov).value_or(viewmodel_mod_params::kPreviewFallbackFov);
    }

    [[nodiscard]] auto& state() const
    {
        return hookContext.panoramaGuiState().viewmodelModPreviewPanelState;
    }

    HookContext& hookContext;
    cs2::CUI_Item3dPanel* item3dPanel;
};

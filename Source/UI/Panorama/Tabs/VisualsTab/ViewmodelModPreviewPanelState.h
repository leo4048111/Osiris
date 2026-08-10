#pragma once

#include <CS2/Constants/EntityHandle.h>
#include <CS2/Panorama/PanelHandle.h>
#include <CS2/Econ/ItemDefinitionIndex.h>

struct ViewmodelModPreviewPanelState {
    bool hadPreviewWeaponHandle{true};
    bool recreatedPreviewWeapon{false};
    cs2::CEntityHandle previewWeaponHandle{cs2::INVALID_EHANDLE_INDEX};
    cs2::ItemDefinitionIndex previewItemDefinitionIndex{cs2::ItemDefinitionIndex::M9Bayonet};
};

#pragma once

#include "ModelGlow/ModelGlowState.h"
#include "EquipmentModelChanger/EquipmentModelChangerState.h"
#include "PlayerInfoInWorld/PlayerInfoInWorldState.h"

struct VisualFeaturesStates {
    PlayerInfoInWorldState playerInfoInWorldState;
    ModelGlowState modelGlowState;
    EquipmentModelChangerState equipmentModelChangerState;
};

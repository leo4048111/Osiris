#pragma once

#include <Config/ConfigVariable.h>
#include "EquipmentModelChangerParams.h"

namespace equipment_model_changer_vars
{

CONFIG_VARIABLE(Enabled, bool, false);
CONFIG_VARIABLE_RANGE(KnifeModelId, equipment_model_changer_params::kItemDefinitionIndex);
CONFIG_VARIABLE_RANGE(GunModelId, equipment_model_changer_params::kItemDefinitionIndex);
CONFIG_VARIABLE_RANGE(GloveModelId, equipment_model_changer_params::kItemDefinitionIndex);
CONFIG_VARIABLE_RANGE(KnifePaintKitId, equipment_model_changer_params::kPaintKitIndex);
CONFIG_VARIABLE_RANGE(GunPaintKitId, equipment_model_changer_params::kPaintKitIndex);
CONFIG_VARIABLE_RANGE(GlovePaintKitId, equipment_model_changer_params::kPaintKitIndex);

}

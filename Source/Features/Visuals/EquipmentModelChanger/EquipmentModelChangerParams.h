#pragma once

#include <cstdint>
#include <Config/RangeConstrainedVariableParams.h>

namespace equipment_model_changer_params
{

// 0 keeps the model supplied by the game. Item definition indices are uint16.
constexpr auto kItemDefinitionIndex = RangeConstrainedVariableParams<std::uint16_t>{.min = 0, .max = 65535, .def = 0};
constexpr auto kPaintKitIndex = RangeConstrainedVariableParams<std::uint16_t>{.min = 0, .max = 65535, .def = 0};

}

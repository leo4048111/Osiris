#pragma once

#include <cstddef>
#include <cstdint>

namespace cs2
{

struct CEconItemDefinition;

struct C_EconItemView {
    using m_iItemDefinitionIndex = std::uint16_t;
    using m_bInitialized = bool;
    using m_bDisallowSOC = bool;
    using GetStaticData = CEconItemDefinition*(C_EconItemView* thisptr);
};

struct CEconItemDefinition {
    using GetWorldDisplayModel = const char*(*)(const CEconItemDefinition* thisptr);

    [[nodiscard]] const char* worldDisplayModel() const noexcept
    {
        const auto vmt = *reinterpret_cast<GetWorldDisplayModel* const*>(this);
        return vmt ? vmt[7](this) : nullptr;
    }
};

}

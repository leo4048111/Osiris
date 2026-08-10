#pragma once

#include <array>
#include <cstdint>

enum class EquipmentModelPresetCategory {
    Knife,
    Gun,
    Glove
};

struct EquipmentModelPreset {
    std::uint16_t itemDefinitionIndex;
    std::uint16_t paintKitIndex;
};

namespace equipment_model_presets
{

inline constexpr std::array knife{
    EquipmentModelPreset{0, 0},
    EquipmentModelPreset{500, 415}, // Bayonet | Doppler
    EquipmentModelPreset{503, 38},  // Classic Knife | Fade
    EquipmentModelPreset{505, 415}, // Flip Knife | Doppler
    EquipmentModelPreset{506, 558}, // Gut Knife | Lore
    EquipmentModelPreset{507, 413}, // Karambit | Marble Fade
    EquipmentModelPreset{508, 413}, // M9 Bayonet | Marble Fade
    EquipmentModelPreset{509, 409}, // Huntsman Knife | Tiger Tooth
    EquipmentModelPreset{515, 38},  // Butterfly Knife | Fade
    EquipmentModelPreset{523, 413}, // Talon Knife | Marble Fade
    EquipmentModelPreset{525, 38},  // Skeleton Knife | Fade
    EquipmentModelPreset{526, 38}   // Kukri Knife | Fade
};

inline constexpr std::array gun{
    EquipmentModelPreset{0, 0},
    EquipmentModelPreset{1, 37},    // Desert Eagle | Blaze
    EquipmentModelPreset{4, 38},    // Glock-18 | Fade
    EquipmentModelPreset{7, 282},   // AK-47 | Redline
    EquipmentModelPreset{9, 344},   // AWP | Dragon Lore
    EquipmentModelPreset{16, 255},  // M4A4 | Asiimov
    EquipmentModelPreset{19, 359},  // P90 | Asiimov
    EquipmentModelPreset{40, 624},  // SSG 08 | Dragonfire
    EquipmentModelPreset{60, 984},  // M4A1-S | Printstream
    EquipmentModelPreset{61, 504}   // USP-S | Kill Confirmed
};

inline constexpr std::array glove{
    EquipmentModelPreset{0, 0},
    EquipmentModelPreset{5027, 10006}, // Bloodhound Gloves | Charred
    EquipmentModelPreset{5030, 10037}, // Sport Gloves | Pandora's Box
    EquipmentModelPreset{5030, 10048}, // Sport Gloves | Vice
    EquipmentModelPreset{5031, 10041}, // Driver Gloves | King Snake
    EquipmentModelPreset{5032, 10053}, // Hand Wraps | Cobalt Skulls
    EquipmentModelPreset{5033, 10026}, // Moto Gloves | Spearmint
    EquipmentModelPreset{5034, 10033}, // Specialist Gloves | Crimson Kimono
    EquipmentModelPreset{5035, 10057}  // Hydra Gloves | Emerald
};

template <EquipmentModelPresetCategory category>
[[nodiscard]] constexpr const auto& presets() noexcept
{
    if constexpr (category == EquipmentModelPresetCategory::Knife)
        return knife;
    else if constexpr (category == EquipmentModelPresetCategory::Gun)
        return gun;
    else
        return glove;
}

template <EquipmentModelPresetCategory category>
[[nodiscard]] constexpr const EquipmentModelPreset* fromIndex(int index) noexcept
{
    const auto& values = presets<category>();
    if (index < 0 || static_cast<std::size_t>(index) >= values.size())
        return nullptr;
    return &values[static_cast<std::size_t>(index)];
}

template <EquipmentModelPresetCategory category>
[[nodiscard]] constexpr int indexOf(std::uint16_t itemDefinitionIndex, std::uint16_t paintKitIndex) noexcept
{
    const auto& values = presets<category>();
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (values[i].itemDefinitionIndex == itemDefinitionIndex && values[i].paintKitIndex == paintKitIndex)
            return static_cast<int>(i);
    }
    return 0;
}

}

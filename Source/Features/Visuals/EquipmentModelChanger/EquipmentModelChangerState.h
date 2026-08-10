#pragma once

#include <array>
#include <cstdint>

#include <CS2/Classes/EntitySystem/CEntityHandle.h>
#include <CS2/Classes/CEntitySubclassVDataBase.h>
#include <CS2/Constants/EntityHandle.h>

struct EquipmentModelSchemaOffsets {
    [[nodiscard]] bool weaponOffsetsValid() const noexcept
    {
        return econAttributeManager != 0 && attributeManagerItem != 0 && itemDefinitionIndex != 0
            && subclassId != 0 && weaponModelHandle != 0 && viewmodelAttachment != 0
            && weaponWorldModel != 0;
    }

    [[nodiscard]] bool gloveOffsetsValid() const noexcept
    {
        return econGloves != 0 && itemDefinitionIndex != 0 && needToReapplyGloves != 0 && econGlovesChanged != 0;
    }

    // client.dll schema revision 2026-08-03. Keeping these values together makes
    // a game-update refresh atomic and avoids touching SchemaSystem during injection.
    std::uint32_t econAttributeManager{0x11A8};
    std::uint32_t attributeManagerItem{0x50};
    std::uint32_t itemDefinitionIndex{0x1BA};
    std::uint32_t entityQuality{0x1BC};
    std::uint32_t itemId{0x1C8};
    std::uint32_t itemIdHigh{0x1D0};
    std::uint32_t itemIdLow{0x1D4};
    std::uint32_t itemInitialized{0x1E8};
    std::uint32_t itemDisallowSoc{0x1E9};
    std::uint32_t econAttributesInitialized{0x11A0};
    std::uint32_t subclassId{0x380};
    std::uint32_t fallbackPaintKit{0x1680};
    std::uint32_t fallbackSeed{0x1684};
    std::uint32_t fallbackWear{0x1688};
    std::uint32_t attachmentDirty{0x16B8};
    std::uint32_t weaponModelHandle{0x1760};
    std::uint32_t viewmodelAttachment{0x16B0};
    std::uint32_t visualsDataSet{0x18E9};
    std::uint32_t weaponWorldModel{0x28};
    std::uint32_t econGloves{0x1690};
    std::uint32_t needToReapplyGloves{0x168D};
    std::uint32_t econGlovesChanged{0x1B00};
};

struct ChangedWeaponModel {
    cs2::CEntityHandle handle{cs2::INVALID_EHANDLE_INDEX};
    std::uint16_t originalItemDefinitionIndex{};
    std::uint64_t originalItemId{};
    std::uint32_t originalItemIdHigh{};
    std::uint32_t originalItemIdLow{};
    std::int32_t originalEntityQuality{};
    std::int32_t originalFallbackPaintKit{};
    std::int32_t originalFallbackSeed{};
    float originalFallbackWear{};
    std::uint32_t originalSubclassId{};
    cs2::CEntitySubclassVDataBase* originalVData{};
    cs2::CEntityHandle weaponModelHandle{cs2::INVALID_EHANDLE_INDEX};
    cs2::CEntityHandle viewmodelAttachmentHandle{cs2::INVALID_EHANDLE_INDEX};
    std::uint8_t attachmentRefreshFrames{};
    bool knife{};
    bool seen{};
    bool active{};
};

struct ChangedGloveModel {
    cs2::CEntityHandle pawnHandle{cs2::INVALID_EHANDLE_INDEX};
    std::uint16_t originalItemDefinitionIndex{};
    std::uint64_t originalItemId{};
    std::uint32_t originalItemIdHigh{};
    std::uint32_t originalItemIdLow{};
    std::int32_t originalEntityQuality{};
    bool active{};
};

struct EquipmentModelChangerState {
    EquipmentModelSchemaOffsets offsets;
    std::array<ChangedWeaponModel, 64> changedWeapons;
    ChangedGloveModel changedGloves;
};

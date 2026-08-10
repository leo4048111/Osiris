#pragma once

#include <cstddef>
#include <cstdint>

#include <CS2/Classes/CEconItemView.h>
#include <CS2/Classes/CCSWeaponBaseVData.h>
#include <CS2/Classes/Entities/WeaponEntities.h>
#include <CS2/Econ/ItemId.h>
#include <GameClient/Entities/EntityClassifier.h>
#include <GameClient/Entities/PlayerPawn.h>
#include <GameClient/EntitySystem/EntitySystem.h>
#include <HookContext/HookContextMacros.h>
#include <MemoryPatterns/PatternTypes/EntityPatternTypes.h>
#include <MemoryPatterns/PatternTypes/BaseModelEntityPatternTypes.h>
#include <MemoryPatterns/PatternTypes/WeaponVDataPatternTypes.h>
#include <Platform/Macros/IsPlatform.h>
#include <Utils/StringBuilder.h>

#include "EquipmentModelChangerConfigVariables.h"
#include "EquipmentModelChangerState.h"

template <typename HookContext>
class EquipmentModelChanger {
public:
    explicit EquipmentModelChanger(HookContext& hookContext) noexcept
        : hookContext{hookContext}
    {
    }

    void run() const noexcept
    {
        resetSeenFlags();

        auto&& localPawn = hookContext.localPlayerController().pawn().template cast<PlayerPawn>();
        if (!GET_CONFIG_VAR(equipment_model_changer_vars::Enabled) || !localPawn) {
            restoreAll();
            return;
        }

        localPawn.weapons().forEach([this](auto&& weaponEntity) {
            applyWeapon(weaponEntity);
        });
        restoreWeaponsNoLongerCarried();
        applyGloves(localPawn);
    }

    void onUnload() const noexcept
    {
        restoreAll();
    }

private:
    template <typename T>
    [[nodiscard]] static T* field(void* object, std::uint32_t offset) noexcept
    {
        if (!object || offset == 0)
            return nullptr;
        return reinterpret_cast<T*>(reinterpret_cast<std::byte*>(object) + offset);
    }

    [[nodiscard]] cs2::C_EconItemView* weaponItemView(cs2::C_CSWeaponBase* weapon) const noexcept
    {
        if (!state().offsets.weaponOffsetsValid() || !weapon)
            return nullptr;
        auto attributeManager = reinterpret_cast<std::byte*>(weapon) + state().offsets.econAttributeManager;
        return reinterpret_cast<cs2::C_EconItemView*>(attributeManager + state().offsets.attributeManagerItem);
    }

    [[nodiscard]] std::uint16_t* itemDefinitionIndex(cs2::C_EconItemView* itemView) const noexcept
    {
        return field<std::uint16_t>(itemView, state().offsets.itemDefinitionIndex);
    }

    [[nodiscard]] std::uint64_t* itemId(cs2::C_EconItemView* itemView) const noexcept
    {
        return field<std::uint64_t>(itemView, state().offsets.itemId);
    }

    [[nodiscard]] std::uint32_t* itemIdHigh(cs2::C_EconItemView* itemView) const noexcept
    {
        return field<std::uint32_t>(itemView, state().offsets.itemIdHigh);
    }

    [[nodiscard]] std::uint32_t* itemIdLow(cs2::C_EconItemView* itemView) const noexcept
    {
        return field<std::uint32_t>(itemView, state().offsets.itemIdLow);
    }

    [[nodiscard]] std::int32_t* entityQuality(cs2::C_EconItemView* itemView) const noexcept
    {
        return field<std::int32_t>(itemView, state().offsets.entityQuality);
    }

    [[nodiscard]] std::int32_t* fallbackPaintKit(cs2::C_CSWeaponBase* weapon) const noexcept
    {
        return field<std::int32_t>(weapon, state().offsets.fallbackPaintKit);
    }

    [[nodiscard]] std::int32_t* fallbackSeed(cs2::C_CSWeaponBase* weapon) const noexcept
    {
        return field<std::int32_t>(weapon, state().offsets.fallbackSeed);
    }

    [[nodiscard]] float* fallbackWear(cs2::C_CSWeaponBase* weapon) const noexcept
    {
        return field<float>(weapon, state().offsets.fallbackWear);
    }

    void invalidateWeaponVisuals(cs2::C_CSWeaponBase* weapon) const noexcept
    {
        if (auto initialized = field<bool>(weapon, state().offsets.econAttributesInitialized))
            *initialized = false;
        if (auto attachmentIsDirty = field<bool>(weapon, state().offsets.attachmentDirty))
            *attachmentIsDirty = true;
        if (auto visualsAreSet = field<bool>(weapon, state().offsets.visualsDataSet))
            *visualsAreSet = false;
    }

    [[nodiscard]] cs2::CEntitySubclassVDataBase** weaponVData(cs2::C_CSWeaponBase* weapon) const noexcept
    {
        return hookContext.patternSearchResults().template get<OffsetToVData>()
            .of(static_cast<cs2::C_BaseEntity*>(weapon)).get();
    }

    [[nodiscard]] std::uint32_t* weaponSubclassId(cs2::C_CSWeaponBase* weapon) const noexcept
    {
        return field<std::uint32_t>(weapon, state().offsets.subclassId);
    }

    [[nodiscard]] cs2::CEntityHandle viewmodelAttachmentHandle(cs2::C_CSWeaponBase* weapon) const noexcept
    {
        if (const auto handle = field<cs2::CEntityHandle>(weapon, state().offsets.viewmodelAttachment))
            return *handle;
        return cs2::CEntityHandle{cs2::INVALID_EHANDLE_INDEX};
    }

    [[nodiscard]] cs2::CEntityHandle weaponModelHandle(cs2::C_CSWeaponBase* weapon) const noexcept
    {
        if (const auto handle = field<cs2::CEntityHandle>(weapon, state().offsets.weaponModelHandle))
            return *handle;
        return cs2::CEntityHandle{cs2::INVALID_EHANDLE_INDEX};
    }

    [[nodiscard]] const char* weaponWorldModel(cs2::CEntitySubclassVDataBase* vData) const noexcept
    {
        return field<const char>(vData, state().offsets.weaponWorldModel);
    }

    [[nodiscard]] bool changeWeaponRenderModels(cs2::C_CSWeaponBase* weapon,
        cs2::CEntitySubclassVDataBase* vData) const noexcept
    {
#if IS_WIN64()
        const auto model = weaponWorldModel(vData);
        const auto changeModel = hookContext.patternSearchResults().template get<ChangeModelFunction>();
        if (!weapon || !model || model[0] != 'm' || model[1] != 'o' || model[2] != 'd'
            || model[3] != 'e' || model[4] != 'l' || model[5] != 's' || model[6] != '/'
            || !changeModel)
            return false;

        // The held weapon is the primary first-person model. The two handles are
        // engine-managed companion entities used by the weapon and econ paths.
        changeModel(static_cast<cs2::C_BaseModelEntity*>(weapon), model);

        const auto entitySystem = hookContext.template make<EntitySystem>();
        const auto modelHandle = weaponModelHandle(weapon);
        auto weaponModel = static_cast<cs2::C_BaseModelEntity*>(entitySystem.getEntityFromHandle(modelHandle));
        if (weaponModel && weaponModel != weapon)
            changeModel(weaponModel, model);

        const auto attachmentHandle = viewmodelAttachmentHandle(weapon);
        auto attachment = static_cast<cs2::C_BaseModelEntity*>(entitySystem.getEntityFromHandle(attachmentHandle));
        if (attachment && attachment != weapon && attachment != weaponModel)
            changeModel(attachment, model);
        return true;
#else
        (void)weapon;
        (void)vData;
        return false;
#endif
    }

    [[nodiscard]] static std::uint32_t subclassIdForName(const char* name) noexcept
    {
        constexpr std::uint32_t kMultiplier = 0x5BD1E995;
        std::size_t length = 0;
        while (name[length] != '\0')
            ++length;
        auto hash = std::uint32_t{0x31415926} ^ static_cast<std::uint32_t>(length);
        auto data = reinterpret_cast<const std::uint8_t*>(name);

        while (length >= 4) {
            auto value = static_cast<std::uint32_t>(data[0])
                | (static_cast<std::uint32_t>(data[1]) << 8)
                | (static_cast<std::uint32_t>(data[2]) << 16)
                | (static_cast<std::uint32_t>(data[3]) << 24);
            value *= kMultiplier;
            value ^= value >> 24;
            value *= kMultiplier;
            hash *= kMultiplier;
            hash ^= value;
            data += 4;
            length -= 4;
        }

        if (length == 3)
            hash ^= static_cast<std::uint32_t>(data[2]) << 16;
        if (length >= 2)
            hash ^= static_cast<std::uint32_t>(data[1]) << 8;
        if (length >= 1) {
            hash ^= data[0];
            hash *= kMultiplier;
        }
        hash ^= hash >> 13;
        hash *= kMultiplier;
        hash ^= hash >> 15;
        return hash;
    }

    [[nodiscard]] std::uint32_t subclassIdForItemDefinition(std::uint16_t itemDefinitionIndex) const noexcept
    {
        StringBuilderStorage<6> storage;
        return subclassIdForName(storage.builder().put(itemDefinitionIndex).cstring());
    }

    void updateWeaponSubclass(cs2::C_CSWeaponBase* weapon) const noexcept
    {
#if IS_WIN64()
        if (const auto updateSubclass = hookContext.patternSearchResults().template get<UpdateSubclassFunction>())
            updateSubclass(static_cast<cs2::C_BaseEntity*>(weapon));
#else
        (void)weapon;
#endif
    }

    void notifyWeaponSubclassChanged(cs2::C_CSWeaponBase* weapon) const noexcept
    {
#if IS_WIN64()
        if (!weapon)
            return;
        using NotifySubclassChanged = void(*)(cs2::C_CSWeaponBase*, int);
        constexpr std::size_t kNotifySubclassChangedVmtIndex = 0x628 / sizeof(void*);
        const auto vmt = *reinterpret_cast<NotifySubclassChanged**>(weapon);
        if (vmt && vmt[kNotifySubclassChangedVmtIndex])
            vmt[kNotifySubclassChangedVmtIndex](weapon, 1);
#else
        (void)weapon;
#endif
    }

    [[nodiscard]] cs2::CCSWeaponBaseVData* findWeaponVData(std::uint16_t itemDefinitionIndex) const noexcept
    {
#if IS_WIN64()
        const auto findVData = hookContext.patternSearchResults().template get<FindWeaponVDataFunction>();
        if (!findVData)
            return nullptr;
        StringBuilderStorage<6> storage;
        return findVData(1, storage.builder().put(itemDefinitionIndex).cstring());
#else
        (void)itemDefinitionIndex;
        return nullptr;
#endif
    }

    [[nodiscard]] static constexpr std::uint64_t fauxItemId(std::uint16_t itemDefinitionIndex, std::uint16_t paintKitIndex) noexcept
    {
        return cs2::kFauxItemIdMask | itemDefinitionIndex
            | (static_cast<std::uint64_t>(paintKitIndex) << cs2::kFauxItemIdPaintKitIndexShift);
    }

    void rebuildFirstPersonWeapon(cs2::C_CSWeaponBase* weapon) const noexcept
    {
#if IS_WIN64()
        if (const auto onEconDataChanged = hookContext.patternSearchResults().template get<EconEntityOnDataChangedFunction>())
            onEconDataChanged(weapon, 1);
        if (const auto onDataChanged = hookContext.patternSearchResults().template get<WeaponOnDataChangedFunction>())
            onDataChanged(weapon, 1);
#else
        (void)weapon;
#endif
    }

    void setWeaponPaintAttributes(cs2::C_EconItemView* itemView, std::int32_t paintKit,
        std::int32_t seed, float wear) const noexcept
    {
#if IS_WIN64()
        const auto setAttribute = hookContext.patternSearchResults().template get<SetEconItemAttributeByNameFunction>();
        if (!setAttribute)
            return;

        setAttribute(itemView, "set item texture prefab", static_cast<float>(paintKit));
        setAttribute(itemView, "set item texture seed", static_cast<float>(seed));
        setAttribute(itemView, "set item texture wear", wear);
#else
        (void)itemView;
        (void)paintKit;
        (void)seed;
        (void)wear;
#endif
    }

    void initializeWeaponEconAttributes(cs2::C_CSWeaponBase* weapon) const noexcept
    {
#if IS_WIN64()
        if (const auto initializeAttributes = hookContext.patternSearchResults().template get<InitializeWeaponEconAttributesFunction>())
            initializeAttributes(weapon);
#else
        (void)weapon;
#endif
    }

    void applyWeapon(auto&& weaponEntity) const noexcept
    {
        const auto entityType = weaponEntity.classify();
        const bool knife = entityType.template is<cs2::C_Knife>();
        if (!knife && !isGun(entityType))
            return;

        const auto targetId = static_cast<std::uint16_t>(knife
            ? GET_CONFIG_VAR(equipment_model_changer_vars::KnifeModelId)
            : GET_CONFIG_VAR(equipment_model_changer_vars::GunModelId));
        const auto targetPaintKit = static_cast<std::uint16_t>(knife
            ? GET_CONFIG_VAR(equipment_model_changer_vars::KnifePaintKitId)
            : GET_CONFIG_VAR(equipment_model_changer_vars::GunPaintKitId));

        const auto handle = weaponEntity.handle();
        auto record = findChangedWeapon(handle);
        if (record)
            record->seen = true;

        auto weapon = static_cast<cs2::C_CSWeaponBase*>(static_cast<cs2::C_BaseEntity*>(weaponEntity));
        auto itemView = weaponItemView(weapon);
        auto definitionIndex = itemDefinitionIndex(itemView);
        auto currentItemId = itemId(itemView);
        auto currentItemIdHigh = itemIdHigh(itemView);
        auto currentItemIdLow = itemIdLow(itemView);
        auto currentEntityQuality = entityQuality(itemView);
        auto currentFallbackPaintKit = fallbackPaintKit(weapon);
        auto currentFallbackSeed = fallbackSeed(weapon);
        auto currentFallbackWear = fallbackWear(weapon);
        auto currentVData = weaponVData(weapon);
        auto currentSubclassId = weaponSubclassId(weapon);
        if (!definitionIndex || !currentItemId || !currentItemIdHigh || !currentItemIdLow
            || !currentEntityQuality || !currentFallbackPaintKit || !currentFallbackSeed
            || !currentFallbackWear || !currentVData || !currentSubclassId)
            return;

        if (targetId == 0) {
            if (record)
                restoreWeapon(*record);
            return;
        }

        bool newlyTracked = false;
        if (!record) {
            record = allocateChangedWeapon(handle, *definitionIndex, *currentItemId,
                *currentItemIdHigh, *currentItemIdLow, *currentEntityQuality,
                *currentFallbackPaintKit, *currentFallbackSeed, *currentFallbackWear,
                *currentSubclassId, *currentVData, knife);
            if (!record)
                return;
            newlyTracked = true;
        }

        const auto previousId = *definitionIndex;
        const auto previousItemId = *currentItemId;
        const auto previousItemIdHigh = *currentItemIdHigh;
        const auto previousItemIdLow = *currentItemIdLow;
        const auto previousFallbackPaintKit = *currentFallbackPaintKit;
        const auto previousFallbackSeed = *currentFallbackSeed;
        const auto previousFallbackWear = *currentFallbackWear;
        const auto targetSubclassId = subclassIdForItemDefinition(targetId);

        constexpr auto kFactoryNewWear = 0.0001f;
        const auto targetItemId = fauxItemId(targetId, targetPaintKit);
        const bool econChanged = previousId != targetId
            || previousItemId != targetItemId
            || previousItemIdHigh != ~std::uint32_t{}
            || previousItemIdLow != 0
            || previousFallbackPaintKit != targetPaintKit
            || previousFallbackSeed != 0
            || previousFallbackWear != kFactoryNewWear
            || *currentSubclassId != targetSubclassId;

        *definitionIndex = targetId;
        *currentItemId = targetItemId;
        *currentItemIdHigh = ~std::uint32_t{};
        *currentItemIdLow = 0;
        if (knife)
            *currentEntityQuality = 3;
        *currentFallbackPaintKit = targetPaintKit;
        *currentFallbackSeed = 0;
        *currentFallbackWear = kFactoryNewWear;
        const bool subclassChanged = *currentSubclassId != targetSubclassId;
        *currentSubclassId = targetSubclassId;
        markItemViewChanged(itemView);
        if (econChanged) {
            setWeaponPaintAttributes(itemView, targetPaintKit, 0, kFactoryNewWear);
            invalidateWeaponVisuals(weapon);
        }

        const auto targetVData = findWeaponVData(targetId);
#if IS_WIN64()
        if (subclassChanged) {
            updateWeaponSubclass(weapon);
            notifyWeaponSubclassChanged(weapon);
        }
        const bool modelChanged = targetVData && *currentVData != targetVData;
        if (targetVData)
            *currentVData = targetVData;
        if (modelChanged || newlyTracked || econChanged || subclassChanged) {
            record->attachmentRefreshFrames = 8;
        }

        const auto modelHandle = weaponModelHandle(weapon);
        const auto attachmentHandle = viewmodelAttachmentHandle(weapon);
        const bool renderEntityChanged = modelHandle != record->weaponModelHandle
            || attachmentHandle != record->viewmodelAttachmentHandle;
        if (renderEntityChanged)
            record->attachmentRefreshFrames = 8;
        if (targetVData && (renderEntityChanged || record->attachmentRefreshFrames > 0)) {
            if (changeWeaponRenderModels(weapon, targetVData)) {
                record->weaponModelHandle = modelHandle;
                record->viewmodelAttachmentHandle = attachmentHandle;
                invalidateWeaponVisuals(weapon);
                rebuildFirstPersonWeapon(weapon);
                if (record->attachmentRefreshFrames > 0)
                    --record->attachmentRefreshFrames;
            }
        }
#else
        (void)targetVData;
#endif
    }

    [[nodiscard]] static bool isGun(EntityTypeInfo type) noexcept
    {
        switch (type.typeIndex) {
        case EntityTypeInfo::indexOf<cs2::C_Flashbang>():
        case EntityTypeInfo::indexOf<cs2::C_HEGrenade>():
        case EntityTypeInfo::indexOf<cs2::C_SmokeGrenade>():
        case EntityTypeInfo::indexOf<cs2::C_MolotovGrenade>():
        case EntityTypeInfo::indexOf<cs2::C_IncendiaryGrenade>():
        case EntityTypeInfo::indexOf<cs2::C_DecoyGrenade>():
        case EntityTypeInfo::indexOf<cs2::C_C4>():
        case EntityTypeInfo::indexOf<cs2::C_Item_Healthshot>(): return false;
        default: return type.isWeapon();
        }
    }

    void applyGloves(auto&& localPawn) const noexcept
    {
        const auto targetId = static_cast<std::uint16_t>(GET_CONFIG_VAR(equipment_model_changer_vars::GloveModelId));
        const auto targetPaintKit = static_cast<std::uint16_t>(GET_CONFIG_VAR(equipment_model_changer_vars::GlovePaintKitId));
        const auto pawnHandle = localPawn.baseEntity().handle();
        if (state().changedGloves.active && state().changedGloves.pawnHandle != pawnHandle)
            restoreGloves();

        auto pawn = static_cast<cs2::C_CSPlayerPawn*>(static_cast<cs2::C_BaseEntity*>(localPawn.baseEntity()));
        auto itemView = gloveItemView(pawn);
        auto definitionIndex = itemDefinitionIndex(itemView);
        auto currentItemId = itemId(itemView);
        auto currentItemIdHigh = itemIdHigh(itemView);
        auto currentItemIdLow = itemIdLow(itemView);
        auto currentEntityQuality = entityQuality(itemView);
        if (!definitionIndex || !currentItemId || !currentItemIdHigh || !currentItemIdLow || !currentEntityQuality)
            return;

        if (targetId == 0) {
            restoreGloves();
            return;
        }

        if (!state().changedGloves.active) {
            state().changedGloves = {
                .pawnHandle = pawnHandle,
                .originalItemDefinitionIndex = *definitionIndex,
                .originalItemId = *currentItemId,
                .originalItemIdHigh = *currentItemIdHigh,
                .originalItemIdLow = *currentItemIdLow,
                .originalEntityQuality = *currentEntityQuality,
                .active = true
            };
        }

        *definitionIndex = targetId;
        *currentItemId = fauxItemId(targetId, targetPaintKit);
        *currentItemIdHigh = ~std::uint32_t{};
        *currentItemIdLow = 0;
        *currentEntityQuality = 4;
        markItemViewChanged(itemView);
        markGlovesChanged(pawn);
    }

    [[nodiscard]] cs2::C_EconItemView* gloveItemView(cs2::C_CSPlayerPawn* pawn) const noexcept
    {
        if (!state().offsets.gloveOffsetsValid())
            return nullptr;
        return field<cs2::C_EconItemView>(pawn, state().offsets.econGloves);
    }

    void markItemViewChanged(cs2::C_EconItemView* itemView) const noexcept
    {
        if (auto initialized = field<bool>(itemView, state().offsets.itemInitialized))
            *initialized = true;
        if (auto disallowSoc = field<bool>(itemView, state().offsets.itemDisallowSoc))
            *disallowSoc = false;
    }

    void markGlovesChanged(cs2::C_CSPlayerPawn* pawn) const noexcept
    {
        if (auto reapply = field<bool>(pawn, state().offsets.needToReapplyGloves))
            *reapply = true;
        if (auto changed = field<std::uint8_t>(pawn, state().offsets.econGlovesChanged))
            ++*changed;
    }

    void resetSeenFlags() const noexcept
    {
        for (auto& record : state().changedWeapons)
            record.seen = false;
    }

    [[nodiscard]] ChangedWeaponModel* findChangedWeapon(cs2::CEntityHandle handle) const noexcept
    {
        for (auto& record : state().changedWeapons) {
            if (record.active && record.handle == handle)
                return &record;
        }
        return nullptr;
    }

    [[nodiscard]] ChangedWeaponModel* allocateChangedWeapon(cs2::CEntityHandle handle, std::uint16_t originalId,
        std::uint64_t originalItemId, std::uint32_t originalItemIdHigh,
        std::uint32_t originalItemIdLow, std::int32_t originalEntityQuality,
        std::int32_t originalFallbackPaintKit, std::int32_t originalFallbackSeed,
        float originalFallbackWear,
        std::uint32_t originalSubclassId,
        cs2::CEntitySubclassVDataBase* originalVData, bool knife) const noexcept
    {
        for (auto& record : state().changedWeapons) {
            if (!record.active) {
                record = {
                    .handle = handle,
                    .originalItemDefinitionIndex = originalId,
                    .originalItemId = originalItemId,
                    .originalItemIdHigh = originalItemIdHigh,
                    .originalItemIdLow = originalItemIdLow,
                    .originalEntityQuality = originalEntityQuality,
                    .originalFallbackPaintKit = originalFallbackPaintKit,
                    .originalFallbackSeed = originalFallbackSeed,
                    .originalFallbackWear = originalFallbackWear,
                    .originalSubclassId = originalSubclassId,
                    .originalVData = originalVData,
                    .knife = knife,
                    .seen = true,
                    .active = true
                };
                return &record;
            }
        }
        return nullptr;
    }

    void restoreWeaponsNoLongerCarried() const noexcept
    {
        for (auto& record : state().changedWeapons) {
            if (record.active && !record.seen)
                restoreWeapon(record);
        }
    }

    void restoreWeapon(ChangedWeaponModel& record) const noexcept
    {
        auto weapon = static_cast<cs2::C_CSWeaponBase*>(hookContext.template make<EntitySystem>().getEntityFromHandle(record.handle));
        if (auto itemView = weaponItemView(weapon)) {
            if (auto definitionIndex = itemDefinitionIndex(itemView)) {
                *definitionIndex = record.originalItemDefinitionIndex;
                if (auto currentItemId = itemId(itemView))
                    *currentItemId = record.originalItemId;
                if (auto currentItemIdHigh = itemIdHigh(itemView))
                    *currentItemIdHigh = record.originalItemIdHigh;
                if (auto currentItemIdLow = itemIdLow(itemView))
                    *currentItemIdLow = record.originalItemIdLow;
                if (auto currentEntityQuality = entityQuality(itemView))
                    *currentEntityQuality = record.originalEntityQuality;
                if (auto currentFallbackPaintKit = fallbackPaintKit(weapon))
                    *currentFallbackPaintKit = record.originalFallbackPaintKit;
                if (auto currentFallbackSeed = fallbackSeed(weapon))
                    *currentFallbackSeed = record.originalFallbackSeed;
                if (auto currentFallbackWear = fallbackWear(weapon))
                    *currentFallbackWear = record.originalFallbackWear;
                if (auto currentSubclassId = weaponSubclassId(weapon)) {
                    *currentSubclassId = record.originalSubclassId;
                    updateWeaponSubclass(weapon);
                    notifyWeaponSubclassChanged(weapon);
                }
                if (auto currentVData = weaponVData(weapon)) {
                    *currentVData = record.originalVData;
                }
                markItemViewChanged(itemView);
                setWeaponPaintAttributes(itemView, record.originalFallbackPaintKit,
                    record.originalFallbackSeed, record.originalFallbackWear);
                invalidateWeaponVisuals(weapon);
                (void)changeWeaponRenderModels(weapon, record.originalVData);
                rebuildFirstPersonWeapon(weapon);
            }
        }
        record = {};
    }

    void restoreGloves() const noexcept
    {
        auto& record = state().changedGloves;
        if (!record.active)
            return;
        auto pawn = static_cast<cs2::C_CSPlayerPawn*>(hookContext.template make<EntitySystem>().getEntityFromHandle(record.pawnHandle));
        if (auto itemView = gloveItemView(pawn)) {
            if (auto definitionIndex = itemDefinitionIndex(itemView)) {
                *definitionIndex = record.originalItemDefinitionIndex;
                if (auto currentItemId = itemId(itemView))
                    *currentItemId = record.originalItemId;
                if (auto currentItemIdHigh = itemIdHigh(itemView))
                    *currentItemIdHigh = record.originalItemIdHigh;
                if (auto currentItemIdLow = itemIdLow(itemView))
                    *currentItemIdLow = record.originalItemIdLow;
                if (auto currentEntityQuality = entityQuality(itemView))
                    *currentEntityQuality = record.originalEntityQuality;
                markItemViewChanged(itemView);
                markGlovesChanged(pawn);
            }
        }
        record = {};
    }

    void restoreAll() const noexcept
    {
        for (auto& record : state().changedWeapons) {
            if (record.active)
                restoreWeapon(record);
        }
        restoreGloves();
    }

    [[nodiscard]] auto& state() const noexcept
    {
        return hookContext.featuresStates().visualFeaturesStates.equipmentModelChangerState;
    }

    HookContext& hookContext;
};

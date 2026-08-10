#pragma once

#include <MemoryPatterns/PatternTypes/WeaponVDataPatternTypes.h>
#include <MemorySearch/CodePattern.h>

struct WeaponVDataPatterns {
    [[nodiscard]] static consteval auto addClientPatterns(auto clientPatterns) noexcept
    {
        return clientPatterns
            .template addPattern<OffsetToWeaponName, CodePattern{"? ? ? ? 48 85 C9 48 0F 45 D9 BA"}.read()>()
            .template addPattern<FindWeaponVDataFunction, CodePattern{"48 89 5C 24 ? 57 48 83 EC ? 33 FF 4C 8B CA 8B D9 8B C7 48 85 D2 74 ? 38 02"}>()
            .template addPattern<WeaponOnDataChangedFunction, CodePattern{"40 55 56 48 83 EC ? 8B EA 48 8B F1 83 FA 01 0F 85 ? ? ? ? 44 8B 81 ? ? ? ? 41 83 F8 FF"}>()
            .template addPattern<EconEntityOnDataChangedFunction, CodePattern{"48 89 5C 24 08 57 48 83 EC 20 8B DA 48 8B F9 E8 ? ? ? ? F6 C3 01 74 ? 48 8B CF E8 ? ? ? ? C6 87 91 16 00 00 00"}>()
            .template addPattern<SetEconItemAttributeByNameFunction, CodePattern{"40 53 48 83 EC 20 48 8B D9 48 81 C1 08 02 00 00 E8 ? ? ? ? 48 8B CB 48 83 C4 20 5B E9"}>()
            .template addPattern<InitializeWeaponEconAttributesFunction, CodePattern{"48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 30 48 8B D9 48 81 C1 A8 11 00 00"}>();
    }
};

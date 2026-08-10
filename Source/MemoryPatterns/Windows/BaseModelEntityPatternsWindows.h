#pragma once

#include <MemoryPatterns/PatternTypes/BaseModelEntityPatternTypes.h>
#include <MemorySearch/CodePattern.h>

struct BaseModelEntityPatterns {
    [[nodiscard]] static consteval auto addClientPatterns(auto clientPatterns) noexcept
    {
        return clientPatterns
            .template addPattern<OffsetToGlowProperty, CodePattern{"90 ? ? ? ? ? 8B ? 48 8D 88 ? ? ? ?"}.add(11).read()>()
            .template addPattern<ChangeModelFunction, CodePattern{"40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24"}>();
    }
};

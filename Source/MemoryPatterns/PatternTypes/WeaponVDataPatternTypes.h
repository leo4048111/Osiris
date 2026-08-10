#pragma once

#include <cstdint>

#include <CS2/Classes/CCSWeaponBaseVData.h>
#include <CS2/Classes/CEconItemView.h>
#include <CS2/Classes/Entities/C_CSWeaponBase.h>
#include <Utils/FieldOffset.h>
#include <Utils/StrongTypeAlias.h>

template <typename FieldType, typename OffsetType>
using WeaponVDataOffset = FieldOffset<cs2::CCSWeaponBaseVData, FieldType, OffsetType>;

STRONG_TYPE_ALIAS(OffsetToWeaponName, WeaponVDataOffset<cs2::CCSWeaponBaseVData::m_szName, std::int32_t>);
STRONG_TYPE_ALIAS(FindWeaponVDataFunction, cs2::CCSWeaponBaseVData*(*)(int type, const char* name));
STRONG_TYPE_ALIAS(WeaponOnDataChangedFunction, void(*)(cs2::C_CSWeaponBase*, int updateType));
STRONG_TYPE_ALIAS(EconEntityOnDataChangedFunction, void(*)(cs2::C_CSWeaponBase*, int updateType));
STRONG_TYPE_ALIAS(SetEconItemAttributeByNameFunction, void(*)(cs2::C_EconItemView*, const char* attributeName, float value));
STRONG_TYPE_ALIAS(InitializeWeaponEconAttributesFunction, void(*)(cs2::C_CSWeaponBase*));

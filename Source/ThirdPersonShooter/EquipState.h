#pragma once

UENUM(BlueprintType)
enum class EEquipState : uint8
{
	EES_Unequipped UMETA(DisplayName = "Unequipped"),
	EES_Equipped UMETA(DisplayName = "Equipped"),

	EES_MAX UMETA(DisplayName = "DefaultMax")
};
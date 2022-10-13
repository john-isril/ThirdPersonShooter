#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};
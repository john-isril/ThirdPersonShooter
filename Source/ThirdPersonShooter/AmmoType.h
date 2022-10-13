#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_SHOTGUN UMETA(DisplayName = "Shotgun"),
	EAT_AR UMETA(DisplayName = "AssaultRifle"),
	EAT_PISTOL UMETA(DisplayName = "Pistol"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};
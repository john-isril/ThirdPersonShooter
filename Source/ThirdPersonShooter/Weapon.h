// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "WeaponType.h"
#include "Engine/DataTable.h"
#include "Weapon.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HipfireMontageSectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite);
	FName BarrelSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsMiddle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAutomatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite);
	bool bIsAPrimaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadshotDamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite);
	UParticleSystem* ImpactParticles;
};

UCLASS()
class THIRDPERSONSHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"));
	UDataTable* WeaponDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"));
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"));
	bool bIsAutomatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"));
	bool bIsAPrimaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"));
	float AutoFireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"));
	int32 Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"));
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"));
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"));
	float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"));
	float HeadshotDamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"));
	FName ReloadMontageSectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"));
	FName HipfireMontageSectionName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"));
	bool bMovingClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"));
	FName ClipBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"));
	FName BarrelSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"));
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"));
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"));
	class UAnimMontage* FireMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsMiddle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsBottom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"));
	USoundCue* FireSound;

	int32 PreviousMaterialIndex;
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponDuration {0.7f};
	float ThrowWeaponImpulseStrength{ 200000.0f };
	bool bIsFalling;

public:
	AWeapon();

	void PlayFireAnimation();
	void ThrowWeapon();
	void IncrementAmmo();
	void DecrementAmmo();
	void ReloadAmmo(int32 Amount);
	bool MagazineIsFull() const;
	
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE class UAnimMontage* GetFireMontage() const { return FireMontage; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }
	FORCEINLINE FName GetReloadMontageSectionName() const { return ReloadMontageSectionName; }
	FORCEINLINE FName GetHipfireMontageSectionName() const { return HipfireMontageSectionName; }
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE FName GetBarrelSocketName() const { return BarrelSocketName; }
	int32 GetClipBoneIndex() const;
	FTransform GetClipBoneTransform();
	FORCEINLINE UTexture2D* GetCrosshairMiddleTexture() const { return CrosshairsMiddle; }
	FORCEINLINE UTexture2D* GetCrosshairTopTexture() const { return CrosshairsTop; }
	FORCEINLINE UTexture2D* GetCrosshairLeftTexture() const { return CrosshairsLeft; }
	FORCEINLINE UTexture2D* GetCrosshairRightTexture() const { return CrosshairsRight; }
	FORCEINLINE UTexture2D* GetCrosshairBottomTexture() const { return CrosshairsBottom; }
	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }
	FORCEINLINE class UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; };
	FORCEINLINE UParticleSystem* GetImpactParticles() const { return ImpactParticles; };
	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; };
	FORCEINLINE float GetHeadshotDamageAmount() const { return HeadshotDamageAmount; }
	FORCEINLINE float GetDamageAmount() const { return DamageAmount; }
	FORCEINLINE bool GetIsAutomatic()  { return bIsAutomatic; }
	FORCEINLINE bool GetIsAPrimaryWeapon() { return bIsAPrimaryWeapon; }

	FORCEINLINE void SetFireSound(USoundCue* Sound) { FireSound = Sound; }
	FORCEINLINE void SetMuzzleFlash(UParticleSystem* MuzzleFlash_) { MuzzleFlash = MuzzleFlash_; }
	FORCEINLINE void SetImpactParticles(UParticleSystem* Particles) { ImpactParticles = Particles; }
	FORCEINLINE void SetFireMontage(UAnimMontage* Montage) { FireMontage = Montage; }
	FORCEINLINE void SetIsAutomatic(bool is_automatic) { bIsAutomatic = is_automatic; }
	FORCEINLINE void SetIsAPrimaryWeapon(bool is_primary) { bIsAPrimaryWeapon = is_primary; }
	FORCEINLINE void SetDamageAmount(float Amnt) { DamageAmount = Amnt; }
	FORCEINLINE void SetHeadshotDamageAmount(float Amnt) { HeadshotDamageAmount = Amnt; }
	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }
	FORCEINLINE void SetClipBoneName(FName Name) { ClipBoneName = Name; }
	FORCEINLINE void SetBarrelSocketName(FName Name) { BarrelSocketName = Name; }
	FORCEINLINE void SetReloadMontageSectionName(FName Name) { ReloadMontageSectionName = Name; }
	FORCEINLINE void SetHipfireMontageSectionName(FName Name) { HipfireMontageSectionName = Name; }
	FORCEINLINE void SetCrosshairMiddleTexture(UTexture2D* Texture) { CrosshairsMiddle = Texture; }
	FORCEINLINE void SetCrosshairTopTexture(UTexture2D* Texture) { CrosshairsTop = Texture; }
	FORCEINLINE void SetCrosshairBottomTexture(UTexture2D* Texture) { CrosshairsBottom = Texture; }
	FORCEINLINE void SetCrosshairLeftTexture(UTexture2D* Texture) { CrosshairsLeft = Texture; }
	FORCEINLINE void SetCrosshairRightTexture(UTexture2D* Texture) { CrosshairsRight = Texture; }
	FORCEINLINE void SetAutoFireRate(float Rate) { AutoFireRate = Rate; };

protected:
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	void StopFalling();
};

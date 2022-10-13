// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "EquipState.h"
#include "MainCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipDelegate, bool, bShouldPlayEquipAnimation);

UCLASS()
class THIRDPERSONSHOOTER_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "3.0", UIMin = "0.0", UIMAX = "3.0"), Category = Combat)
	float MouseHipSensitivityScale {1.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "3.0", UIMin = "0.0", UIMAX = "3.0"), Category = Combat)
	float MouseAimSensitivityScale {0.6f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CombatCollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float DefaultFOV {90.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	FVector DefaultCameraOffset { 0.0f, 100.0f, 55.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimFOV{ 65.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimInterpSpeed{ 15.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float DeathInterpSpeed{ 5.f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	FVector CurrentCameraOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float CurrentFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float CapsuleHalfHeightInterpSpeed{ 5.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float CurrentCapsuleHalfHeight {95.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float StandingCapsuleHalfHeight{ 95.0f };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Camera)
	float CrouchingCapsuleHalfHeight{ 47.5f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float WalkSpeed{ 300.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchSpeed{ 300.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AimSpeed{ 300.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float SprintSpeed{ 600.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Movement)
	float RunningGroundFriction {2.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Equip)
	float CrouchingGroundFriction{ 100.0f };


	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	class AWeapon* EquippedWeapon;

	// tsub classof holds a reference to a blueprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	class UAnimMontage* HipFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	UAnimMontage* EquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Explosion)
	UAnimMontage* ExplosionReactMontage;

	// how far the linetrace can go, ie how far a bullet can go
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	float LineTraceDistance{ 50000.0f };

	// attatch the scene component to hand when reloading. when reloading clip
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Weapon);
	USceneComponent* HandSceneComponent;

	UPROPERTY(eDITAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon);
	FName HandBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	int32 StartingPistolAmmo {30};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	int32 StartingARAmmo {90};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	int32 StartingShotgunAmmo { 14 };

	// when reloading clip
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Weapon);
	FTransform ClipTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = HUD)
	float CrosshairSpreadMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = HUD)
	float CrosshairVelocityFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = HUD)
	float CrosshairInAirFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = HUD)
	float CrosshairAimFactor;

	// a y axis offset from the middle of the screen (crosshairs a little lower)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = HUD)
	float CrosshairYOffset{ 50.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = HUD);
	bool bCanShowHitMarker; // if enemy is shot at

	FTimerHandle FireTimer;
	FTimerHandle HitMarkerTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	TArray<FName> HitReactSectionNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	UAnimMontage* MeleeCombatMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	UAnimMontage* StrangleMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	TArray<FName> PrimaryAttackMontageSectionNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	TArray<FName> SecondaryAttackMontageSectionNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Inventory)
	UAnimMontage* PickupMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	float StealthModeInterpSpeed{ 0.0005f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	bool bIsInStealthMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	bool bIsCrouching;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	bool bIsInMeleeState;

	bool bMeleeAttackInProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	float MeleeDamage {25.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class AEnemy *StrangleVictim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	UParticleSystem* BloodParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class USoundCue* MeleeimpactSoundGrunt; // sound when player gets hit

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class USoundCue* MeleeimpactSoundHit; // sound when player gets hit
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class USoundCue* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	class USoundCue* JumpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	float MaxHealth {100.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	bool bIsDead;

	const int16 INVENTORY_CAPACITY{ 3 };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Inventory)
	TArray<class AItem*> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Inventory)
	TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	ECombatState CombatState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	bool bIsAiming;

	bool bIsFireButtonPressed;
	bool bShotsFired;

	AItem* OverlappedPickupItem;
	int32 CurrentWeaponIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Combat)
	bool bIsHitByExplosion;

	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"));
	FEquipDelegate EquipWidgetDelegate;

	float DefaultCameraArmInterpSpeed{ 5.0f };
	float MeleeCameraArmInterpSpeed{ 5.0f };
	float DeathCameraArmInterpSpeed{ 2.0f };
	float TargetCameraArmInterpSpeed;
	float DefaultCameraArmLength;
	float TargetCameraArmLength;
	float CurrentCameraArmLength;
	float MeleeCameraArmLength{ 300.0f };
	float DeathCameraArmLength{ 600.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon);
	USceneComponent* TargetStrangleSceneComponent;

	bool bStranglingInProgress;
	bool bIsPaused;

public:
	// Sets default values for this character's properties
	AMainCharacter();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// CAMERA
	void SetCameraZoomFov(float DeltaTime);
	void UpdateCapsuleHalfHeight(float DeltaTime);
	void UpdateCameraArmLength (float DeltaTime);

	// HUD
	void CalculateCrosshairSpreadMultiplier(float DeltaTime);
	void ResetHitMarker();

	// MOVEMENT
	void Turn(float Scale);
	void LookUp(float Scale);
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Sprint();
	void StopSprint();
	void UpdateSpeed();

	// WEAPON SYSTEM: Action
	void Aim();
	void AimReleased();
	void FireButtonPressed();
	void FireWeapon();
	void GetBulletHitLocation(const FTransform& SocketTransform, FHitResult& OutHitResult);
	void StartFireTimer();
	
	UFUNCTION()
	void AutoFireReset();
	
	void FireButtonReleased();
	bool CrosshairLineTrace(FHitResult& OutHitResult, FVector& CrosshairStart, FVector& CrosshairDirection);
	bool CrosshairLineTrace(FHitResult& OutHitResult);
	void ProjectBullet();
	void ReloadWeapon();
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	// Sound/Animation
	void PlayFireSound() const;
	void PlayGunFireAnimation() const;
	
	UFUNCTION(BlueprintCallable) // used for weapon clip reload animation. will be called through anim notifies.
	void GrabClip();

	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	// WEAPON SYSTEM: Equip/swap/inventory
	class AWeapon* SpawnDefaultWeapon();
	void EquipWeapon(AWeapon* Weapon);
	UFUNCTION(BlueprintCallable)
	void FinishEquipping();
	void DetachWeapon();
	void SwapWeapon(AWeapon* NewWeapon);
	void AttachWeaponToInventorySocket(AWeapon* Weapon);
	void PlayEquipMontage();
	void CycleWeaponsLeftButtonPressed();
	void CycleWeaponsRightButtonPressed();
	void SwitchToMeleeState();
	void SwitchToWeaponState();
	void ToggleMeleeState();
	void PickupAmmo(class AAmmo* Ammo);
	bool IsCarryingAmmo() const; // checks if the player has ammo for the equipped weapon
	void InitializeAmmoMap();
	bool WeaponHasAmmo();
	void InitializeHitReactNamesArray();

	// Stealth System
	UFUNCTION(BlueprintNativeEvent)
	void StealthButtonPressed();
	void StealthButtonPressed_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	void StealthButtonReleased();
	void StealthButtonReleased_Implementation();
	void CrouchButtonPressed();
	
	UFUNCTION(BlueprintCallable)
	void CrouchStandTransitioning();
	
	UFUNCTION(BlueprintCallable)
	void CrouchStandTransitionFinished();

	// DEATH
	void Die();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	virtual void Jump() override;
	UFUNCTION(BlueprintCallable)
	void Footstep();

	// Melee
	void InitializePrimaryAttackMontageSectionNames();
	void InitializeSecondaryAttackMontageSectionNames();
	void PlayPrimaryAttack();
	void PlaySecondaryAttack();
	
	UFUNCTION(BlueprintCallable)
	void EndMeleeAttack();

	UFUNCTION()
	void OnCombatCollisionBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// these activation/deactivations will be called in anim notifies
	UFUNCTION(BlueprintCallable)
	void ActivateCombatCollisionBox();

	UFUNCTION(BlueprintCallable)
	void DeactivateCombatCollisionBox();

	void PlayStrangleMontage();

	UFUNCTION(BlueprintNativeEvent)
	void StrangleEnemy();
	void StrangleEnemy_Implementation();

	UFUNCTION(BlueprintCallable)
	void FinishStrangle();

	void TogglePauseScreen();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetOverlappedPickupItem(AItem* Item_);
	void GetPickupItem(AItem* Item);
	void PlayHitReactMontage();
	void PlayExplosionReactMontage();
	void PlayPickupMontage();
	void PickupItem();
	void SetPlayerPaused(bool paused);
	void PlayMeleeHitSound();

	FORCEINLINE bool GetIsSprinting() const { return bIsSprinting; }
	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE USoundCue* GetMeleeImpactSoundHit() const { return MeleeimpactSoundHit; }
	FORCEINLINE USoundCue* GetMeleeImpactSoundGrunt() const { return MeleeimpactSoundGrunt; }
	FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }
	FORCEINLINE bool GetIsCrouching() const { return bIsCrouching; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCrosshairYOffset() const { return CrosshairYOffset; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCrosshairSpreadMultiplier() const { return CrosshairSpreadMultiplier; }
	FORCEINLINE bool GetIsInMeleeState() const { return bIsInMeleeState; }
	FORCEINLINE void SetStrangleVictim(AEnemy* Victim) { StrangleVictim = Victim; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE USceneComponent* GetHandSceneComponent() { return HandSceneComponent; };
	FORCEINLINE bool GetShotsFired() const { return bShotsFired; }
};

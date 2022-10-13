// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Ammo.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "BulletHitInterface.h"
#include "ThirdPersonShooter.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "DrawDebugHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Explosive.h"
#include "ThirdPersonShooterGameModeBase.h"
#include "MainCharacterPlayerController.h"

// Sets default values
AMainCharacter::AMainCharacter() :
	CurrentFOV{}, CurrentCameraOffset{DefaultCameraOffset},
	bIsSprinting{false}, CrosshairSpreadMultiplier{}, CrosshairInAirFactor{}, CrosshairVelocityFactor{}, CrosshairAimFactor{},
	bCanShowHitMarker{ false }, bIsInStealthMode{ false }, Health{ 100.0f }, bIsDead{ false }, bIsCrouching{ false }, bIsInMeleeState{ false }, bIsAiming{ false },
	bIsFireButtonPressed{false}, HandBoneName{TEXT("None")}, OverlappedPickupItem{nullptr}, CurrentWeaponIndex{0}, bIsHitByExplosion{false}, bMeleeAttackInProgress{false},
	StrangleVictim{nullptr}, bShotsFired{false}, bStranglingInProgress{false}, bIsPaused{false}
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->bUsePawnControlRotation = true;
	CameraArm->SocketOffset = DefaultCameraOffset;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.3f;

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));
	InitializeHitReactNamesArray();
	InitializePrimaryAttackMontageSectionNames();
	InitializeSecondaryAttackMontageSectionNames();

	CombatCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Combat Collision Box"));
	CombatCollisionBox->SetupAttachment(GetRootComponent());

	TargetStrangleSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Strangle Scene Component"));
}

float AMainCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float TempHealth{ Health - DamageAmount };

	if (TempHealth < 0.1)
	{
		Health = 0.0f;
		Die();

		AEnemyController* EnemyController{ Cast<AEnemyController>(EventInstigator) };

		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(FName{ TEXT("PlayerIsDead") }, true);
		}

		if (DeathSound)
		{
			UGameplayStatics::PlaySound2D(this, DeathSound);
		}
	}
	else
	{
		Health = TempHealth;

		if (Cast<AExplosive>(DamageCauser))
		{
			PlayExplosionReactMontage();
		}
	}

	if (bStranglingInProgress)
	{
		StrangleVictim->StopBeingStrangled();
		FinishStrangle();
	}

	return DamageAmount;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentFOV = DefaultFOV;
	DefaultCameraArmLength = CameraArm->TargetArmLength;
	TargetCameraArmLength = DefaultCameraArmLength;
	CurrentCameraArmLength = DefaultCameraArmLength;
	TargetCameraArmInterpSpeed = DefaultCameraArmInterpSpeed;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetInventoryIndex(0);
	InitializeAmmoMap();

	CombatCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AMainCharacter::OnCombatCollisionBoxOverlap);
	CombatCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // only trigger collision events when attacking
	CombatCollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetCameraZoomFov(DeltaTime);
	UpdateCapsuleHalfHeight(DeltaTime);
	UpdateCameraArmLength(DeltaTime);
	CalculateCrosshairSpreadMultiplier(DeltaTime);
	UpdateSpeed();

	CurrentCameraArmLength = CameraArm->TargetArmLength;
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//for mouse/KEYBOARD
	PlayerInputComponent->BindAxis("LookUp", this, &AMainCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AMainCharacter::Turn);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::StopSprint);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMainCharacter::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMainCharacter::AimReleased);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AMainCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AMainCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &AMainCharacter::PickupItem);
	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AMainCharacter::ReloadWeapon);
	PlayerInputComponent->BindAction("CrouchButton", IE_Pressed, this, &AMainCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("StealthButton", IE_Pressed, this, &AMainCharacter::StealthButtonPressed);
	PlayerInputComponent->BindAction("StealthButton", IE_Released, this, &AMainCharacter::StealthButtonReleased);
	PlayerInputComponent->BindAction("CycleWeaponsLeftButton", IE_Pressed, this, &AMainCharacter::CycleWeaponsLeftButtonPressed);
	PlayerInputComponent->BindAction("CycleWeaponsRightButton", IE_Pressed, this, &AMainCharacter::CycleWeaponsRightButtonPressed);
	PlayerInputComponent->BindAction("SwitchToWeaponState", IE_Pressed, this, &AMainCharacter::SwitchToWeaponState);
	PlayerInputComponent->BindAction("SwitchToMeleeState", IE_Pressed, this, &AMainCharacter::SwitchToMeleeState);
	PlayerInputComponent->BindAction("ToggleMeleeState", IE_Pressed, this, &AMainCharacter::ToggleMeleeState);
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AMainCharacter::TogglePauseScreen);
}

void AMainCharacter::Turn(float Scale)
{
	if (bIsPaused) return;

	if (bIsAiming)
	{
		AddControllerYawInput(Scale * MouseAimSensitivityScale);
	}
	else
	{
		AddControllerYawInput(Scale * MouseHipSensitivityScale);
	}
}

void AMainCharacter::LookUp(float Scale)
{
	if (bIsPaused) return;

	if (bIsAiming)
	{
		AddControllerPitchInput(Scale * MouseAimSensitivityScale);
	}
	else
	{
		AddControllerPitchInput(Scale * MouseHipSensitivityScale);
	}
}

void AMainCharacter::SetCameraZoomFov(float DeltaTime)
{
	float TargetFOV{};

	if (bIsAiming)
	{
		if (bIsInStealthMode)
		{
			TargetFOV = FMath::FInterpTo(CurrentFOV, AimFOV - 7.0f, DeltaTime, AimInterpSpeed);
		}
		else
		{
			TargetFOV = FMath::FInterpTo(CurrentFOV, AimFOV, DeltaTime, AimInterpSpeed);
		}
	}
	else
	{
		if (bIsInStealthMode)
		{
			TargetFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV - 7.0f, DeltaTime, AimInterpSpeed);
		}
		else
		{
			TargetFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, AimInterpSpeed);
		}
	}

	CurrentFOV = TargetFOV;

	GetCamera()->FieldOfView = CurrentFOV;
}

void AMainCharacter::MoveForward(float Value)
{
	if (bIsPaused) return;

	if (Controller && Value)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMainCharacter::MoveRight(float Value)
{
	if (bIsPaused) return;

	if (Controller && Value)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMainCharacter::Sprint()
{
	if (bIsPaused) return;

	bIsSprinting = true;
	bIsCrouching = false;

	GetCharacterMovement()->GroundFriction = 2.0f;
}

void AMainCharacter::StopSprint()
{
	if (bIsPaused) return;

	bIsSprinting = false;
	GetCharacterMovement()->GroundFriction = 12.0f;
}

void AMainCharacter::UpdateSpeed()
{
	if (bIsAiming)
	{
		GetCharacterMovement()->MaxWalkSpeed = AimSpeed;
	}
	else if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else if (bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void AMainCharacter::UpdateCapsuleHalfHeight(float DeltaTime)
{
	float LastCapsuleHalfHeight{ CurrentCapsuleHalfHeight };

	if (bIsCrouching)
	{
		CurrentCapsuleHalfHeight = FMath::FInterpTo(CurrentCapsuleHalfHeight, CrouchingCapsuleHalfHeight, DeltaTime, CapsuleHalfHeightInterpSpeed);
	}
	else
	{
		CurrentCapsuleHalfHeight = FMath::FInterpTo(CurrentCapsuleHalfHeight, StandingCapsuleHalfHeight, DeltaTime, CapsuleHalfHeightInterpSpeed);
	}

	GetCapsuleComponent()->SetCapsuleHalfHeight(CurrentCapsuleHalfHeight);
	// when we crouch, the capsule shrinks
	// when we crouch, we want the mesh to be higher, if were standing then the mesh should be lower
	float DeltaOffset{ CurrentCapsuleHalfHeight - LastCapsuleHalfHeight };
	GetMesh()->AddLocalOffset(FVector { 0.0f, 0.0f, -(DeltaOffset) });
}

void AMainCharacter::UpdateCameraArmLength(float DeltaTime)
{
	CameraArm->TargetArmLength = FMath::FInterpTo(CurrentCameraArmLength, TargetCameraArmLength, DeltaTime, TargetCameraArmInterpSpeed);
	CurrentCameraArmLength = CameraArm->TargetArmLength;
}
void AMainCharacter::CalculateCrosshairSpreadMultiplier(float DeltaTime)
{
	FVector2D VelocityMapping{ 0.0f, 600.0f };
	FVector2D MultiplierMapping{ 0.0f, 1.0f };
	FVector CurrentVelocity{ GetVelocity() };
	CurrentVelocity.Z = 0.0f;
	double VelocityMagnitude{ CurrentVelocity.Size() };

	double VelocityMultiplier{ FMath::GetMappedRangeValueClamped(std::move(VelocityMapping), std::move(MultiplierMapping), VelocityMagnitude) };

	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);
	}

	if (bIsAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.0f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, 30.0f);
	}

	CrosshairSpreadMultiplier = 0.5f + VelocityMultiplier + CrosshairInAirFactor - CrosshairAimFactor;
}

void AMainCharacter::ResetHitMarker()
{
	bCanShowHitMarker = false;
}

void AMainCharacter::Aim()
{
	if (bIsPaused) return;

	if (bIsInMeleeState)
	{
		if (bIsCrouching && StrangleVictim)
		{
			StrangleEnemy();
		}
		else if (!bMeleeAttackInProgress)
		{
			PlaySecondaryAttack();
			bMeleeAttackInProgress = true;
		}
	}
	else
	{
		bIsAiming = true;
	}
}

void AMainCharacter::AimReleased()
{
	if (bIsPaused || bIsInMeleeState) return;
	
	bIsAiming = false;

	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void AMainCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		const USkeletalMeshSocket* HandSocket{ GetMesh()->GetSocketByName(FName("RightHandSocket")) };

		if (HandSocket)
		{
			HandSocket->AttachActor(Weapon, GetMesh());
		}

		EquippedWeapon = Weapon;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
		EquippedWeapon->SetInventoryIndex(CurrentWeaponIndex);
	}
}

void AMainCharacter::FireButtonPressed()
{
	if (bIsPaused) return;

	if (bIsInMeleeState)
	{
		if (bIsCrouching && StrangleVictim )
		{
			StrangleEnemy();
		}
		else if (!bMeleeAttackInProgress)
		{
			PlayPrimaryAttack();
			bMeleeAttackInProgress = true;
		}
	}
	else
	{
		bIsFireButtonPressed = true;
		FireWeapon();
	}
}

void AMainCharacter::FireWeapon()
{
	if (!EquippedWeapon || (CombatState != ECombatState::ECS_Unoccupied))
	{
		return;
	}

	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	if (WeaponHasAmmo())
	{
		PlayFireSound();
		ProjectBullet();
		PlayGunFireAnimation();
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();
	}
}

void AMainCharacter::StartFireTimer()
{
	if (EquippedWeapon)
	{
		CombatState = ECombatState::ECS_FireTimerInProgress;
		bShotsFired = true;
		GetWorldTimerManager().SetTimer(FireTimer, this, &AMainCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
	}
}

void AMainCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if (bIsFireButtonPressed && EquippedWeapon->GetIsAutomatic())
		{
			bShotsFired = true;
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}
}

void AMainCharacter::FireButtonReleased()
{
	if (bIsPaused) return;

	bIsFireButtonPressed = false;
	bShotsFired = false;
}

bool AMainCharacter::CrosshairLineTrace(FHitResult& OutHitResult)
{
	FVector CrosshairStart{};
	FVector CrosshairDirection{};

	return this->CrosshairLineTrace(OutHitResult, CrosshairStart, CrosshairDirection);
}

bool AMainCharacter::CrosshairLineTrace(FHitResult& OutHitResult, FVector& CrosshairStart, FVector& CrosshairDirection)
{
	bool bCrosshairToScreenSuccess{ false };

	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize{};
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		FVector2D CrosshairLocation{ ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f - CrosshairYOffset };
		// gets the crosshair world position and direction
		bCrosshairToScreenSuccess = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairStart, CrosshairDirection);

		if (bCrosshairToScreenSuccess)
		{
			FVector CrosshairLineTraceEnd{ CrosshairStart + CrosshairDirection * LineTraceDistance };
			// save whatever was hit by the linetrace in outhitresult
			GetWorld()->LineTraceSingleByChannel(OutHitResult, CrosshairStart, CrosshairLineTraceEnd, ECollisionChannel::ECC_Visibility);

			return true;
		}
	}

	return false;
}

void AMainCharacter::GetBulletHitLocation(const FTransform& SocketTransform, FHitResult& OutHitResult)
{
	FVector CrosshairStart{};
	FVector CrosshairDirection{};
	FVector HitLocation{};
	FHitResult CrossairHit{};

	FVector WeaponStart = SocketTransform.GetLocation();
	FVector WeaponDirection = SocketTransform.GetRotation().GetAxisX();
	FVector WeaponLineTraceEnd = WeaponStart + WeaponDirection * LineTraceDistance;
	FHitResult WeaponHit{};

	bool successful_crosshair_trace{ CrosshairLineTrace(OutHitResult, CrosshairStart, CrosshairDirection) };

	if (successful_crosshair_trace)
	{
		if (!OutHitResult.bBlockingHit)
		{
			OutHitResult.Location = WeaponLineTraceEnd;
		}
	}

	GetWorld()->LineTraceSingleByChannel(WeaponHit, WeaponStart, OutHitResult.Location, ECollisionChannel::ECC_Visibility);
	
	if (WeaponHit.bBlockingHit)
	{
		OutHitResult = WeaponHit;
	}
}

void AMainCharacter::ProjectBullet()
{
	const USkeletalMeshSocket* BarrelSocket{ EquippedWeapon->GetMesh()->GetSocketByName("BarrelSocket") };
	if (BarrelSocket)
	{
		const FTransform SocketTransform{ BarrelSocket->GetSocketTransform(EquippedWeapon->GetMesh()) };

		FHitResult FinalHitResult{};
		GetBulletHitLocation(SocketTransform, FinalHitResult);
		
		if (EquippedWeapon->GetImpactParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetImpactParticles(), FinalHitResult.Location);
		}

		if (FinalHitResult.GetActor())
		{
			IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(FinalHitResult.GetActor());
			AEnemy* HitEnemy{ Cast<AEnemy>(FinalHitResult.GetActor()) };

			if (BulletHitInterface)
			{
				BulletHitInterface->BulletHit_Implementation(FinalHitResult, this, GetController());
			}

			if (HitEnemy)
			{
				if (FinalHitResult.BoneName.ToString() == HitEnemy->GetHeadBoneName())
				{
					UGameplayStatics::ApplyDamage(FinalHitResult.GetActor(), EquippedWeapon->GetHeadshotDamageAmount(), GetController(), this, UDamageType::StaticClass());
				}
				else
				{
					UGameplayStatics::ApplyDamage(FinalHitResult.GetActor(), EquippedWeapon->GetDamageAmount(), GetController(), this, UDamageType::StaticClass());
				}
				bCanShowHitMarker = true;
				GetWorldTimerManager().SetTimer(HitMarkerTimer, this, &AMainCharacter::ResetHitMarker, 0.1f);
			}

		}
	}
}

void AMainCharacter::ReloadWeapon()
{
	if (bIsPaused || !EquippedWeapon || (CombatState != ECombatState::ECS_Unoccupied) || bIsInMeleeState) return;

	if (IsCarryingAmmo() && !EquippedWeapon->MagazineIsFull())
	{
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

		if (ReloadMontage && AnimInstance)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSectionName());
		}
	}
}

void AMainCharacter::FinishReloading()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (!EquippedWeapon) return;

	// reload the weapon with the equipped weapon ammo type.
	// fill/reload the magazine with some or all of carried ammo
	EAmmoType EquippedWeaponAmmoType{ EquippedWeapon->GetAmmoType() };

	if (AmmoMap.Contains(EquippedWeaponAmmoType))
	{
		const int32 MagazineSpace{ EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo() };
		const int32 CarriedAmmo{ AmmoMap[EquippedWeaponAmmoType] };

		if (MagazineSpace > CarriedAmmo)
		{
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			AmmoMap[EquippedWeaponAmmoType] = 0;
		}
		else
		{
			EquippedWeapon->ReloadAmmo(MagazineSpace);
			AmmoMap[EquippedWeaponAmmoType] -= MagazineSpace;
		}
	}
}

void AMainCharacter::PlayFireSound() const
{
	if (EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void AMainCharacter::PlayGunFireAnimation() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (HipFireMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(EquippedWeapon->GetHipfireMontageSectionName());
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->PlayFireAnimation();
	}
}

void AMainCharacter::GrabClip()
{
	if (!EquippedWeapon || !HandSceneComponent ) return;
	if (HandBoneName != TEXT("None")) return;

	// Get the clips transform;
	ClipTransform = EquippedWeapon->GetClipBoneTransform();

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName{ HandBoneName });
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
}

void AMainCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

AWeapon* AMainCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		// spawn an actor of class aweapon, the (defaultweaponclass) means we can give it that defaultweaponclass blueprint
		AWeapon* DefaultWeapon{ GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass) };
		return DefaultWeapon;
	}

	return nullptr;
}

void AMainCharacter::InitializeHitReactNamesArray()
{
	HitReactSectionNames.Add(FName{ TEXT("HitReactFront") });
	HitReactSectionNames.Add(FName{ TEXT("HitReactBack") });
	HitReactSectionNames.Add(FName{ TEXT("HitReactLeft") });
	HitReactSectionNames.Add(FName{ TEXT("HitReactRight") });
}

void AMainCharacter::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
	EquipWidgetDelegate.Broadcast(false);
}

void AMainCharacter::DetachWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetMesh()->DetachFromComponent(DetachmentTransformRules);
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->SetInventoryIndex(-1);
		EquippedWeapon->ThrowWeapon();
		EquippedWeapon = nullptr;
	}
}

void AMainCharacter::SwapWeapon(AWeapon* NewWeapon)
{
	if (Inventory.Num() == INVENTORY_CAPACITY)
	{
		Inventory[EquippedWeapon->GetInventoryIndex()] = NewWeapon;
		NewWeapon->SetInventoryIndex(EquippedWeapon->GetInventoryIndex());
	}

	DetachWeapon();
	EquipWeapon(NewWeapon);
	OverlappedPickupItem = nullptr;
}

void AMainCharacter::SetOverlappedPickupItem(AItem* Item_)
{
	OverlappedPickupItem = Item_;
}

void AMainCharacter::AttachWeaponToInventorySocket(AWeapon* Weapon)
{
	if (Weapon)
	{
		if (Weapon->GetIsAPrimaryWeapon())
		{
			if (Inventory.Num() > 2)
			{
				const USkeletalMeshSocket* PrimaryWeaponSocket{ GetMesh()->GetSocketByName(FName("PrimaryWeaponSocket2")) };

				if (PrimaryWeaponSocket)
				{
					PrimaryWeaponSocket->AttachActor(Weapon, GetMesh());
				}
			}
			else
			{
				const USkeletalMeshSocket* PrimaryWeaponSocket{ GetMesh()->GetSocketByName(FName("PrimaryWeaponSocket1")) };

				if (PrimaryWeaponSocket)
				{
					PrimaryWeaponSocket->AttachActor(Weapon, GetMesh());
				}
			}
		}
		else
		{
			const USkeletalMeshSocket* SecondaryWeaponSocket{ GetMesh()->GetSocketByName(FName("SecondaryWeaponSocket")) };

			if (SecondaryWeaponSocket)
			{
				SecondaryWeaponSocket->AttachActor(Weapon, GetMesh());
			}
		}
	}
}

void AMainCharacter::PlayEquipMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (EquipMontage && AnimInstance)
	{
		EquipWidgetDelegate.Broadcast(true);
		AnimInstance->Montage_Play(EquipMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Equip"));
	}
}
void AMainCharacter::CycleWeaponsLeftButtonPressed()
{
	if (bIsPaused || (Inventory.Num() == 1) || (CombatState != ECombatState::ECS_Unoccupied) || bIsInMeleeState)
	{
		return;
	}

	CurrentWeaponIndex = (CurrentWeaponIndex == 0) ? (Inventory.Num() - 1) : (CurrentWeaponIndex - 1);
	AWeapon* OldEquippedWeapon{ EquippedWeapon };
	AWeapon* NewWeapon{ Cast<AWeapon>(Inventory[CurrentWeaponIndex]) };
	EquipWeapon(NewWeapon);

	OldEquippedWeapon->SetItemState(EItemState::EIS_IdleInventory);

	PlayEquipMontage();

	AttachWeaponToInventorySocket(OldEquippedWeapon);

	NewWeapon->SetItemState(EItemState::EIS_Equipped);
	CombatState = ECombatState::ECS_Equipping;
	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };
}

void AMainCharacter::CycleWeaponsRightButtonPressed()
{
	if (bIsPaused || Inventory.Num() == 1 || (CombatState != ECombatState::ECS_Unoccupied) || bIsInMeleeState)
	{
		return;
	}

	CurrentWeaponIndex = (CurrentWeaponIndex + 1) % Inventory.Num();

	AWeapon* OldEquippedWeapon{ EquippedWeapon };
	AWeapon* NewWeapon{ Cast<AWeapon>(Inventory[CurrentWeaponIndex]) };
	EquipWeapon(NewWeapon);

	OldEquippedWeapon->SetItemState(EItemState::EIS_IdleInventory);

	PlayEquipMontage();

	AttachWeaponToInventorySocket(OldEquippedWeapon);

	NewWeapon->SetItemState(EItemState::EIS_Equipped);
	CombatState = ECombatState::ECS_Equipping;
}

void AMainCharacter::SwitchToMeleeState()
{
	if (bIsPaused) return;

	bIsInMeleeState = true;

	AttachWeaponToInventorySocket(EquippedWeapon);
	EquippedWeapon->SetItemState(EItemState::EIS_IdleInventory);
	TargetCameraArmInterpSpeed = MeleeCameraArmInterpSpeed;
	TargetCameraArmLength = MeleeCameraArmLength;
}

void AMainCharacter::SwitchToWeaponState()
{
	if (bIsPaused) return;

	bIsInMeleeState = false;

	const USkeletalMeshSocket* HandSocket{ GetMesh()->GetSocketByName(FName("RightHandSocket")) };

	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, GetMesh());
	}

	EquippedWeapon->SetItemState(EItemState::EIS_Equipped);

	TargetCameraArmInterpSpeed = DefaultCameraArmInterpSpeed;
	TargetCameraArmLength = DefaultCameraArmLength;
}

void AMainCharacter::ToggleMeleeState()
{
	if (bIsPaused) return;

	if (bIsInMeleeState)
	{
		SwitchToWeaponState();
	}
	else
	{
		SwitchToMeleeState();
	}

	PlayEquipMontage();
}

void AMainCharacter::PickupAmmo(AAmmo* Ammo)
{
	const int32 PickupAmmoCount{ Ammo->GetItemAmount() };

	auto PickupAmmoType = Ammo->GetAmmoType();

	if (AmmoMap.Contains(PickupAmmoType))
	{
		AmmoMap[PickupAmmoType] += PickupAmmoCount;

		if (EquippedWeapon && (EquippedWeapon->GetAmmoType() == PickupAmmoType) && (EquippedWeapon->GetAmmo() == 0))
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AMainCharacter::PickupItem()
{
	if (bIsPaused || (CombatState != ECombatState::ECS_Unoccupied))
	{
		return;
	}
	if (OverlappedPickupItem)
	{
		AWeapon* Weapon{ Cast<AWeapon>(OverlappedPickupItem) };

		if (Weapon)
		{
			if (Inventory.Num() < INVENTORY_CAPACITY)
			{
				Weapon->SetInventoryIndex(Inventory.Num());
				Inventory.Add(Weapon);

				CurrentWeaponIndex = Weapon->GetInventoryIndex();

				AWeapon* OldEquippedWeapon{ EquippedWeapon };
				EquipWeapon(Weapon);

				OldEquippedWeapon->SetItemState(EItemState::EIS_IdleInventory);

				AttachWeaponToInventorySocket(OldEquippedWeapon);

				FinishEquipping();

				bIsInMeleeState = false;

				TargetCameraArmInterpSpeed = DefaultCameraArmInterpSpeed;
				TargetCameraArmLength = DefaultCameraArmLength;
			}
			else
			{
				SwapWeapon(Weapon);
			}

			PlayPickupMontage();

			return;
		}

		auto Ammo{ Cast<AAmmo>(OverlappedPickupItem) };

		if (Ammo)
		{
			PickupAmmo(Ammo);
		}
		PlayPickupMontage();
	}
}

void AMainCharacter::SetPlayerPaused(bool paused)
{
	bIsPaused = paused;

	if (bIsPaused)
	{
		GetMesh()->bPauseAnims = true;
	}
	else
	{
		GetMesh()->bPauseAnims = false;
	}
}

void AMainCharacter::StealthButtonPressed_Implementation()
{
	if (bIsPaused) return;
	bIsInStealthMode = true;
}

void AMainCharacter::StealthButtonReleased_Implementation()
{
	if (bIsPaused) return;

	bIsInStealthMode = false;
}

void AMainCharacter::CrouchButtonPressed()
{
	if (bIsPaused) return;

	if (!GetCharacterMovement()->IsFalling())
	{
		bIsCrouching = !bIsCrouching;
		GetCharacterMovement()->GroundFriction = bIsCrouching ? CrouchingGroundFriction : RunningGroundFriction;
	}
}

void AMainCharacter::CrouchStandTransitioning()
{
	GetCharacterMovement()->MaxWalkSpeed = 50.0f;
}

void AMainCharacter::CrouchStandTransitionFinished()
{
	GetCharacterMovement()->MaxWalkSpeed = bIsCrouching ? CrouchSpeed : WalkSpeed;
}

void AMainCharacter::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	APlayerController* PlayerController{ UGameplayStatics::GetPlayerController(this, 0) };

	if (PlayerController)
	{
		DisableInput(PlayerController);
	}

	bUseControllerRotationYaw = true;

	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}

	TargetCameraArmInterpSpeed = DeathCameraArmInterpSpeed;
	TargetCameraArmLength = DeathCameraArmLength;
}

void AMainCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	AThirdPersonShooterGameModeBase* GameMode{ GetWorld()->GetAuthGameMode<AThirdPersonShooterGameModeBase>() };

	if (GameMode)
	{
		GameMode->PawnKilled(this);
	}
}

void AMainCharacter::PlayHitReactMontage()
{
	if (bIsDead) return;

	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

	if (HitReactMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(HitReactMontage);

		const int32 section_idx{ FMath::RandRange(0, HitReactSectionNames.Num() - 1) };
		AnimInstance->Montage_JumpToSection(HitReactSectionNames[section_idx]);
	}
}

void AMainCharacter::PlayExplosionReactMontage()
{
	if (bIsDead) return;

	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

	if (ExplosionReactMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(ExplosionReactMontage);
	}
}

void AMainCharacter::PlayPickupMontage()
{
	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };
	if (AnimInstance && PickupMontage)
	{
		AnimInstance->Montage_Play(PickupMontage);
	}
}

bool AMainCharacter::IsCarryingAmmo() const
{
	if (!EquippedWeapon)
	{
		return false;
	}

	EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return (AmmoMap[AmmoType] > 0);
	}

	return false;
}

void AMainCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_PISTOL, StartingPistolAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
	AmmoMap.Add(EAmmoType::EAT_SHOTGUN, StartingShotgunAmmo );
}

bool AMainCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon)
	{
		return (EquippedWeapon->GetAmmo() > 0);
	}

	return false;
}

void AMainCharacter::Jump()
{
	if (bIsPaused) return;

	if (bIsCrouching)
	{
		bIsCrouching = false;
	}
	if (JumpSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), JumpSound);
	}

	ACharacter::Jump();
}

void AMainCharacter::Footstep()
{
	FHitResult HitResult;
	const FVector Start{ GetActorLocation() };
	const FVector End{ Start + FVector{0.0f, 0.0f, -400.0f } };
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);
	auto HitSurface = HitResult.PhysMaterial->SurfaceType;
	//if (HitSurface == EPS_Grass)
	// use this func when you decide to do material footsteps
	//UE_LOG(LogTemp, Warning, TEXT("Hit Actor: "), *HitResult.Actor->GetName());
}

void AMainCharacter::InitializePrimaryAttackMontageSectionNames()
{
	PrimaryAttackMontageSectionNames.Add(FName{ TEXT("Punch1") });
	PrimaryAttackMontageSectionNames.Add(FName{ TEXT("Punch2") });
	PrimaryAttackMontageSectionNames.Add(FName{ TEXT("Punch3") });
}

void AMainCharacter::InitializeSecondaryAttackMontageSectionNames()
{
	SecondaryAttackMontageSectionNames.Add(FName{ TEXT("Kick1") });
	SecondaryAttackMontageSectionNames.Add(FName{ TEXT("Kick2") });
	SecondaryAttackMontageSectionNames.Add(FName{ TEXT("Kick3") });
	SecondaryAttackMontageSectionNames.Add(FName{ TEXT("Kick4") });
	SecondaryAttackMontageSectionNames.Add(FName{ TEXT("Kick5") });
	SecondaryAttackMontageSectionNames.Add(FName{ TEXT("Kick6") });
	SecondaryAttackMontageSectionNames.Add(FName{ TEXT("Kick7") });
}

void AMainCharacter::PlayPrimaryAttack()
{
	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

	if (MeleeCombatMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(MeleeCombatMontage);
		const int32 section_idx{ FMath::RandRange(0, PrimaryAttackMontageSectionNames.Num() - 1) };
		AnimInstance->Montage_JumpToSection(PrimaryAttackMontageSectionNames[section_idx]);

	}
}

void AMainCharacter::PlaySecondaryAttack()
{
	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

	if (MeleeCombatMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(MeleeCombatMontage);

		if (bIsSprinting)
		{
			AnimInstance->Montage_JumpToSection(FName{ "Slide" });
		}
		else
		{
			const int32 section_idx{ FMath::RandRange(0, SecondaryAttackMontageSectionNames.Num() - 1) };
			AnimInstance->Montage_JumpToSection(SecondaryAttackMontageSectionNames[section_idx]);
		}
	}
}

void AMainCharacter::EndMeleeAttack()
{
	bMeleeAttackInProgress = false;
}

void AMainCharacter::OnCombatCollisionBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEnemy* Enemy{ Cast<AEnemy>(OtherActor) };

	if (Enemy)
	{
		UGameplayStatics::ApplyDamage(Cast<AActor>(Enemy), MeleeDamage, Controller, this, UDamageType::StaticClass());
	}
}

void AMainCharacter::ActivateCombatCollisionBox()
{
	CombatCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMainCharacter::DeactivateCombatCollisionBox()
{
	CombatCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMainCharacter::PlayStrangleMontage()
{
	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

	if (StrangleMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(StrangleMontage);
	}
}

void AMainCharacter::StrangleEnemy_Implementation()
{
	bStranglingInProgress = true;
	bIsCrouching = false;
	APlayerController* PlayerController{ UGameplayStatics::GetPlayerController(this, 0) };

	if (PlayerController)
	{
		//DisableInput(PlayerController);
		bUseControllerRotationYaw = false;
	}
	TargetStrangleSceneComponent->SetWorldTransform(StrangleVictim->GetTransform());
	PlayStrangleMontage();
	StrangleVictim->GetStrangled();
}

void AMainCharacter::FinishStrangle()
{
	bStranglingInProgress = false;
	bIsCrouching = true;
	APlayerController* PlayerController{ UGameplayStatics::GetPlayerController(this, 0) };
	if (PlayerController)
	{
		EnableInput(PlayerController);
		bUseControllerRotationYaw = true;
	}

	StrangleVictim = nullptr;
}

void AMainCharacter::TogglePauseScreen()
{
	AMainCharacterPlayerController* PlayerController{ Cast<AMainCharacterPlayerController>(UGameplayStatics::GetPlayerController(this, 0)) };
	if (PlayerController)
	{
		PlayerController->TogglePauseScreen();
	}
}

void AMainCharacter::PlayMeleeHitSound()
{
	if (MeleeimpactSoundHit && bIsInMeleeState)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), MeleeimpactSoundHit);
	}
}

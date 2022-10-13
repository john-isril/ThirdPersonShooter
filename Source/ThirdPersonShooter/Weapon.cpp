// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Item.h"

AWeapon::AWeapon() :
	bIsFalling{ false },
	Ammo{ 30 }, MagazineCapacity{ 30 }, WeaponType{ EWeaponType::EWT_AssaultRifle }, AmmoType{ EAmmoType::EAT_AR },
	ReloadMontageSectionName{ FName{TEXT("None")} }, bMovingClip{ false }, ClipBoneName{ TEXT("Clip_Bone") }, BarrelSocketName{ TEXT("BarrelSocket") },
	bIsAutomatic{ true }, bIsAPrimaryWeapon{true}
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::PlayFireAnimation()
{
	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

	if (FireMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(FireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AWeapon::ThrowWeapon()
{
	// get the meshes rotation and set it upright, keep the yaw
	FRotator MeshRotation{ 0.0f, GetMesh()->GetComponentRotation().Yaw, 0.0f };
	GetMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// take the right vector, rotate it down 20degs and set it as the impulse direction(direction to throw the weapon )
	const FVector MeshForward{ GetMesh()->GetForwardVector() };
	const FVector MeshRight{ GetMesh()->GetRightVector() };
	FVector ImpulseDirection{ MeshRight.RotateAngleAxis(-20.0f, MeshForward) };

	// rotate the impulse vector around the z axis. world z or mesh z. doesnt matter because mesh only has yaw. random between 0 n 30
	float RandomRotation{ FMath::FRandRange(0.0f, 30.0f) };
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector{ 0.0f, 0.0f, 1.0f });
	
	// impulse strength
	ImpulseDirection *= ThrowWeaponImpulseStrength;
	GetMesh()->AddImpulse(ImpulseDirection);

	bIsFalling = true;
	// after the timer finishes, call the stopfalling function
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponDuration);
}

void AWeapon::IncrementAmmo()
{
	++Ammo;
}

void AWeapon::DecrementAmmo()
{
	if (Ammo > 0)
	{
		--Ammo;
	}
}

void AWeapon::ReloadAmmo(int32 Amnt)
{
	Ammo += Amnt;
}

bool AWeapon::MagazineIsFull() const
{
	return Ammo == MagazineCapacity;
}

int32 AWeapon::GetClipBoneIndex() const
{
	return GetMesh()->GetBoneIndex(GetClipBoneName());
}

FTransform AWeapon::GetClipBoneTransform()
{
	return GetMesh()->GetBoneTransform(GetClipBoneIndex());
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// keep the weapon upright when throwing
	if (GetItemState() == EItemState::EIS_Falling && bIsFalling)
	{
		const FRotator MeshRotation{ 0.0f, GetMesh()->GetComponentRotation().Yaw, 0.0f };
		GetMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	const FString WeaponTablePath(TEXT("DataTable'/Game/Game_/DataTables/Weapon_DataTable.Weapon_DataTable'"));
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if (WeaponTableObject)
	{
		FWeaponDataTable* WeaponDataRow = nullptr;

		switch (WeaponType)
		{
		case EWeaponType::EWT_Shotgun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Shotgun"), TEXT(""));
			break;

		case EWeaponType::EWT_AssaultRifle:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
			break;

		case EWeaponType::EWT_Pistol:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
			break;

		}

		if (WeaponDataRow)
		{
			AmmoType = WeaponDataRow->AmmoType;
			Ammo = WeaponDataRow->WeaponAmmo;
			MagazineCapacity = WeaponDataRow->MagazineCapacity;
			GetMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
			SetItemName(WeaponDataRow->ItemName);
			SetItemIcon(WeaponDataRow->InventoryIcon);

			SetMaterialInstance(WeaponDataRow->MaterialInstance);
			PreviousMaterialIndex = GetMaterialIndex();
			GetMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
			SetMaterialIndex(WeaponDataRow->MaterialIndex);
			SetClipBoneName(WeaponDataRow->ClipBoneName);
			SetBarrelSocketName(WeaponDataRow->BarrelSocketName);
			SetReloadMontageSectionName(WeaponDataRow->ReloadMontageSectionName);
			GetMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);
			SetCrosshairMiddleTexture(WeaponDataRow->CrosshairsMiddle);
			SetCrosshairTopTexture(WeaponDataRow->CrosshairsTop);
			SetCrosshairBottomTexture(WeaponDataRow->CrosshairsBottom);
			SetCrosshairLeftTexture(WeaponDataRow->CrosshairsLeft);
			SetCrosshairRightTexture(WeaponDataRow->CrosshairsRight);
			SetAutoFireRate(WeaponDataRow->AutoFireRate);
			SetMuzzleFlash(WeaponDataRow->MuzzleFlash);
			SetFireSound(WeaponDataRow->FireSound);
			SetFireMontage(WeaponDataRow->FireMontage);
			SetIsAutomatic(WeaponDataRow->bIsAutomatic);
			SetIsAPrimaryWeapon(WeaponDataRow->bIsAPrimaryWeapon);
			SetDamageAmount(WeaponDataRow->DamageAmount);
			SetHeadshotDamageAmount(WeaponDataRow->HeadshotDamageAmount);
			SetHipfireMontageSectionName(WeaponDataRow->HipfireMontageSectionName);
			SetImpactParticles(WeaponDataRow->ImpactParticles);
		}

		if (GetMaterialInstance())
		{
			SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
			GetMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());
			DisableGlowMaterial();
		}
	}

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::StopFalling()
{
	// this is important bc once this is false, we want to set the weapon back upright
	bIsFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}

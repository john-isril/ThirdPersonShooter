// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterAnimInstance.h"
#include "MainCharacter.h"
#include "Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UMainCharacterAnimInstance::UMainCharacterAnimInstance() :
	Speed{ 0.0f }, bIsInAir{ false }, bIsAccelerating{ false }, RootYawOffset{ 0.0f }, CharacterRotation{ 0.0f }, CharacterRotation_LastFrame{ 0.0f },
	CharacterYaw_InPlace{ 0.0f }, CharacterYaw_InPlace_LastFrame{ 0.0f }, bIsAiming{ false }, Pitch{ 0.0f }, bIsReloading{ false },
	ActionState{ EActionState::EAS_Hip },
	StrafeAngle{ 0.0f }, LastStrafeAngle{ 0.0f },
	bIsCrouching{ false }, bIsInMeleeState {false},
	RecoilWeight{ 0.75f }, bIsTurning{ false }, bIsMovingBackwards{false},
	bShouldUseFABRIK{ true }
{
}

void UMainCharacterAnimInstance::NativeInitializeAnimation()
{
	MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
}

void UMainCharacterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!MainCharacter)
	{
		MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
		if (!MainCharacter) return;
	}

	if (MainCharacter->GetEquippedWeapon())
	{
		WeaponType = MainCharacter->GetEquippedWeapon()->GetWeaponType();
	}

	if (MainCharacter->GetEquippedWeapon())
	{
		bIsAutomatic = MainCharacter->GetEquippedWeapon()->GetIsAutomatic();
	}

	bIsCrouching = MainCharacter->GetIsCrouching();
	bIsInAir = MainCharacter->GetCharacterMovement()->IsFalling();
	bIsReloading = (MainCharacter->GetCombatState() == ECombatState::ECS_Reloading);
	bIsEquipping = (MainCharacter->GetCombatState() == ECombatState::ECS_Equipping);
	bIsAccelerating = (MainCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f) ? bIsAccelerating = true : bIsAccelerating = false;
	bIsAiming = MainCharacter->GetIsAiming();
	bShouldUseFABRIK = ((MainCharacter->GetCombatState() == ECombatState::ECS_Unoccupied) || (MainCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress));
	bIsSprinting = MainCharacter->GetIsSprinting();
	bIsInMeleeState = MainCharacter->GetIsInMeleeState();
	CalcSpeed();
	StrafeAngle = CalculateDirection(MainCharacter->GetVelocity(), MainCharacter->GetActorRotation());

	bIsMovingBackwards = (FMath::Abs(StrafeAngle) > 170.0f && bIsAccelerating && !bIsCrouching);

	if (Speed > 0.0f)
	{
		LastStrafeAngle = StrafeAngle;
	}
	
	TurnInPlace();
	UpdateRecoilWeight();
	UpdateActionState();

	if (bIsAiming)
	{
		AimBlendWeight = 0.7f;
	}
	else
	{
		AimBlendWeight = 0.0f;
	}
}

void UMainCharacterAnimInstance::CalcSpeed()
{
	FVector LateralVelocity{ MainCharacter->GetVelocity() };
	LateralVelocity.Z = 0;
	Speed = LateralVelocity.Size();
}

void UMainCharacterAnimInstance::TurnInPlace()
{
	if (!MainCharacter) return;

	Pitch = MainCharacter->GetBaseAimRotation().Pitch;
	bIsTurning = false;

	if (Speed > 0 || bIsInAir)
	{
		//Dont turn in place when the characters moving or is in air
		RootYawOffset = 0.0f;
		CharacterYaw_InPlace = MainCharacter->GetActorRotation().Yaw;
		CharacterYaw_InPlace_LastFrame = CharacterYaw_InPlace;
		RotationCurveValueLastFrame = 0.0f;
		RotationCurve = 0.f;
	}
	else
	{
		CharacterYaw_InPlace_LastFrame = CharacterYaw_InPlace;
		CharacterYaw_InPlace = MainCharacter->GetActorRotation().Yaw;
		const float YawDelta_InPlace{ CharacterYaw_InPlace - CharacterYaw_InPlace_LastFrame };

		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta_InPlace);
		// we will be notified by the animation curve
		// if the turning animation is playing, this is 1
		const bool TurningAnimationIsPlaying{ (GetCurveValue(TEXT("Turning")) > 0.0f) };

		if (TurningAnimationIsPlaying)
		{
			bIsTurning = true;
			RotationCurveValueLastFrame = RotationCurve;
			// When the turning animation plays, the rotation animation curve begins.
			// the value tells us how much we should turn
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveValueLastFrame };

			// if the rootyawoffset is positive, turn left by deltarotation, negative turn right
			if (RootYawOffset > 0.0f)
			{
				RootYawOffset -= DeltaRotation;
			}
			else
			{
				RootYawOffset += DeltaRotation;
			}

			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
			if (ABSRootYawOffset > 90.f)
			{
				const float YawExcess{ ABSRootYawOffset - 90.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
	}
}

void UMainCharacterAnimInstance::UpdateRecoilWeight()
{
	if (bIsInMeleeState)
	{
		RecoilWeight = 1.0f;
	}

	if (bIsTurning)
	{
		RecoilWeight = 0.0f;
	}
	else
	{
		if (bIsReloading || bIsEquipping)
		{
			RecoilWeight = 1.0f;
		}
		else
		{
			switch (WeaponType)
			{
			case EWeaponType::EWT_Shotgun:
				RecoilWeight = 0.5f;
				break;

			case EWeaponType::EWT_Pistol:
				if (bIsAiming || bIsCrouching)
				{
					RecoilWeight = 0.05f;
				}
				else
				{
					RecoilWeight = 0.3f;
				}
				break;
			case EWeaponType::EWT_AssaultRifle:
				if (bIsAiming || bIsCrouching)
				{
					RecoilWeight = 0.1f;
				}
				else
				{
					RecoilWeight = 0.3f;
				}
				break;

			default:
				break;
			}
		}
	}
}

void UMainCharacterAnimInstance::UpdateActionState()
{
	if (bIsReloading)
	{
		ActionState = EActionState::EAS_Reloading;
	}
	else if (bIsInAir)
	{
		ActionState = EActionState::EAS_InAir;
	}
	else if (MainCharacter->GetIsAiming())
	{
		ActionState = EActionState::EAS_Aiming;
	}
	else
	{
		ActionState = EActionState::EAS_Hip;
	}
}

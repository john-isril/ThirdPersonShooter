// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieGirlAnimInstance.h"
#include "Enemy.h"

void UZombieGirlAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!Enemy)
	{
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
	}

	if (!Enemy)
	{
		return;
	}

	bShouldPlayTurnAnimation = Enemy->GetShouldPlayTurnAnimation();
	bShouldPlayLeftTurnAnimation = Enemy->GetShouldPlayLeftTurnAnimation();
	CalcSpeed();
}

void UZombieGirlAnimInstance::CalcSpeed()
{
	FVector LateralVelocity{ Enemy->GetVelocity() };
	LateralVelocity.Z = 0;
	Speed = LateralVelocity.Size();
}
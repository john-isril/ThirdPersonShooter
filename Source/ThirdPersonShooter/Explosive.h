// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletHitInterface.h"
#include "Explosive.generated.h"

UCLASS()
class THIRDPERSONSHOOTER_API AExplosive : public AActor, public IBulletHitInterface
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ExplosionParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* BlastRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties, meta = (AllowPrivateAccess = "true"))
	float DamageAmount { 70.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ExplosionSound;

public:
	// Sets default values for this actor's properties
	AExplosive();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;

};

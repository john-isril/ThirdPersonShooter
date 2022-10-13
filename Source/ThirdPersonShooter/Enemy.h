// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BulletHitInterface.h"
#include "Enemy.generated.h"

UCLASS()
class THIRDPERSONSHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();
	void ShowHealthBar_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	void Die();
	// called in the bullet hit interface
	void PlayHitMontage(FName Section, float PlayRate = 1.0f);
	void PlayExplosiveReactMontage();

	UFUNCTION(BlueprintCallable) // BP Callable in order to call this in the behavior tree
	void PlayAttackMontage();
	void ResetHitReactTimer();

	UFUNCTION()
	void AttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnAttackSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

	// a BP pure means it takes no input
	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	UFUNCTION()
	void OnCombatCollisionBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// these activation/deactivations will be called in anim notifies
	UFUNCTION(BlueprintCallable)
	void ActivateCombatCollisionBox();

	UFUNCTION(BlueprintCallable)
	void DeactivateCombatCollisionBox();

	void SpawnBlood(class AMainCharacter* Character);

	void ApplyDamage(class AMainCharacter* Character);
	void ResetCanAttack();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	void DestroyEnemy();
	void InitializeAttackSectionNamesArray();
	void InitializeWeaponSocketNamesArray();
	void InitializeHitReactSectionNamesArray();

	void ActivateUnalertVision();
	void ActivateAlertVision();
	bool Listen();
	void SetToUnalert();
	void SetToAlert();
	bool LineTraceToPlayer(const FVector& EnemyToPlayer, float Distance);
	void ChasePlayer(const FVector& EnemyToPlayer);

	UFUNCTION(BlueprintCallable)
	void EnableTurnAnimation();

	UFUNCTION(BlueprintCallable)
	void DisableTurnAnimation();

	UFUNCTION(BlueprintCallable)
	void TurnLeft();

	UFUNCTION()
	void OnStrangleCollisionBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void PlayBeingStrangledMontage();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health, meta = (AllowPrivateAccess = "true"))
	float Health {100.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health, meta = (AllowPrivateAccess = "true"))
	float MaxHealth {100.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health, meta = (AllowPrivateAccess = "true"))
	float HealthBarVisibilityTime{ 4.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CombatCollisionBox; // collision volume for enemy melee

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* StrangleCollisionBox;

	/** Particles to spawn when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;

	/** Sound to play when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FString HeadBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ExplosiveReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* BeingStrangledMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax {0.7f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin {0.5f};

	UPROPERTY(EditAnywhere, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	class AEnemyController* EnemyController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AttackSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bIsStunned;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bIsInAttackRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance {0.5f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float WalkSpeed{ 90.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float RunSpeed{ 500.0 };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AlertRotationSpeed{ 300.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float UnalertRotationSpeed{ 90.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bIsAlert;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bShouldPlayTurnAnimation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bShouldPlayLeftTurnAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float BaseDamage {20.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName LeftHandSocket; // location to spawn blood particles

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName RightHandSocket;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName RightFootSocket;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Crosshair)
	TArray<FName> WeaponSocketNames;

	UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bCanAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackWaitTime{ 1.0f };

	FTimerHandle AttackWaitTimer;

	FName AttackL;
	FName AttackR;
	FName AttackLR;
	FName KickR;
	FName PunchL;
	int32 CurrentlyPlayingAttackSectionIDX;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Combat)
	TArray<FName> AttackSectionNames;

	TArray<FName> HitReactSectionNames;

	FTimerHandle HealthBarVisibilityTimer;
	FTimerHandle HitReactTimer;
	bool bCanPlayHitReact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Death, meta = (AllowPrivateAccess = "true"))
	float DeathTime{ 3.0f };

	FTimerHandle DeathTimer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	bool bDying;
	bool bIsDead;
	class AMainCharacter* PlayerReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMAX = "180.0"), Category = AI)
	float VisionCone {180.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = AI)
	float VisionDistance {50.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = AI)
	float ListenDistance{ 1200.0f };

	FVector PatrolLocationA, PatrolLocationB;
	bool bEyesOnPlayer;

	FTimerHandle EyesOffPlayerTimer;
	bool bEyesOffPlayerTimerSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Weapon);
	USceneComponent* StrangleSceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = AI)
	bool bWalksUnalerted{true};

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	FORCEINLINE FString GetHeadBoneName() const { return HeadBoneName; }
	FORCEINLINE void SetHeadBoneName(FString Name) { HeadBoneName = Name; }
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	FORCEINLINE bool GetShouldPlayTurnAnimation() const { return bShouldPlayTurnAnimation; }
	FORCEINLINE bool GetShouldPlayLeftTurnAnimation() const { return bShouldPlayLeftTurnAnimation; }
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }
	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }
	FORCEINLINE bool GetIsDead() const { return bIsDead; }
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetHealth() const { return Health; }

	void GetStrangled();
	void StopBeingStrangled();
};

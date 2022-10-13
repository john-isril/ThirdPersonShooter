// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "MainCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/Vector.h"
#include "Explosive.h"
#include "ThirdPersonShooterGameModeBase.h"
#include "DrawDebugHelpers.h"

// Sets default values
AEnemy::AEnemy() :
	bCanPlayHitReact{ true }, bIsStunned{ false }, bIsInAttackRange{ false },
	AttackL{ TEXT("AttackL") }, AttackR{ TEXT("AttackR") }, AttackLR{ TEXT("AttackLR") }, PunchL{ TEXT("PunchL") }, KickR {TEXT("KickR")},
	LeftHandSocket{ TEXT("LeftHandSocket") }, RightHandSocket{ TEXT("RightHandSocket") }, RightFootSocket{ TEXT("RightFootSocket") }, bCanAttack{ true },
	bDying{ false }, CurrentlyPlayingAttackSectionIDX{0}, bIsAlert{false}, PlayerReference{nullptr}, VisionCone{ 180.0f }, bShouldPlayTurnAnimation{false},
	bEyesOnPlayer{false}, bEyesOffPlayerTimerSet{false}, bIsDead{false}
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphereComponent"));
	AttackSphere->SetupAttachment(GetRootComponent());

	CombatCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Combat Collision Box"));
	CombatCollisionBox->SetupAttachment(GetRootComponent());

	StrangleCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Strangle Collision Box"));
	StrangleCollisionBox->SetupAttachment(GetRootComponent());

	InitializeAttackSectionNamesArray();
	InitializeWeaponSocketNamesArray();
	InitializeHitReactSectionNamesArray();
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AttackSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AttackSphereOverlap);
	AttackSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnAttackSphereEndOverlap);

	CombatCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnCombatCollisionBoxOverlap);
	CombatCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // only trigger collision events when attacking
	CombatCollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	StrangleCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnStrangleCollisionBoxOverlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	EnemyController = Cast<AEnemyController>(GetController());

	if (EnemyController)
	{
		EnemyController->RunBehaviorTree(BehaviorTree);
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), bCanAttack);
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("IsAlert"), bIsAlert);
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("WalksUnalerted"), bWalksUnalerted);
	}

	PlayerReference = Cast<AMainCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	PatrolLocationA = GetActorLocation();
	PatrolLocationB = PatrolLocationA;
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarVisibilityTimer);
	GetWorldTimerManager().SetTimer(HealthBarVisibilityTimer, this, &AEnemy::HideHealthBar, HealthBarVisibilityTime);
}

void AEnemy::Die()
{
	if (bDying) return;
	bDying = true;

	HideHealthBar();

	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}

	if (EnemyController)
	{
		bIsDead = true;
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName{ "IsDead" }, bIsDead);
		EnemyController->StopMovement();

		AThirdPersonShooterGameModeBase* GameMode{ GetWorld()->GetAuthGameMode<AThirdPersonShooterGameModeBase>() };

		if (GameMode)
		{
			GameMode->PawnKilled(this);
		}
	}
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanPlayHitReact)
	{
		UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

		if (AnimInstance && HitMontage)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}

		bCanPlayHitReact = false;
		GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, FMath::FRandRange(HitReactTimeMin, HitReactTimeMax));
	}
}

void AEnemy::PlayExplosiveReactMontage()
{
	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };

	if (AnimInstance && ExplosiveReactMontage)
	{
		AnimInstance->Montage_Play(ExplosiveReactMontage);
	}
}

void AEnemy::PlayAttackMontage()
{
	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage, 1.0f);
		CurrentlyPlayingAttackSectionIDX = FMath::RandRange(0, AttackSectionNames.Num() - 1);
		AnimInstance->Montage_JumpToSection(AttackSectionNames[CurrentlyPlayingAttackSectionIDX], AttackMontage);
	}
	bCanAttack = false;
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime);

	if (EnemyController && EnemyController->GetBlackboardComponent())
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), bCanAttack);
	}
}

void AEnemy::ResetHitReactTimer()
{
	bCanPlayHitReact = true;
}

void AEnemy::AttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	//AMainCharacter *Character{ Cast<AMainCharacter>(OtherActor) };
	if (OtherActor != PlayerReference) return;

	bIsInAttackRange = true;
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bIsInAttackRange);
}

void AEnemy::OnAttackSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	AMainCharacter* Character{ Cast<AMainCharacter>(OtherActor) };

	if (!Character) return;

	bIsInAttackRange = false;

	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bIsInAttackRange);
}

void AEnemy::SetStunned(bool Stunned)
{
	bIsStunned = Stunned;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), bIsStunned);
	}
}

FName AEnemy::GetAttackSectionName()
{
	const int32 SectionIdx{ FMath::RandRange(1, 4) };

	return AttackSectionNames[SectionIdx];
}

void AEnemy::OnCombatCollisionBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMainCharacter* Character{ Cast<AMainCharacter>(OtherActor) };

	if (Character)
	{
		ApplyDamage(Character);
		SpawnBlood(Character);
		Character->PlayHitReactMontage();
	}
}

void AEnemy::ActivateCombatCollisionBox()
{
	CombatCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateCombatCollisionBox()
{
	CombatCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::SpawnBlood(AMainCharacter* Character)
{
	const USkeletalMeshSocket* WeaponSocket{ GetMesh()->GetSocketByName(WeaponSocketNames[CurrentlyPlayingAttackSectionIDX])};

	if (WeaponSocket)
	{
		const FTransform SocketTransform{ WeaponSocket->GetSocketTransform(GetMesh()) };
		if (Character->GetBloodParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Character->GetBloodParticles(), SocketTransform);
		}
	}
}

void AEnemy::ResetCanAttack()
{
	bCanAttack = true;
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), bCanAttack);
	}
}

void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, DeathTime);
}

void AEnemy::DestroyEnemy()
{
	Destroy();
}

void AEnemy::InitializeAttackSectionNamesArray()
{
	AttackSectionNames.Add(AttackL);
	AttackSectionNames.Add(AttackR);
	AttackSectionNames.Add(AttackLR);
	AttackSectionNames.Add(PunchL);
	AttackSectionNames.Add(KickR);
}

void AEnemy::InitializeWeaponSocketNamesArray()
{
	WeaponSocketNames.Add(LeftHandSocket);
	WeaponSocketNames.Add(RightHandSocket);
	WeaponSocketNames.Add(LeftHandSocket);
	WeaponSocketNames.Add(LeftHandSocket);
	WeaponSocketNames.Add(RightFootSocket);
}

void AEnemy::InitializeHitReactSectionNamesArray()
{
	HitReactSectionNames.Add(FName{"HitReactFront"});
	HitReactSectionNames.Add(FName{ "HitReactRight" });
	HitReactSectionNames.Add(FName{ "HitReactBack" });
	HitReactSectionNames.Add(FName{ "HitReactGut" });
}

void AEnemy::ActivateUnalertVision()
{
	if (!PlayerReference || bIsAlert) return;

	const FVector EnemyToPlayer{ PlayerReference->GetActorLocation() - GetActorLocation() };
	const double EnemyToPlayerDistance{ EnemyToPlayer.Size() };
	const double VisionDot{ FVector::DotProduct(GetActorForwardVector(), EnemyToPlayer) };
	
	const double VisionDotOverMagnitude{ VisionDot / (GetActorForwardVector().Size() * EnemyToPlayer.Size()) };
	const double VisionConeRadians  { FMath::Cos(FMath::DegreesToRadians(VisionCone / 2.0))};

	// Successful if the player is within the enemies field of view and if nothing is blocking the enemies vision to the player.
	if (VisionDotOverMagnitude > VisionConeRadians)
	{
		if (LineTraceToPlayer(EnemyToPlayer, VisionDistance))
		{
			SetToAlert();
		}
	}
	else
	{
		bIsAlert = false;
	}

	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsAlert"), bIsAlert);
}

void AEnemy::ActivateAlertVision()
{
	const FVector EnemyToPlayer{ PlayerReference->GetActorLocation() - GetActorLocation() };
	bEyesOnPlayer = LineTraceToPlayer(EnemyToPlayer, VisionDistance);

	if (!bEyesOnPlayer)
	{
		if (!bEyesOffPlayerTimerSet)
		{
			GetWorldTimerManager().SetTimer(EyesOffPlayerTimer, this, &AEnemy::SetToUnalert, 5.0f);
			bEyesOffPlayerTimerSet = true;
		}
	}
	else
	{
		if (bEyesOffPlayerTimerSet)
		{
			GetWorldTimerManager().ClearTimer(EyesOffPlayerTimer);
			bEyesOffPlayerTimerSet = false;
		}
	}
}

bool AEnemy::Listen()
{
	if (!PlayerReference || bIsAlert) return false;

	const FVector EnemyToPlayer{ PlayerReference->GetActorLocation() - GetActorLocation() };
	float EnemyToPlayerDistanceSquared{ static_cast<float>(FVector::DistSquared(GetActorLocation(), PlayerReference->GetActorLocation())) };

	if ((EnemyToPlayerDistanceSquared < (ListenDistance * ListenDistance)) && PlayerReference->GetShotsFired())
	{
		SetToAlert();
		return true;
	}

	return false;
}

void AEnemy::SetToUnalert()
{
	if (bEyesOnPlayer) return;

	bIsAlert = false;
	bEyesOnPlayer = false;
	bEyesOffPlayerTimerSet = false;
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsAlert"), bIsAlert);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->RotationRate.Yaw = UnalertRotationSpeed;
	CombatCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::SetToAlert()
{
	bIsAlert = true;
	bEyesOnPlayer = true;
	EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), PlayerReference);
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsAlert"), bIsAlert);
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	GetCharacterMovement()->RotationRate.Yaw = AlertRotationSpeed;
	bShouldPlayTurnAnimation = false;
	CombatCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool AEnemy::LineTraceToPlayer(const FVector &EnemyToPlayer, float Distance)
{
	if (!PlayerReference) return false;

	FHitResult VisionHitResult{};
	FCollisionQueryParams Params{};
	Params.AddIgnoredActor(this);

	FVector Start{ GetActorLocation() };
	FVector Direction{ EnemyToPlayer };
	Direction.Normalize();
	FVector LineTraceEnd{ Start + Direction * Distance };

	GetWorld()->LineTraceSingleByChannel(VisionHitResult, Start, LineTraceEnd, ECollisionChannel::ECC_Pawn, Params);

	if (Cast<AMainCharacter>(VisionHitResult.GetActor()))
	{
		return true;
	}

	return false;
}

void AEnemy::ChasePlayer(const FVector& EnemyToPlayer)
{
	bIsAlert = LineTraceToPlayer(EnemyToPlayer, VisionDistance);
}

void AEnemy::EnableTurnAnimation()
{
	bShouldPlayTurnAnimation = true;
}

void AEnemy::DisableTurnAnimation()
{
	bShouldPlayTurnAnimation = false;
}

void AEnemy::TurnLeft()
{
	FVector EnemyToPatrolLocation {EnemyController->GetBlackboardComponent()->GetValueAsVector(FName{ "PatrolLocation" }) - GetActorLocation()};
	EnemyToPatrolLocation.Z = 0.0f;

	FVector EnemyForward{ GetActorForwardVector() };
	EnemyForward.Z = 0;

	FVector Turn{ FVector::CrossProduct(EnemyForward, EnemyToPatrolLocation)};
	
	if (Turn.Z < 0)
	{
		bShouldPlayLeftTurnAnimation = true;
	}
	else
	{
		bShouldPlayLeftTurnAnimation = false;
	}

}

void AEnemy::OnStrangleCollisionBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMainCharacter* Player{ Cast<AMainCharacter>(OtherActor) };

	if (Player)
	{
		Player->SetStrangleVictim(this);
	}
}

void AEnemy::ApplyDamage(class AMainCharacter* Character)
{
	if (Character)
	{
		UGameplayStatics::ApplyDamage(Cast<AActor>(Character), BaseDamage, EnemyController, this, UDamageType::StaticClass());
		if (Character->GetMeleeImpactSoundGrunt())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Character->GetMeleeImpactSoundGrunt(), GetActorLocation());
		}
		if (Character->GetMeleeImpactSoundHit())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Character->GetMeleeImpactSoundHit(), GetActorLocation());
		}
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsAlert)
	{
		ActivateAlertVision();
	}
	else if(!Listen())
	{
		ActivateUnalertVision();
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{	
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator{ 0.0f }, true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float TempHealth{ Health - DamageAmount };

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(FName{ "Target" }, PlayerReference); // chase the player once shot at
	}

	if (TempHealth < 0.1)
	{
		Health = 0.0f;
		Die();
	}
	else
	{
		Health = TempHealth;
	}

	if (bDying) return DamageAmount;

	if (Cast<AExplosive>(DamageCauser))
	{
		PlayExplosiveReactMontage();
	}
	else
	{
		const int32 Section{ FMath::RandRange(0, HitReactSectionNames.Num() - 1) };
		PlayHitMontage(HitReactSectionNames[Section], 1.0f);
	}

	const float Stunned{ FMath::FRandRange(0.0f, 1.0f) };

	if (Stunned <= StunChance)
	{
		SetStunned(true);
	}
	else
	{
		SetStunned(false);
	}

	if (!bIsAlert)
	{
		bIsAlert = true;
		bEyesOnPlayer = true;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		GetCharacterMovement()->RotationRate.Yaw = AlertRotationSpeed;
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName{ "IsAlert" }, bIsAlert);
	}

	PlayerReference->PlayMeleeHitSound();

	return DamageAmount;
}

void AEnemy::GetStrangled()
{
	EnemyController->GetBlackboardComponent()->SetValueAsBool(FName{ "IsDead" }, true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PlayBeingStrangledMontage();
}

void AEnemy::StopBeingStrangled()
{
	EnemyController->GetBlackboardComponent()->SetValueAsBool(FName{ "IsDead" }, true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };
	if (AnimInstance && BeingStrangledMontage)
	{
		AnimInstance->StopAllMontages(0.5f);
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName{ "IsDead" }, false);
	}
}

void AEnemy::PlayBeingStrangledMontage()
{
	UAnimInstance* AnimInstance{ GetMesh()->GetAnimInstance() };
	if (AnimInstance && BeingStrangledMontage)
	{
		AnimInstance->Montage_Play(BeingStrangledMontage, 1.0f);
	}
}



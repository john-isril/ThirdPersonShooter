// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Enemy.h"
AEnemyController::AEnemyController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	check(BlackboardComponent);

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	check(BehaviorTreeComponent);
}
void AEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (!InPawn)
	{
		return;
	}

	AEnemy* Enemy{ Cast<AEnemy>(InPawn) };

	if (Enemy)
	{
		UBehaviorTree* EnemyBehaviorTree{ Enemy->GetBehaviorTree() };

		if (EnemyBehaviorTree)
		{
			BlackboardComponent->InitializeBlackboard(*(EnemyBehaviorTree->BlackboardAsset));
		}

	}
}

bool AEnemyController::IsDead() const
{
	AEnemy* Enemy{ Cast<AEnemy>(GetPawn()) };

	if (Enemy)
	{
		return Enemy->GetIsDead();
	}

	return true;
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSONSHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = AI)
	class UBlackboardComponent* BlackboardComponent;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = AI)
	class UBehaviorTreeComponent* BehaviorTreeComponent;

public:
	AEnemyController();
	virtual void OnPossess(APawn* InPawn) override;
	bool IsDead() const;

	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
	
};

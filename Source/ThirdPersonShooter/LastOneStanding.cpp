// Fill out your copyright notice in the Description page of Project Settings.


#include "LastOneStanding.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "EnemyController.h"

void ALastOneStanding::PawnKilled(APawn* PawnKilled)
{
	Super::PawnKilled(PawnKilled);

	APlayerController* PlayerController{ Cast<APlayerController>(PawnKilled->GetController()) };

	if (PlayerController)
	{
		EndGame(false);
	}

	for (AEnemyController* Controller : TActorRange<AEnemyController>(GetWorld()))
	{
		if (!Controller->IsDead())
		{
			return;
		}
	}

	EndGame(true);
}

void ALastOneStanding::EndGame(bool bIsPlayerWinner)
{
	for (AController* Controller : TActorRange<AController>(GetWorld()))
	{
		bool bIsPlayerController{ Controller->IsPlayerController() == bIsPlayerWinner};
		Controller->GameHasEnded(Controller->GetPawn(), bIsPlayerController);
	}
}

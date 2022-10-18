// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterPlayerController.h"
#include "Blueprint/UserWidget.H"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "MainCharacter.h"
#include "Enemy.h"

AMainCharacterPlayerController::AMainCharacterPlayerController() :
	bPauseScreenVisible{false}
{

}

void AMainCharacterPlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	Super::GameHasEnded(EndGameFocus, bIsWinner);

	if (bIsWinner)
	{
		if (WinScreenClass) // check if weve chosen a blueprint class
		{
			WinScreen = CreateWidget<UUserWidget>(this, WinScreenClass); // create an instance of the BP Class
			if (WinScreen)
			{
				WinScreen->AddToViewport();
				WinScreen->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
	else
	{
		if (LoseScreenClass)
		{
			LoseScreen = CreateWidget<UUserWidget>(this, LoseScreenClass);
			if (LoseScreen)
			{
				LoseScreen->AddToViewport();
				LoseScreen->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}

	GetWorldTimerManager().SetTimer(RestartTimer, this, &APlayerController::RestartLevel, RestartLevelDelay);
}

void AMainCharacterPlayerController::DisplayPauseScreen_Implementation()
{
	if (PauseScreen)
	{
		bPauseScreenVisible = true;
		PauseScreen->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputMode{};
		SetInputMode(InputMode);
		bShowMouseCursor = true;

		for (AEnemyController* Controller : TActorRange<AEnemyController>(GetWorld()))
		{
			if (!Controller->IsDead())
			{
				Controller->GetBlackboardComponent()->SetValueAsBool(FName("PlayerPaused"), true);
				Cast<ACharacter>(Controller->GetPawn())->GetMesh()->bPauseAnims = true;
			}
		}

		AMainCharacter* Main{ Cast<AMainCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)) };
		Main->SetPlayerPaused(true);
	}
}

void AMainCharacterPlayerController::HidePauseScreen_Implementation()
{
	if (PauseScreen)
	{
		FInputModeGameOnly InputMode{};
		SetInputMode(InputMode);
		bShowMouseCursor = false;

		bPauseScreenVisible = false;

		for (AEnemyController* Controller : TActorRange<AEnemyController>(GetWorld()))
		{
			if (!Controller->IsDead())
			{
				Controller->GetBlackboardComponent()->SetValueAsBool(FName("PlayerPaused"), false);
				Cast<ACharacter>(Controller->GetPawn())->GetMesh()->bPauseAnims = false;
			}
		}

		HideControlsScreen_Implementation();
		AMainCharacter* Main{ Cast<AMainCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)) };
		Main->SetPlayerPaused(false);
	}
}

void AMainCharacterPlayerController::DisplayControlsScreen_Implementation()
{
	if (ControlsScreen)
	{
		ControlsScreen->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainCharacterPlayerController::HideControlsScreen_Implementation()
{
	if (ControlsScreen)
	{
		ControlsScreen->SetVisibility(ESlateVisibility::Hidden);
	}
}


void AMainCharacterPlayerController::TogglePauseScreen()
{
	bPauseScreenVisible = !bPauseScreenVisible;

	if (bPauseScreenVisible)
	{
		DisplayPauseScreen();
	}
	else
	{
		HidePauseScreen();
	}
}

void AMainCharacterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayClass) // check if weve chosen a blueprint class
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass); // create an instance of the BP Class
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}

	if (PauseScreenClass) // check if weve chosen a blueprint class
	{
		PauseScreen = CreateWidget<UUserWidget>(this, PauseScreenClass); // create an instance of the BP Class
		if (PauseScreen)
		{
			PauseScreen->AddToViewport();
			PauseScreen->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (ControlsScreenClass) // check if weve chosen a blueprint class
	{
		ControlsScreen = CreateWidget<UUserWidget>(this, ControlsScreenClass); // create an instance of the BP Class
		if (ControlsScreen)
		{
			ControlsScreen->AddToViewport();
			ControlsScreen->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
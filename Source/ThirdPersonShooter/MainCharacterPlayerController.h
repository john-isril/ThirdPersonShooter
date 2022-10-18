// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainCharacterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSONSHOOTER_API AMainCharacterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainCharacterPlayerController();
	virtual void GameHasEnded(class AActor* EndGameFocus = nullptr, bool bIsWinner = false) override;
	void TogglePauseScreen();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	TSubclassOf<class UUserWidget> HUDOverlayClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"));
	UUserWidget* HUDOverlay; // this allows us to create an object of the blueprint class

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	TSubclassOf<UUserWidget> LoseScreenClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	UUserWidget* LoseScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	TSubclassOf<UUserWidget> WinScreenClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	UUserWidget* WinScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	TSubclassOf<UUserWidget> PauseScreenClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	UUserWidget* PauseScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	TSubclassOf<UUserWidget> ControlsScreenClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	UUserWidget* ControlsScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode, meta = (AllowPrivateAccess = "true")); // This allows us to choose/reference a blueprint class
	float RestartLevelDelay{ 5.0f };

	FTimerHandle RestartTimer;
	bool bPauseScreenVisible;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	void DisplayPauseScreen();
	void DisplayPauseScreen_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	void HidePauseScreen();
	void HidePauseScreen_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	void DisplayControlsScreen();
	void DisplayControlsScreen_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	void HideControlsScreen();
	void HideControlsScreen_Implementation();
};

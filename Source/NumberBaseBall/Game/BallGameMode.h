// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BallGameMode.generated.h"

class ABallPlayerController;

UCLASS()
class NUMBERBASEBALL_API ABallGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	void PrintChatMessageString(ABallPlayerController* InChattingPlayerController, const FString& InChatMessageString);

	virtual void OnPostLogin(AController* NewPlayer) override;

	FString GenerateRandomNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString CheckAnswer(const FString& InGuessNumberString);

	void IncreaseGuessCount(ABallPlayerController* InChattingPlayerController);

	void ResetGame();

	void JudgeGame(ABallPlayerController* InChattingPlayerController, int InStrikeCount);

protected:
	FString SecretNumberString;

	TArray<TWeakObjectPtr<ABallPlayerController>> AllPlayerControllers;
};

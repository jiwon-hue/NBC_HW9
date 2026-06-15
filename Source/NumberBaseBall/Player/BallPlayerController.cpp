// Fill out your copyright notice in the Description page of Project Settings.

#include "BallPlayerController.h"

#include "../UI/ChatInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../NumberBaseBall.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "../Game/BallGameMode.h"
#include "../Player/BallPlayerState.h"
#include "Net/UnrealNetwork.h"


ABallPlayerController::ABallPlayerController()
{
	bReplicates = true;
}

void ABallPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	// UI 생성 및 보여주기
	if (ChatInputWidgetClass)
	{
		ChatInputWidgetInstance = CreateWidget<UChatInput>(this, ChatInputWidgetClass);
		if (ChatInputWidgetInstance)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (NotificationTextWidgetClass)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (NotificationTextWidgetInstance)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
}

void ABallPlayerController::SetChatMessage(const FString& InChatMessage)
{
	ChatMessageString = InChatMessage;

	if (IsLocalController())
	{
		ABallPlayerState* BallPS = GetPlayerState<ABallPlayerState>();
		if (BallPS)
		{
			FString CombinedMessageString = BallPS->PlayerNameString + TEXT(": ") + InChatMessage;

			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void ABallPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	NumberBaseBallLibrary::PrintChatString(this, InChatMessageString, 10.f);
}

void ABallPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void ABallPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (GM)
	{
		ABallGameMode* BallGM = Cast<ABallGameMode>(GM);
		if (BallGM)
		{
			BallGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}

void ABallPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}

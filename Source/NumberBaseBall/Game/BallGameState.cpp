// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BallGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Player/BallPlayerController.h"

void ABallGameState::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			ABallPlayerController* BallPC = Cast<ABallPlayerController>(PC);
			if (BallPC)
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				BallPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}
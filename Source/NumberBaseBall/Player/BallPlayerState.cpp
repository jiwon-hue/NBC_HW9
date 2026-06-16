
#include "BallPlayerState.h"
#include "Net/UnrealNetwork.h"

ABallPlayerState::ABallPlayerState()
	: PlayerNameString(TEXT("None"))
	, CurrentGuessCount(0)
	, MaxGuessCount(3)
{
	bReplicates = true;
}

void ABallPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerNameString);
	DOREPLIFETIME(ThisClass, CurrentGuessCount);
	DOREPLIFETIME(ThisClass, MaxGuessCount);
}

FString ABallPlayerState::GetPlayerInfoString()
{
	FString PlayerInfoString = TEXT(" [") + FString::FromInt(CurrentGuessCount) + TEXT("/") + FString::FromInt(MaxGuessCount) + TEXT("]");
	return PlayerInfoString;
}
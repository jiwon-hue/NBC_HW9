// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BallGameMode.h"

#include "BallGameState.h"
#include "Player/BallPlayerController.h"
#include "Player/BallPlayerState.h"
#include "EngineUtils.h"


void ABallGameMode::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateRandomNumber();
}

void ABallGameMode::PrintChatMessageString(ABallPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	ABallPlayerState* BallPS = InChattingPlayerController->GetPlayerState<ABallPlayerState>();

	if (BallPS->CurrentGuessCount < BallPS->MaxGuessCount && IsGuessNumberString(GuessNumberString, InChattingPlayerController) )
	{
		FString JudgeResultString = CheckAnswer(GuessNumberString);

		IncreaseGuessCount(InChattingPlayerController);

		for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
		{
			ABallPlayerController* BallPC = AllPlayerControllers[i].Get();
			if (!BallPC)
			{
				AllPlayerControllers.RemoveAt(i);
				continue;
			}

			if (BallPS)
			{
				FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString + BallPS->GetPlayerInfoString();
				BallPC->ClientRPCPrintChatMessageString(CombinedMessageString);
			}
		}

		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
		JudgeGame(InChattingPlayerController, StrikeCount);
	}
	else
	{
		if (BallPS->CurrentGuessCount >= BallPS->MaxGuessCount)
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("You have used up all attempts"));
		}

		for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
		{
			ABallPlayerController* BallPC = AllPlayerControllers[i].Get();
			if (!BallPC)
			{
				AllPlayerControllers.RemoveAt(i);
				continue;
			}
			BallPC->ClientRPCPrintChatMessageString(InChatMessageString);
		}
	}
}

void ABallGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ABallPlayerController* BallPC = Cast<ABallPlayerController>(NewPlayer);
	if (BallPC)
	{
		BallPC->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.Add(BallPC);

		static int32 PlayerNumber = 0;
		ABallPlayerState* BallPS = BallPC->GetPlayerState<ABallPlayerState>();
		if (BallPS)
		{
			BallPS->PlayerNameString = TEXT("Player") + FString::FromInt(++PlayerNumber);
		}

		ABallGameState* BallGS = GetGameState<ABallGameState>();
		if (BallGS)
		{
			BallGS->MulticastRPCBroadcastLoginMessage(BallPS->PlayerNameString);
		}
	}
}

FString ABallGameMode::GenerateRandomNumber()
{
	FString Result;
	TSet<int32> Nums;
	while(Nums.Num() != 3)
	{
		Nums.Add(FMath::RandRange(1, 9));
	}
	for (int32 Num : Nums)
	{
		Result.AppendInt(Num);
	}

	return Result;
}

bool ABallGameMode::IsGuessNumberString(const FString& InNumberString, ABallPlayerController* InChattingPlayerController)
{
	bool bCanPlay = false;

	do {

		bool bIsUnique = true;
		
		if (InNumberString.Len() != 3)
		{
			break;
		}

		if (InNumberString.Len() != 3)
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("!! Enter three at least 3 digits"));
			bIsUnique = false;
			break;
		}
		
		if (!bIsUnique)
		{
			
			break;
		}

		TArray<TCHAR> Digits;
		TSet<TCHAR> UniqueDigits;
		// 입력한 것이 숫자 3자리인지 확인
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
			}
			Digits.Add(C);
		}
		if (!bIsUnique)
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("!! Enter only numbers excluding 0"));
			break;
		}

		// 입력한 것이 중복이 없는지 확인
		for (TCHAR Num : Digits)
		{
			UniqueDigits.Add(Num);
		}
		if (UniqueDigits.Num() <= 2)
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("!! Enter a non-duplicate number"));
			bIsUnique = false;
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;

	} while (false);

	return bCanPlay;
}

FString ABallGameMode::CheckAnswer(const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (SecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (SecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ABallGameMode::IncreaseGuessCount(ABallPlayerController* InChattingPlayerController)
{
	ABallPlayerState* BallPS = InChattingPlayerController->GetPlayerState<ABallPlayerState>();
	if (BallPS)
	{
		if (BallPS->CurrentGuessCount < BallPS->MaxGuessCount)
		{
			BallPS->CurrentGuessCount++;
		}		
	}
}

void ABallGameMode::ResetGame()
{
	SecretNumberString = GenerateRandomNumber();

	for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
	{
		ABallPlayerController* BallPC = AllPlayerControllers[i].Get();
		if (BallPC)
		{
			ABallPlayerState* BallPS = BallPC->GetPlayerState<ABallPlayerState>();
			if (BallPS)
			{
				BallPS->CurrentGuessCount = 0;
			}
		}
	}
}

void ABallGameMode::JudgeGame(ABallPlayerController* InChattingPlayerController, int32 InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		ABallPlayerState* BallPS = InChattingPlayerController->GetPlayerState<ABallPlayerState>();

		for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
		{
			ABallPlayerController* BallPC = AllPlayerControllers[i].Get();
			if (BallPC)
			{
				if (BallPS)
				{
					FString CombinedMessageString = BallPS->PlayerNameString + TEXT(" has won the game.");
					BallPC->NotificationText = FText::FromString(CombinedMessageString);
				}
			}
		}

		ResetGame();
	}
	else
	{
		bool bIsDraw = true;
		for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
		{
			ABallPlayerController* BallPC = AllPlayerControllers[i].Get();
			if (BallPC)
			{
				ABallPlayerState* BallPS = BallPC->GetPlayerState<ABallPlayerState>();
				if (BallPS)
				{
					if (BallPS->CurrentGuessCount < BallPS->MaxGuessCount)
					{
						bIsDraw = false;
						break;
					}
				}
			}
		}

		if (bIsDraw)
		{
			for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
			{
				ABallPlayerController* BallPC = AllPlayerControllers[i].Get();
				if (BallPC)
				{
					BallPC->NotificationText = FText::FromString(TEXT("Draw..."));
				}
			}

			ResetGame();
		}
	}
}

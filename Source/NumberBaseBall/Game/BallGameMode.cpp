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
	if (IsGuessNumberString(GuessNumberString))
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

			ABallPlayerState* BallPS = InChattingPlayerController->GetPlayerState<ABallPlayerState>();
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
		// string이 숫자로만 이루어져있고
		if (GuessNumberString.IsNumeric())
		{
			// 3자리 숫자를 입력하지 않은 경우
			if (GuessNumberString.Len() != 3)
			{
				InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("Enter three digits"));
			}

			// 중복된 숫자를 입력한 경우
			for (int32 Index1 = 0; Index1 < GuessNumberString.Len(); Index1++)
			{
				for (int32 Index2 = 0; Index2 < GuessNumberString.Len(); Index2++)
				{
					if (Index1 != Index2)
					{
						if (GuessNumberString[Index1] == GuessNumberString[Index2])
						{
							InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("Enter a non-overlapping number"));
							return;
						}
					}
				}
			}
			return;
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

bool ABallGameMode::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {

		// InNumberString가 3개가 아니면 종료
		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (UniqueDigits.Num() <= 2)
		{
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
		BallPS->CurrentGuessCount++;
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

void ABallGameMode::JudgeGame(ABallPlayerController* InChattingPlayerController, int InStrikeCount)
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

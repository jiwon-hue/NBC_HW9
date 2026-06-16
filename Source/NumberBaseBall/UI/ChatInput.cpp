
#include "ChatInput.h"

#include "Components/EditableTextBox.h"
#include "../Player/BallPlayerController.h"

void UChatInput::NativeConstruct()
{
	Super::NativeConstruct();

	// TextCommitted 라는 TextBox의 델리게이트에 바인드 시도
	if (!(ChatInputBox->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted)))
	{
		ChatInputBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UChatInput::NativeDestruct()
{
	Super::NativeDestruct();

	// TextCommitted 바인드 해제
	if (ChatInputBox->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted))
	{
		ChatInputBox->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UChatInput::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		// 컨트롤러 가져오기
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			ABallPlayerController* BallPC = Cast<ABallPlayerController>(PC);
			if (BallPC)
			{
				// 출력
				BallPC->SetChatMessage(Text.ToString());

				// 텍스트 리셋
				ChatInputBox->SetText(FText());
			}
		}
	}
}

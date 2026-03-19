#include "OffBalanceGameOverWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UOffBalanceGameOverWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (replay)
	{
		replay->OnClicked.AddDynamic(this, &UOffBalanceGameOverWidget::HandleReplayClicked);
	}
}

void UOffBalanceGameOverWidget::UpdateScoreHistory(const TArray<int32>& TopScores)
{
	if (!ScoreHistory)
	{
		return;
	}

	FString Result;
	for (int32 Index = 0; Index < TopScores.Num(); ++Index)
	{
		Result += FString::Printf(TEXT("%d. %d"), Index + 1, TopScores[Index]);
		if (Index + 1 < TopScores.Num())
		{
			Result += TEXT("\n");
		}
	}

	ScoreHistory->SetText(FText::FromString(Result));
}

void UOffBalanceGameOverWidget::HandleReplayClicked()
{
	OnReplayClicked.Broadcast();
}

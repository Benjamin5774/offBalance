#include "OffBalanceGameplayWidget.h"

#include "Components/TextBlock.h"

void UOffBalanceGameplayWidget::UpdateScore(int32 NewScore)
{
	if (CurrentScore)
	{
		CurrentScore->SetText(FText::AsNumber(NewScore));
	}
}

void UOffBalanceGameplayWidget::UpdateTimerText(float RemainingSeconds)
{
	if (!Timer)
	{
		return;
	}

	const int32 SecondsCeil = FMath::Max(0, FMath::CeilToInt(RemainingSeconds));
	Timer->SetText(FText::AsNumber(SecondsCeil));
}

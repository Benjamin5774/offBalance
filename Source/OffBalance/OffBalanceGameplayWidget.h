#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OffBalanceGameplayWidget.generated.h"

class UTextBlock;

UCLASS()
class OFFBALANCE_API UOffBalanceGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateScore(int32 NewScore);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateTimerText(float RemainingSeconds);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentScore;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Timer;
};

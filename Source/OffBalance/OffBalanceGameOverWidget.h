#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OffBalanceGameOverWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOffBalanceReplayClicked);

UCLASS()
class OFFBALANCE_API UOffBalanceGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateScoreHistory(const TArray<int32>& TopScores);

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOffBalanceReplayClicked OnReplayClicked;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreHistory;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> replay;

private:
	UFUNCTION()
	void HandleReplayClicked();
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UfoGameMode.generated.h"

class UOffBalanceGameOverWidget;
class UOffBalanceGameplayWidget;

UCLASS()
class OFFBALANCE_API AUfoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUfoGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RegisterCollectedItem(int32 ScoreAmount = 1);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RestartCurrentGame();

	UFUNCTION(BlueprintPure, Category = "Game")
	int32 GetCurrentScore() const { return CurrentScore; }

	UFUNCTION(BlueprintPure, Category = "Game")
	float GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Game")
	bool IsGameOver() const { return bGameOver; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UOffBalanceGameplayWidget> InGameWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UOffBalanceGameOverWidget> GameOverWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (ClampMin = "1"))
	int32 GameDurationSeconds = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (ClampMin = "1"))
	int32 MaxSavedScores = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString ScoreSaveSlotName = TEXT("OffBalanceScoreSlot");

private:
	void CreateInGameWidget();
	void UpdateInGameWidget();
	void HandleGameOver();
	void ShowGameOverWidget(const TArray<int32>& TopScores);
	TArray<int32> SaveAndGetTopScores() const;
	void SetupGameplayInputMode();

	UFUNCTION()
	void HandleReplayClicked();

	UPROPERTY(Transient)
	TObjectPtr<UOffBalanceGameplayWidget> InGameWidget;

	UPROPERTY(Transient)
	TObjectPtr<UOffBalanceGameOverWidget> GameOverWidget;

	int32 CurrentScore = 0;
	float RemainingTime = 0.f;
	bool bGameOver = false;
};

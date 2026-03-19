#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "OffBalanceScoreSaveGame.generated.h"

UCLASS()
class OFFBALANCE_API UOffBalanceScoreSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	TArray<int32> TopScores;
};

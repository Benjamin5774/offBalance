#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MsFallBallController.generated.h"

UCLASS()
class OFFBALANCE_API AMsFallBallController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Move")
	void SetMoveInput(const FVector2D& Input);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Move")
	FVector2D MoveInput = FVector2D::ZeroVector;
};

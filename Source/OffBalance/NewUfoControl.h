#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NewUfoControl.generated.h"

UCLASS()
class OFFBALANCE_API ANewUfoControl : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "UFO|Input")
	void SetMoveInput(const FVector2D& Input);

	UFUNCTION(BlueprintPure, Category = "UFO|Input")
	FVector2D GetMoveInput() const { return MoveInput; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Input")
	bool bReadKeyboardInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Input")
	bool bUseArrowKeys = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Input")
	bool bNormalizeCombinedInput = true;

private:
	FVector2D GetKeyboardMoveInput() const;

	UPROPERTY(BlueprintReadOnly, Category = "UFO|Input", meta = (AllowPrivateAccess = "true"))
	FVector2D MoveInput = FVector2D::ZeroVector;
};

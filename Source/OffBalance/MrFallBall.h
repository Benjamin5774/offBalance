#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MrFallBall.generated.h"

UCLASS()
class OFFBALANCE_API AMrFallBall : public APawn
{
	GENERATED_BODY()

public:
	AMrFallBall();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	// Called from BP/Controller after parsing serial value (-1..+1)
	UFUNCTION(BlueprintCallable, Category = "Gyro")
	void SetGyroSteer(float InSteer);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* BallMesh;

	// Movement tuning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float ForwardSpeed = 600.f; // units/sec

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float SteerSpeed = 500.f;   // units/sec sideways

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float SteerDeadzone = 0.08f; // ignore tiny drift

private:
	float GyroSteer = 0.f; // -1..+1
};

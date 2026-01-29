#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrunZone.generated.h"

class UBoxComponent;

UCLASS()
class OFFBALANCE_API ATrunZone : public AActor
{
	GENERATED_BODY()

public:
	ATrunZone();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* Box;

	// Half-extent of the trigger box (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrunZone")
	FVector BoxExtent = FVector(200.f, 200.f, 200.f);

	// Actor tag that can trigger turning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrunZone")
	FName EnterTag = "TurningPointEnter";

	// Actors with this tag will be rotated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrunZone")
	FName TargetTag = "TurningPoint";

	// Rotate around world Z (yaw) by this degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrunZone")
	float TurnYawDegrees = 90.f;

	UFUNCTION()
	void OnBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};

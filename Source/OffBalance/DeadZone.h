#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeadZone.generated.h"

class UBoxComponent;

UCLASS()
class OFFBALANCE_API ADeadZone : public AActor
{
	GENERATED_BODY()

public:
	ADeadZone();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* Box;

	// Half-extent of the trigger box (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeadZone")
	FVector BoxExtent = FVector(200.f, 200.f, 200.f);

	// Only overlap with components that have this tag (e.g. set on Pawn's mesh)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeadZone")
	FName TargetComponentTag = "DeadZoneTarget";

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

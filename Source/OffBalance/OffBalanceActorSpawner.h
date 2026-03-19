#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OffBalanceActorSpawner.generated.h"

class UBoxComponent;

UCLASS()
class OFFBALANCE_API AOffBalanceActorSpawner : public AActor
{
	GENERATED_BODY()

public:
	AOffBalanceActorSpawner();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void MaintainActorCount();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	TObjectPtr<UBoxComponent> SpawnBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<AActor> ActorClassToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0"))
	int32 TargetCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0.05"))
	float RefillCheckInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	FVector BoxExtent = FVector(400.f, 400.f, 150.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	FName SpawnedActorTag = TEXT("UFOGrabbable");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	bool bStartFilled = true;

private:
	FVector GetRandomSpawnLocation() const;
	void SpawnOneActor();
	void CleanupInvalidActors();

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> SpawnedActors;

	FTimerHandle RefillTimerHandle;
};

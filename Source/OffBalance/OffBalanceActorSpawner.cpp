#include "OffBalanceActorSpawner.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AOffBalanceActorSpawner::AOffBalanceActorSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	RootComponent = SpawnBox;

	SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnBox->SetGenerateOverlapEvents(false);
	SpawnBox->SetBoxExtent(BoxExtent);
	SpawnBox->SetHiddenInGame(true);
	SpawnBox->ShapeColor = FColor::Green;
}

void AOffBalanceActorSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bStartFilled)
	{
		MaintainActorCount();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RefillTimerHandle,
			this,
			&AOffBalanceActorSpawner::MaintainActorCount,
			RefillCheckInterval,
			true
		);
	}
}

void AOffBalanceActorSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SpawnBox)
	{
		SpawnBox->SetBoxExtent(BoxExtent);
		SpawnBox->SetHiddenInGame(true);
	}
}

void AOffBalanceActorSpawner::MaintainActorCount()
{
	CleanupInvalidActors();

	if (!ActorClassToSpawn)
	{
		return;
	}

	int32 SpawnAttempts = 0;
	const int32 MaxSpawnAttempts = FMath::Max(TargetCount * 3, 8);

	while (SpawnedActors.Num() < TargetCount && SpawnAttempts < MaxSpawnAttempts)
	{
		SpawnOneActor();
		++SpawnAttempts;
	}
}

FVector AOffBalanceActorSpawner::GetRandomSpawnLocation() const
{
	const FVector Origin = GetActorLocation();

	return FVector(
		Origin.X + FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		Origin.Y + FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		Origin.Z + FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z)
	);
}

void AOffBalanceActorSpawner::SpawnOneActor()
{
	UWorld* World = GetWorld();
	if (!World || !ActorClassToSpawn)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(
		ActorClassToSpawn,
		GetRandomSpawnLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!SpawnedActor)
	{
		return;
	}

	if (!SpawnedActorTag.IsNone())
	{
		SpawnedActor->Tags.AddUnique(SpawnedActorTag);
	}

	SpawnedActors.Add(SpawnedActor);
}

void AOffBalanceActorSpawner::CleanupInvalidActors()
{
	SpawnedActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
	{
		return !ActorPtr.IsValid();
	});
}

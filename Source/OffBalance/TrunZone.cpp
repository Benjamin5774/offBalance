#include "TrunZone.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ATrunZone::ATrunZone()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("TrunZoneBox"));
	RootComponent = Box;

	Box->SetBoxExtent(BoxExtent);
	Box->SetCollisionProfileName(TEXT("Trigger"));
	Box->SetGenerateOverlapEvents(true);

	Box->OnComponentBeginOverlap.AddDynamic(this, &ATrunZone::OnBoxOverlap);
}

void ATrunZone::BeginPlay()
{
	Super::BeginPlay();

	UKismetSystemLibrary::PrintString(
		this,
		TEXT("TrunZone active"),
		true,
		true,
		FLinearColor(0.7f, 1.f, 0.4f, 1.f),
		2.f
	);
}

void ATrunZone::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Box)
	{
		Box->SetBoxExtent(BoxExtent);
	}
}

void ATrunZone::OnBoxOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	const bool bActorTagged = OtherActor && OtherActor->ActorHasTag(EnterTag);
	const bool bCompTagged = OtherComp && OtherComp->ComponentHasTag(EnterTag);
	if (!OtherActor || (!bActorTagged && !bCompTagged))
	{
		return;
	}

	UKismetSystemLibrary::PrintString(
		this,
		FString::Printf(TEXT("TurningPointEnter: %s entered TrunZone"), *OtherActor->GetName()),
		true,
		true,
		FLinearColor(0.f, 0.66f, 1.f, 1.f),
		2.f
	);

	TArray<AActor*> Targets;
	UGameplayStatics::GetAllActorsWithTag(this, TargetTag, Targets);

	if (Targets.IsEmpty())
	{
		return;
	}

	const FRotator Delta(0.f, TurnYawDegrees, 0.f);
	for (AActor* Target : Targets)
	{
		if (Target)
		{
			Target->AddActorWorldRotation(Delta);
			UKismetSystemLibrary::PrintString(
				this,
				FString::Printf(TEXT("TurningPoint rotated: %s by %0.2f deg"), *Target->GetName(), TurnYawDegrees),
				true,
				true,
				FLinearColor(0.f, 0.66f, 1.f, 1.f),
				2.f
			);
		}
	}
}

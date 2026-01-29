#include "DeadZone.h"
#include "Components/BoxComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ADeadZone::ADeadZone()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("DeadZoneBox"));
	RootComponent = Box;

	Box->SetBoxExtent(BoxExtent);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldStatic);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Box->OnComponentBeginOverlap.AddDynamic(this, &ADeadZone::OnBoxOverlap);
}

void ADeadZone::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Box)
	{
		Box->SetBoxExtent(BoxExtent);
	}
}

void ADeadZone::OnBoxOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	if (!OtherComp || !OtherComp->ComponentHasTag(TargetComponentTag))
	{
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		Pawn->Destroy();
		return;
	}

	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (!GameMode)
	{
		Pawn->Destroy();
		return;
	}

	// Destroy current pawn then respawn at a PlayerStart.
	Pawn->Destroy();
	GameMode->RestartPlayer(Controller);
}

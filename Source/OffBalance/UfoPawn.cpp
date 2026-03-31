#include "UfoPawn.h"

#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UfoGameMode.h"

AUfoPawn::AUfoPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
	VisualRoot->SetupAttachment(RootComponent);

	UfoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UfoMesh"));
	UfoMesh->SetupAttachment(VisualRoot);
	UfoMesh->SetSimulatePhysics(false);
	UfoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AUfoPawn::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	StartLocation.Z = GetActorLocation().Z;

	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z = StartLocation.Z + Height;
	SetActorLocation(SpawnLocation);

	if (VisualRoot)
	{
		RestVisualScale = VisualRoot->GetRelativeScale3D();
	}
}

void AUfoPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyMovement(DeltaTime);
	UpdateHoverHeight(DeltaTime);
	UpdateAbsorb(DeltaTime);
	UpdateTilt(DeltaTime);
	UpdateCollectPulse(DeltaTime);
}

void AUfoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	PlayerInputComponent->BindAxis(TEXT("UFO_MoveForward"), this, &AUfoPawn::MoveForwardAxis);
	PlayerInputComponent->BindAxis(TEXT("UFO_MoveRight"), this, &AUfoPawn::MoveRightAxis);
}

void AUfoPawn::Move(const FVector2D& Input)
{
	FVector2D NewInput(
		FMath::Clamp(Input.X, -1.f, 1.f),
		FMath::Clamp(Input.Y, -1.f, 1.f)
	);

	if (NewInput.SizeSquared() > 1.f)
	{
		NewInput.Normalize();
	}

	CurrentMoveInput = NewInput;
}

void AUfoPawn::MoveForwardAxis(float Value)
{
	if (!bEnableWASDInput)
	{
		return;
	}

	ForwardAxisValue = Value;
	Move(FVector2D(RightAxisValue, ForwardAxisValue));
}

void AUfoPawn::MoveRightAxis(float Value)
{
	if (!bEnableWASDInput)
	{
		return;
	}

	RightAxisValue = Value;
	Move(FVector2D(RightAxisValue, ForwardAxisValue));
}

void AUfoPawn::ApplyMovement(float DeltaTime)
{
	if (CurrentMoveInput.IsNearlyZero())
	{
		return;
	}

	const FVector MoveDirection =
		(GetActorForwardVector() * CurrentMoveInput.Y) +
		(GetActorRightVector() * CurrentMoveInput.X);

	if (MoveDirection.IsNearlyZero())
	{
		return;
	}

	const FVector Delta = MoveDirection.GetSafeNormal() * MoveSpeed * DeltaTime;
	AddActorWorldOffset(Delta, true);
}

void AUfoPawn::UpdateHoverHeight(float DeltaTime)
{
	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = CurrentLocation;
	TargetLocation.Z = StartLocation.Z + Height;

	if (HeightFollowSpeed <= 0.f)
	{
		SetActorLocation(TargetLocation);
		return;
	}

	const FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, HeightFollowSpeed);
	SetActorLocation(NewLocation);
}

void AUfoPawn::UpdateTilt(float DeltaTime)
{
	if (!VisualRoot)
	{
		return;
	}

	const float TargetPitch = -CurrentMoveInput.Y * MaxTiltPitch;
	const float TargetRoll = CurrentMoveInput.X * MaxTiltRoll;

	if (TiltInterpSpeed <= 0.f)
	{
		CurrentTiltPitch = TargetPitch;
		CurrentTiltRoll = TargetRoll;
	}
	else
	{
		CurrentTiltPitch = FMath::FInterpTo(CurrentTiltPitch, TargetPitch, DeltaTime, TiltInterpSpeed);
		CurrentTiltRoll = FMath::FInterpTo(CurrentTiltRoll, TargetRoll, DeltaTime, TiltInterpSpeed);
	}

	VisualRoot->SetRelativeRotation(FRotator(CurrentTiltPitch, 0.f, CurrentTiltRoll));
}

void AUfoPawn::UpdateAbsorb(float DeltaTime)
{
	if (!bEnableAutoAbsorb)
	{
		AbsorbingActor = nullptr;
		AbsorbingComponent = nullptr;
		AbsorbingPhysicsComponent = nullptr;
		CurrentAbsorbElapsed = 0.f;
		return;
	}

	if (!IsValid(AbsorbingActor))
	{
		AbsorbingActor = FindAbsorbTarget();
		if (IsValid(AbsorbingActor))
		{
			PrepareActorForAbsorb(AbsorbingActor);
		}
	}

	if (!IsValid(AbsorbingActor) || !IsValid(AbsorbingComponent))
	{
		return;
	}

	const FVector TargetLocation = GetAbsorbTargetLocation();
	CurrentAbsorbElapsed += DeltaTime;

	const float Duration = FMath::Max(AbsorbDuration, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(CurrentAbsorbElapsed / Duration, 0.f, 1.f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);
	const FVector NewLocation = FMath::Lerp(AbsorbStartLocation, TargetLocation, EasedAlpha);

	SetCurrentAbsorbingLocation(NewLocation);

	if (Alpha >= 1.f || FVector::DistSquared(NewLocation, TargetLocation) <= FMath::Square(CollectDistance))
	{
		CompleteAbsorb();
	}
}

void AUfoPawn::UpdateCollectPulse(float DeltaTime)
{
	if (!VisualRoot)
	{
		return;
	}

	CurrentVisualScaleMultiplier = FMath::FInterpTo(CurrentVisualScaleMultiplier, 1.f, DeltaTime, CollectPulseReturnSpeed);
	VisualRoot->SetRelativeScale3D(RestVisualScale * CurrentVisualScaleMultiplier);
}

AActor* AUfoPawn::FindAbsorbTarget() const
{
	if (GrabbableActorTag.IsNone())
	{
		return nullptr;
	}

	TArray<AActor*> Candidates;
	UGameplayStatics::GetAllActorsWithTag(this, GrabbableActorTag, Candidates);

	const FVector UfoLocation = GetActorLocation();
	const float MaxHorizontalDistanceSq = FMath::Square(AbsorbRadius);

	AActor* BestActor = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (AActor* Candidate : Candidates)
	{
		if (!IsValid(Candidate) || Candidate == this)
		{
			continue;
		}

		const FVector Delta = UfoLocation - Candidate->GetActorLocation();
		if (Delta.Z < 0.f || Delta.Z > AbsorbDepth)
		{
			continue;
		}

		const float HorizontalDistanceSq = FVector2D(Delta.X, Delta.Y).SizeSquared();
		if (HorizontalDistanceSq > MaxHorizontalDistanceSq)
		{
			continue;
		}

		const float DistanceSq = Delta.SizeSquared();
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestActor = Candidate;
		}
	}

	return BestActor;
}

FVector AUfoPawn::GetAbsorbTargetLocation() const
{
	return GetActorLocation() + AbsorbOffset;
}

void AUfoPawn::PrepareActorForAbsorb(AActor* ActorToPrepare)
{
	if (!IsValid(ActorToPrepare))
	{
		return;
	}

	AbsorbingComponent = FindBestAbsorbComponent(ActorToPrepare);
	AbsorbingPhysicsComponent = Cast<UPrimitiveComponent>(AbsorbingComponent);
	if (!IsValid(AbsorbingComponent))
	{
		return;
	}

	AbsorbStartLocation = GetCurrentAbsorbingLocation();
	CurrentAbsorbElapsed = 0.f;

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	ActorToPrepare->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		if (PrimitiveComponent->IsSimulatingPhysics())
		{
			PrimitiveComponent->SetSimulatePhysics(false);
		}

		PrimitiveComponent->SetEnableGravity(false);
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AUfoPawn::CompleteAbsorb()
{
	if (AUfoGameMode* UfoGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AUfoGameMode>() : nullptr)
	{
		UfoGameMode->RegisterCollectedItem();
	}

	if (IsValid(AbsorbingActor))
	{
		AbsorbingActor->Destroy();
	}

	AbsorbingActor = nullptr;
	AbsorbingComponent = nullptr;
	AbsorbingPhysicsComponent = nullptr;
	CurrentAbsorbElapsed = 0.f;
	CurrentVisualScaleMultiplier = CollectPulseScale;
}

USceneComponent* AUfoPawn::FindBestAbsorbComponent(AActor* ActorToPrepare) const
{
	if (!IsValid(ActorToPrepare))
	{
		return nullptr;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	ActorToPrepare->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent && PrimitiveComponent->IsSimulatingPhysics())
		{
			return PrimitiveComponent;
		}
	}

	if (USceneComponent* ActorRootComponent = ActorToPrepare->GetRootComponent())
	{
		return ActorRootComponent;
	}

	return nullptr;
}

FVector AUfoPawn::GetCurrentAbsorbingLocation() const
{
	if (IsValid(AbsorbingComponent))
	{
		return AbsorbingComponent->GetComponentLocation();
	}

	if (IsValid(AbsorbingActor))
	{
		return AbsorbingActor->GetActorLocation();
	}

	return FVector::ZeroVector;
}

void AUfoPawn::SetCurrentAbsorbingLocation(const FVector& NewLocation)
{
	if (IsValid(AbsorbingPhysicsComponent))
	{
		AbsorbingPhysicsComponent->SetWorldLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	if (IsValid(AbsorbingComponent))
	{
		AbsorbingComponent->SetWorldLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	if (IsValid(AbsorbingActor))
	{
		AbsorbingActor->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

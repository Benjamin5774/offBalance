#include "UfoPawn.h"

#include "Components/BoxComponent.h"
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

	AbsorbRangeVisualizer = CreateDefaultSubobject<UBoxComponent>(TEXT("AbsorbRangeVisualizer"));
	AbsorbRangeVisualizer->SetupAttachment(RootComponent);
	AbsorbRangeVisualizer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AbsorbRangeVisualizer->SetGenerateOverlapEvents(false);
	AbsorbRangeVisualizer->SetHiddenInGame(true);
	AbsorbRangeVisualizer->SetCanEverAffectNavigation(false);
	AbsorbRangeVisualizer->ShapeColor = FColor(80, 200, 255, 120);
	AbsorbRangeVisualizer->SetLineThickness(1.2f);
	AbsorbRangeVisualizer->SetIsVisualizationComponent(true);
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

	RefreshAbsorbRangeVisualizer();
}

void AUfoPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshAbsorbRangeVisualizer();
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

void AUfoPawn::SetAbsorbAmount(float NewAbsorbAmount)
{
	AbsorbAmount = FMath::Max(0.f, NewAbsorbAmount);
}

void AUfoPawn::AddAbsorbAmount(float DeltaAbsorbAmount)
{
	AbsorbAmount = FMath::Max(0.f, AbsorbAmount + DeltaAbsorbAmount);
}

void AUfoPawn::SetAbsorbSpeed(float NewAbsorbSpeed)
{
	AbsorbSpeed = FMath::Max(0.f, NewAbsorbSpeed);
}

void AUfoPawn::AddAbsorbSpeed(float DeltaAbsorbSpeed)
{
	AbsorbSpeed = FMath::Max(0.f, AbsorbSpeed + DeltaAbsorbSpeed);
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
		AbsorbDetectCooldownRemaining = 0.f;
		return;
	}

	if (!IsValid(AbsorbingActor))
	{
		AbsorbDetectCooldownRemaining -= DeltaTime;
		if (AbsorbDetectCooldownRemaining <= 0.f)
		{
			AbsorbingActor = FindAbsorbTarget();
			AbsorbDetectCooldownRemaining = FMath::Max(0.f, AbsorbDetectInterval);
			if (IsValid(AbsorbingActor))
			{
				PrepareActorForAbsorb(AbsorbingActor);
			}
		}
	}

	if (!IsValid(AbsorbingActor) || !IsValid(AbsorbingComponent))
	{
		return;
	}

	const FVector TargetLocation = GetAbsorbTargetLocation();
	const float EffectiveAbsorbRate = FMath::Max(0.f, AbsorbSpeed) * FMath::Max(0.f, AbsorbAmount);
	CurrentAbsorbElapsed += DeltaTime * EffectiveAbsorbRate;

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

void AUfoPawn::RefreshAbsorbRangeVisualizer()
{
	if (!AbsorbRangeVisualizer)
	{
		return;
	}

	const float Radius = FMath::Max(AbsorbRadius, 1.f);
	const float Depth = FMath::Max(AbsorbDepth, 0.f);
	AbsorbRangeVisualizer->SetBoxExtent(FVector(Radius, Radius, FMath::Max(Depth * 0.5f, 1.f)));
	AbsorbRangeVisualizer->SetRelativeLocation(FVector(0.f, 0.f, -Depth * 0.5f));
	AbsorbRangeVisualizer->SetHiddenInGame(true);
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
	const FVector AbsorbBoxCenter = UfoLocation + FVector(0.f, 0.f, -AbsorbDepth * 0.5f);
	const FVector AbsorbBoxExtent(
		FMath::Max(AbsorbRadius, 0.f),
		FMath::Max(AbsorbRadius, 0.f),
		FMath::Max(AbsorbDepth * 0.5f, 0.f)
	);

	AActor* BestActor = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (AActor* Candidate : Candidates)
	{
		if (!IsValid(Candidate) || Candidate == this)
		{
			continue;
		}

		FVector CandidateOrigin = FVector::ZeroVector;
		FVector CandidateExtent = FVector::ZeroVector;
		Candidate->GetActorBounds(true, CandidateOrigin, CandidateExtent);

		const bool bOverlapsAbsorbBox =
			FMath::Abs(CandidateOrigin.X - AbsorbBoxCenter.X) <= (CandidateExtent.X + AbsorbBoxExtent.X) &&
			FMath::Abs(CandidateOrigin.Y - AbsorbBoxCenter.Y) <= (CandidateExtent.Y + AbsorbBoxExtent.Y) &&
			FMath::Abs(CandidateOrigin.Z - AbsorbBoxCenter.Z) <= (CandidateExtent.Z + AbsorbBoxExtent.Z);
		if (!bOverlapsAbsorbBox)
		{
			continue;
		}

		const FVector ClosestPointOnBounds = UfoLocation.BoundToBox(CandidateOrigin - CandidateExtent, CandidateOrigin + CandidateExtent);
		const float DistanceSq = FVector::DistSquared(UfoLocation, ClosestPointOnBounds);
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
	AbsorbDetectCooldownRemaining = 0.f;
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

#if WITH_EDITOR
void AUfoPawn::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshAbsorbRangeVisualizer();
}
#endif

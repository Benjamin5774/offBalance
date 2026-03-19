#include "UfoPawn.h"

#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AUfoPawn::AUfoPawn()
{
	PrimaryActorTick.bCanEverTick = true;

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
}

void AUfoPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyMovement(DeltaTime);
	UpdateHoverHeight(DeltaTime);
	UpdateTilt(DeltaTime);
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

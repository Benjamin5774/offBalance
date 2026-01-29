#include "MrFallBall.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

AMrFallBall::AMrFallBall()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(RootComponent);

	// If you want physics later, enable Simulate Physics in BP instead.
	BallMesh->SetSimulatePhysics(false);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AMrFallBall::BeginPlay()
{
	Super::BeginPlay();
}

void AMrFallBall::SetGyroSteer(float InSteer)
{
	// Clamp to sane range
	GyroSteer = FMath::Clamp(InSteer, -1.f, 1.f);
}

void AMrFallBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Constant forward in actor's forward direction
	const FVector ForwardMove = GetActorForwardVector() * ForwardSpeed * DeltaTime;

	// Deadzone + sideways steer
	float S = GyroSteer;
	if (FMath::Abs(S) < SteerDeadzone) S = 0.f;

	const FVector RightMove = GetActorRightVector() * (S * SteerSpeed * DeltaTime);

	AddActorWorldOffset(ForwardMove + RightMove, true); // sweep = true (collides)
}

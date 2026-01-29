// Fill out your copyright notice in the Description page of Project Settings.

#include "MsFallBallMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UMsFallBallMovementComponent::UMsFallBallMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMsFallBallMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UPrimitiveComponent* Body = PhysicsBody;
	if (!Body || !Body->IsSimulatingPhysics())
	{
		PendingMovementInput = FVector2D::ZeroVector;
		return;
	}

	if (!IsGrounded())
	{
		PendingMovementInput = FVector2D::ZeroVector;
		return;
	}

	if (PendingMovementInput.IsNearlyZero())
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		PendingMovementInput = FVector2D::ZeroVector;
		return;
	}

	const FVector Direction = (Owner->GetActorForwardVector() * PendingMovementInput.Y)
		+ (Owner->GetActorRightVector() * PendingMovementInput.X);
	if (Direction.IsNearlyZero())
	{
		PendingMovementInput = FVector2D::ZeroVector;
		return;
	}

	Body->AddForce(Direction * Acceleration, NAME_None, true);
	PendingMovementInput = FVector2D::ZeroVector;
}

void UMsFallBallMovementComponent::AddMovementInput(FVector2D Vector)
{
	PendingMovementInput += Vector;
}

void UMsFallBallMovementComponent::SetPhysicsBody(UPrimitiveComponent* InPhysicsBody)
{
	PhysicsBody = InPhysicsBody;
}

bool UMsFallBallMovementComponent::IsGrounded() const
{
	if (!PhysicsBody)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector Origin = PhysicsBody->GetComponentLocation();
	const float Radius = PhysicsBody->Bounds.SphereRadius;
	const float TraceRadius = FMath::Max(5.f, Radius * 0.9f);
	const FVector End = Origin - FVector(0.f, 0.f, Radius + GroundCheckDistance);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(MsFallBallMovementGround), false, GetOwner());
	Params.AddIgnoredComponent(PhysicsBody);

	FHitResult Hit;
	return World->SweepSingleByChannel(
		Hit,
		Origin,
		End,
		FQuat::Identity,
		GroundTraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);
}

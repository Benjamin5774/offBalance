// Fill out your copyright notice in the Description page of Project Settings.

#include "MsFallBallMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
// 仅在 cpp 中引用，避免头文件循环依赖
#include "MsFallBall.h"

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

	// 地面摩擦：对水平速度施加反向阻力（与速度成正比）
	if (GroundFriction > 0.f)
	{
		const FVector Velocity = Body->GetPhysicsLinearVelocity();
		const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.f);
		if (!HorizontalVelocity.IsNearlyZero())
		{
			// 注意：这里使用 bAccelChange=true（按“加速度”施加），因此不要再乘质量，否则会把加速度放大导致“弹飞”。
			const FVector FrictionAccel = (-HorizontalVelocity) * GroundFriction;
			Body->AddForce(FrictionAccel, NAME_None, true);
		}
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

	// 推力：优先使用新变量 ThrustForce；若未设置则回退到旧变量 Acceleration（兼容旧数据）
	const float EffectiveThrust = (ThrustForce > 0.f) ? ThrustForce : Acceleration;
	Body->AddForce(Direction * EffectiveThrust, NAME_None, true);
	PendingMovementInput = FVector2D::ZeroVector;
}

void UMsFallBallMovementComponent::AddMovementInput(FVector2D Vector)
{
	PendingMovementInput += Vector;

	// 让“所有来源的 AddMovementInput（含 WASD/手柄/蓝图）”都能驱动摇摆输入，
	// 从而保证摄像机/反向摇摆对象在键鼠与手柄模式下表现一致。
	if (AMsFallBall* BallPawn = Cast<AMsFallBall>(GetOwner()))
	{
		const FVector2D Clamped(
			FMath::Clamp(PendingMovementInput.X, -1.f, 1.f),
			FMath::Clamp(PendingMovementInput.Y, -1.f, 1.f)
		);
		BallPawn->SetSwayInput(Clamped);
		BallPawn->SetAntiSwayInput(Clamped);
	}
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

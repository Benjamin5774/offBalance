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
		InputSamples.Reset();
		InputSamplesHeadIndex = 0;
		CurrentLaggedInput = FVector2D::ZeroVector;
		PendingMovementInput = FVector2D::ZeroVector;
		return;
	}

	if (!IsGrounded())
	{
		InputSamples.Reset();
		InputSamplesHeadIndex = 0;
		CurrentLaggedInput = FVector2D::ZeroVector;
		PendingMovementInput = FVector2D::ZeroVector;
		return;
	}

	// 采样本帧输入（并清空累计，保证“每帧一次”的输入语义）
	FVector2D RawInput(
		FMath::Clamp(PendingMovementInput.X, -1.f, 1.f),
		FMath::Clamp(PendingMovementInput.Y, -1.f, 1.f)
	);
	PendingMovementInput = FVector2D::ZeroVector;

	// 开启自动向前时，禁用原输入中的“向前”（Y > 0）部分，保留后退与左右输入。
	if (bEnableAutoForward)
	{
		RawInput.Y = FMath::Min(RawInput.Y, 0.f);
	}

	// 计算“生效输入”：可选延迟 + 松开后惯性衰减
	FVector2D EffectiveInput = RawInput;
	if (bSimulateInputLag && InputLagSeconds > 0.f)
	{
		UWorld* World = GetWorld();
		const float Now = World ? World->GetTimeSeconds() : 0.f;

		// 追加样本
		FMsFallBallInputSample Sample;
		Sample.TimeSeconds = Now;
		Sample.Input = RawInput;
		InputSamples.Add(Sample);

		// 清理过旧样本（保留延迟窗口外一点点缓冲）
		const float KeepAfter = 0.25f;
		const float MinTime = Now - (InputLagSeconds + KeepAfter);
		while (InputSamplesHeadIndex < InputSamples.Num() && InputSamples[InputSamplesHeadIndex].TimeSeconds < MinTime)
		{
			++InputSamplesHeadIndex;
		}
		// 偶尔收缩数组，避免无限增长
		if (InputSamplesHeadIndex > 64)
		{
			InputSamples.RemoveAt(0, InputSamplesHeadIndex, false);
			InputSamplesHeadIndex = 0;
		}

		const float TargetTime = Now - InputLagSeconds;
		// 找到 <= TargetTime 的最后一个样本
		FVector2D Delayed = FVector2D::ZeroVector;
		if (InputSamples.Num() > 0 && InputSamplesHeadIndex < InputSamples.Num())
		{
			if (TargetTime >= InputSamples[InputSamplesHeadIndex].TimeSeconds)
			{
				int32 Index = InputSamplesHeadIndex;
				while (Index + 1 < InputSamples.Num() && InputSamples[Index + 1].TimeSeconds <= TargetTime)
				{
					++Index;
				}
				Delayed = InputSamples[Index].Input;
			}
		}

		// 惯性：当延迟后的输入归零时，平滑衰减到 0
		if (!Delayed.IsNearlyZero())
		{
			CurrentLaggedInput = Delayed;
		}
		else if (InputInertiaSeconds > 0.f)
		{
			// 让“约 InputInertiaSeconds 内基本停下”
			const float InterpSpeed = 3.f / FMath::Max(InputInertiaSeconds, KINDA_SMALL_NUMBER);
			CurrentLaggedInput.X = FMath::FInterpTo(CurrentLaggedInput.X, 0.f, DeltaTime, InterpSpeed);
			CurrentLaggedInput.Y = FMath::FInterpTo(CurrentLaggedInput.Y, 0.f, DeltaTime, InterpSpeed);
		}
		else
		{
			CurrentLaggedInput = FVector2D::ZeroVector;
		}

		EffectiveInput = CurrentLaggedInput;
	}
	else
	{
		// 未启用顿感：清空状态，避免切换开关时残留
		InputSamples.Reset();
		InputSamplesHeadIndex = 0;
		CurrentLaggedInput = RawInput;
		EffectiveInput = RawInput;
	}

	// 同步摇摆输入：跟随“生效输入”，保证延迟/惯性对摄像机与 AntiSway 同样有效
	if (AMsFallBall* BallPawn = Cast<AMsFallBall>(GetOwner()))
	{
		BallPawn->SetSwayInput(EffectiveInput);
		BallPawn->SetAntiSwayInput(EffectiveInput);
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

	const bool bApplyAutoForward = bEnableAutoForward && !FMath::IsNearlyZero(AutoForwardThrustForce);
	if (EffectiveInput.IsNearlyZero() && !bApplyAutoForward)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector Direction = (Owner->GetActorForwardVector() * EffectiveInput.Y)
		+ (Owner->GetActorRightVector() * EffectiveInput.X);

	// 推力：优先使用新变量 ThrustForce；若未设置则回退到旧变量 Acceleration（兼容旧数据）
	const float EffectiveThrust = (ThrustForce > 0.f) ? ThrustForce : Acceleration;
	if (!Direction.IsNearlyZero())
	{
		Body->AddForce(Direction * EffectiveThrust, NAME_None, true);
	}

	// 自动向前推力：独立于手动输入，开启后持续向前施加。
	if (bApplyAutoForward)
	{
		Body->AddForce(Owner->GetActorForwardVector() * AutoForwardThrustForce, NAME_None, true);
	}
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

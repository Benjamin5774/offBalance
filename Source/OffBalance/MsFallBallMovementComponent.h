// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "MsFallBallMovementComponent.generated.h"

class UPrimitiveComponent;

USTRUCT()
struct FMsFallBallInputSample
{
	GENERATED_BODY()

	UPROPERTY()
	float TimeSeconds = 0.f;

	UPROPERTY()
	FVector2D Input = FVector2D::ZeroVector;
};

/**
 * Movement logic for ball-like pawns: AddForce on a physics body, ground check, and Blueprint-callable AddMovementInput.
 * Assign PhysicsBody (e.g. the ball mesh) via SetPhysicsBody or in the editor.
 */
UCLASS(ClassGroup=(Movement), meta=(BlueprintSpawnableComponent))
class OFFBALANCE_API UMsFallBallMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMsFallBallMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Add movement input (e.g. from controller or Blueprint). X = right, Y = forward. Accumulates until applied in Tick. */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void AddMovementInput(FVector2D Vector);

	/** Set the primitive to apply force to (e.g. ball mesh with Simulate Physics). */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetPhysicsBody(UPrimitiveComponent* InPhysicsBody);

	/** 推力（力）大小：输入为 1 时施加的基础力（单位随 UE 物理，通常可理解为“推的力度”）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0", UIMin = "0", UIMax = "20000"))
	float ThrustForce = 2000.f;

	/** 自动向前开关。开启后会禁用原输入里的“向前”输入，并持续向前施加推力。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Auto Forward")
	bool bEnableAutoForward = false;

	/** 自动向前推力（按加速度施加）。设为 0 可临时关闭自动向前效果。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Auto Forward", meta = (ClampMin = "0", UIMin = "0", UIMax = "20000", EditCondition = "bEnableAutoForward"))
	float AutoForwardThrustForce = 1200.f;

	/**
	 * 地面摩擦系数：越大越“黏”，更快停下、更不打滑。
	 * 实现方式为在落地时对水平速度施加反向阻力（与速度成正比）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0", UIMin = "0", UIMax = "50"))
	float GroundFriction = 8.f;

	// 兼容旧变量名（如果你蓝图/关卡里已经改过 Acceleration，不会立刻失效）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (DeprecatedProperty, DeprecationMessage = "已改为 ThrustForce。Acceleration 仍保留用于兼容旧数据。"))
	float Acceleration = 2000.f;

	/** 勾选后启用“输入顿感”：延迟若干秒后输入才生效，并可设置松开后的惯性。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Input", meta = (DisplayName = "Simulate Input Lag"))
	bool bSimulateInputLag = false;

	/** 输入延迟（秒）：例如 0.1 表示 100ms 后输入才开始生效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Input", meta = (ClampMin = "0", UIMin = "0", UIMax = "0.5", EditCondition = "bSimulateInputLag"))
	float InputLagSeconds = 0.10f;

	/**
	 * 松开输入后的惯性时间（秒）：延迟后的输入从非 0 变成 0 时，会在该时间内逐渐衰减到 0。
	 * 设为 0 则立即停。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Input", meta = (ClampMin = "0", UIMin = "0", UIMax = "2", EditCondition = "bSimulateInputLag"))
	float InputInertiaSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundCheckDistance = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_WorldStatic;

	/** Physics body to apply force to. Can be set in editor or via SetPhysicsBody. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TObjectPtr<UPrimitiveComponent> PhysicsBody;

private:
	bool IsGrounded() const;

	FVector2D PendingMovementInput = FVector2D::ZeroVector;

	// 输入顿感（延迟/惯性）状态
	UPROPERTY(Transient)
	TArray<FMsFallBallInputSample> InputSamples;

	int32 InputSamplesHeadIndex = 0;
	FVector2D CurrentLaggedInput = FVector2D::ZeroVector;
};

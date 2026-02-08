// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "MsFallBall.generated.h"

class UCameraComponent;
class UMsFallBallMovementComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class OFFBALANCE_API AMsFallBall : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMsFallBall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Move input is expected in range [-1..1] for X (right) and Y (forward)
	UFUNCTION(BlueprintCallable, Category = "Move")
	void Move(const FVector2D& Input);

	/** Get the ball movement component so you can call AddMovementInput from Blueprint. */
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (DisplayName = "Get Ball Movement Component"))
	UMsFallBallMovementComponent* GetBallMovementComponent() const { return MovementComponent; }

	// 设置用于 WASD 摇摆的摄像机。传入球上挂载的 CameraComponent，在 BeginPlay 中调用。
	UFUNCTION(BlueprintCallable, Category = "Move", meta = (DisplayName = "SetSwatCamera", ToolTip = "传入球上挂载的摄像机组件，按 WASD 时会在对应方向旋转该摄像机的 Pitch/Roll。请在 BeginPlay 中调用。"))
	void SetSwayCamera(UCameraComponent* InCamera);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move",
		meta = (DisplayName = "AntiSwayActorTag", ToolTip = "在关卡中查找带有该 Tag 的 Actor 作为反向摇摆对象"))
	FName AntiSwayActorTag = TEXT("AntiSway");


	// 不再建议手动指定 AntiSway Actor：改为仅通过 Tag 自动检测（BeginPlay 会自动调用 SetAntiSwayFromTag）。
	UFUNCTION(BlueprintCallable, Category = "Move", meta = (DeprecatedFunction, DeprecationMessage = "已改为仅通过 AntiSwayActorTag 自动检测。请使用 SetAntiSwayFromTag（BeginPlay 已自动调用）。", DisplayName = "Set Anti Sway Actor"))
	void SetSwayActor(AActor* InActor);

	/** 在关卡中查找带 AntiSwayActorTag 的 Actor，设为反向摇摆对象。可在 BeginPlay 调用。 */
	UFUNCTION(BlueprintCallable, Category = "Move", meta = (DisplayName = "Set Anti Sway From Tag", ToolTip = "按 AntiSwayActorTag 在场景中查找 Actor 并设为反向摇摆对象。"))
	void SetAntiSwayFromTag();

	/** 设置反向摇摆的 XY 输入，供蓝图使用。X 正=右、Y 正=前；与摄像机摇摆同一套约定，但会取反实现翻转。 */
	UFUNCTION(BlueprintCallable, Category = "Move", meta = (DisplayName = "Set Anti Sway Input", ToolTip = "从蓝图传入 X/Y 作为反向摇摆输入；与 Set Sway Input 约定相同，效果取反。"))
	void SetAntiSwayInput(FVector2D InInput);

	/** 直接设置摇摆输入，供蓝图使用。X 正=右、Y 正=前；输入往右则屏幕往右。不依赖 WASD。 */
	UFUNCTION(BlueprintCallable, Category = "Move", meta = (DisplayName = "Set Sway Input", ToolTip = "从蓝图传入 X/Y。X 正=右、Y 正=前；X 往右则屏幕往右。"))
	void SetSwayInput(FVector2D InInput);

	// 摇摆角度（度）。输入 ±1 时应用此角度。允许为负数：负值=反向摇摆。X 影响左右(Roll)，Y 影响上下(Pitch)。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move", meta = (ClampMin = "-90", ClampMax = "90", UIMin = "-45", UIMax = "45", ToolTip = "摇摆角度（度）。允许为负数：负值=反向摇摆。X 影响左右(Roll)，Y 影响上下(Pitch)。"))
	float SwayValue = 5.f;

	// 勾选后仅 X（左右）产生摇摆，Y 不产生上下摇摆。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move", meta = (DisplayName = "Only X (Left/Right)", ToolTip = "勾选后只根据 X 左右摇摆，Y 不产生上下摇摆。"))
	bool bSwayOnlyAD = false;

	// 摇摆缓动速度。0 = 无缓动（立即跟随）；越大跟得越快，约 5–15 较为自然。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move", meta = (ClampMin = "0", UIMin = "0", UIMax = "30", ToolTip = "摇摆缓动速度。0 为无缓动，数值越大跟随越快。"))
	float SwaySmoothSpeed = 8.f;

	// 反向摇摆（AntiSway）角度（度）。输入 ±1 时应用此角度。允许为负数：负值=反向翻转方向。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move", meta = (ClampMin = "-90", ClampMax = "90", UIMin = "-45", UIMax = "45", ToolTip = "AntiSway 角度（度）。允许为负数：负值=反向翻转方向。X 影响左右(Roll)，Y 影响上下(Pitch)。"))
	float AntiSwayValue = 5.f;

	// 反向摇摆（AntiSway）缓动速度。0 = 无缓动（立即跟随）；越大跟得越快。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move", meta = (ClampMin = "0", UIMin = "0", UIMax = "30", ToolTip = "AntiSway 缓动速度。0 为无缓动，数值越大跟随越快。"))
	float AntiSwaySmoothSpeed = 8.f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UMsFallBallMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* BallMesh;

private:
	void UpdateCameraSway(float DeltaTime);
	void UpdateSwayActor(float DeltaTime);

	UCameraComponent* SwayCamera = nullptr;
	FRotator RestRelativeRotation;
	FVector2D SwayInput = FVector2D::ZeroVector;
	float CurrentSwayPitch = 0.f;
	float CurrentSwayRoll = 0.f;

	// 摇摆反向物体
	// 仅内部使用：不再在细节面板暴露选择框，改为使用 AntiSwayActorTag 自动查找。
	UPROPERTY(Transient)
	TObjectPtr<AActor> SwayActor = nullptr;
	FVector2D AntiSwayInput = FVector2D::ZeroVector;
	FRotator RestSwayActorRotation;
	bool bSwayActorRestCaptured = false;
	bool bSwayActorAttachedToThis = false;
	float CurrentActorPitch = 0.f;
	float CurrentActorRoll = 0.f;
};

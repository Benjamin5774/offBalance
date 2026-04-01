#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UfoPawn.generated.h"

class AActor;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
// 引入骨骼网格体组件的前向声明
class USkeletalMeshComponent;
struct FPropertyChangedEvent;

UCLASS()
class OFFBALANCE_API AUfoPawn : public APawn
{
	GENERATED_BODY()

public:
	AUfoPawn();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// === 核心新增：重写获取速度函数，让动画蓝图（如 AnimDynamics）能读到真实的移动速度 ===
	virtual FVector GetVelocity() const override;

	UFUNCTION(BlueprintCallable, Category = "UFO|Move")
	void Move(const FVector2D& Input);

	UFUNCTION(BlueprintCallable, Category = "UFO|Absorb")
	void SetAbsorbAmount(float NewAbsorbAmount);

	UFUNCTION(BlueprintCallable, Category = "UFO|Absorb")
	void AddAbsorbAmount(float DeltaAbsorbAmount);

	UFUNCTION(BlueprintCallable, Category = "UFO|Absorb")
	void SetAbsorbSpeed(float NewAbsorbSpeed);

	UFUNCTION(BlueprintCallable, Category = "UFO|Absorb")
	void AddAbsorbSpeed(float DeltaAbsorbSpeed);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UFO")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UFO")
	TObjectPtr<USceneComponent> VisualRoot;

	// 修改为骨骼网格体组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UFO")
	TObjectPtr<USkeletalMeshComponent> UfoMesh;

	// 仅用于编辑器里可视化吸附体积，游戏内隐藏且不参与碰撞。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UFO|Absorb")
	TObjectPtr<UBoxComponent> AbsorbRangeVisualizer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Move", meta = (ClampMin = "0"))
	float MoveSpeed = 900.f;

	// === 新增：速度插值速率，用于控制起步和刹车的惯性滑行感。数值越小越滑 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Move", meta = (ClampMin = "0.1"))
	float MovementInterpSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Input")
	bool bEnableWASDInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Move")
	float Height = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Move", meta = (ClampMin = "0"))
	float HeightFollowSpeed = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Sway", meta = (ClampMin = "0", UIMin = "0", UIMax = "45"))
	float MaxTiltRoll = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Sway", meta = (ClampMin = "0", UIMin = "0", UIMax = "45"))
	float MaxTiltPitch = 7.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Sway", meta = (ClampMin = "0", UIMin = "0", UIMax = "30"))
	float TiltInterpSpeed = 7.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb")
	bool bEnableAutoAbsorb = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb")
	FName GrabbableActorTag = TEXT("UFOGrabbable");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb", meta = (ClampMin = "0"))
	float AbsorbRadius = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb", meta = (ClampMin = "0"))
	float AbsorbDepth = 500.f;

	// 吸附检测间隔（秒）：每隔 X 秒才检测一次范围内可吸附目标。0 表示每帧都检测。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb", meta = (ClampMin = "0", UIMin = "0", UIMax = "5"))
	float AbsorbDetectInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb", meta = (ClampMin = "0"))
	float AbsorbSpeed = 6.f;

	// 吸附量倍率：用于放大/缩小吸附推进量，1 为默认，>1 更强，0 为停用推进。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb", meta = (ClampMin = "0"))
	float AbsorbAmount = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb", meta = (ClampMin = "0.01"))
	float AbsorbDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb", meta = (ClampMin = "0"))
	float CollectDistance = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb")
	FVector AbsorbOffset = FVector(0.f, 0.f, -80.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Feedback", meta = (ClampMin = "1.0"))
	float CollectPulseScale = 1.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Feedback", meta = (ClampMin = "0"))
	float CollectPulseReturnSpeed = 8.f;

private:
	void RefreshAbsorbRangeVisualizer();
	void MoveForwardAxis(float Value);
	void MoveRightAxis(float Value);
	void ApplyMovement(float DeltaTime);
	void UpdateHoverHeight(float DeltaTime);
	void UpdateTilt(float DeltaTime);
	void UpdateAbsorb(float DeltaTime);
	void UpdateCollectPulse(float DeltaTime);
	AActor* FindAbsorbTarget() const;
	FVector GetAbsorbTargetLocation() const;
	void PrepareActorForAbsorb(AActor* ActorToPrepare);
	void CompleteAbsorb();
	USceneComponent* FindBestAbsorbComponent(AActor* ActorToPrepare) const;
	FVector GetCurrentAbsorbingLocation() const;
	void SetCurrentAbsorbingLocation(const FVector& NewLocation);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	FVector StartLocation = FVector::ZeroVector;
	FVector2D CurrentMoveInput = FVector2D::ZeroVector;

	// === 新增：当前的实际移动速度，配合 C++ 逻辑计算真实加速度 ===
	FVector CurrentVelocity = FVector::ZeroVector;

	float ForwardAxisValue = 0.f;
	float RightAxisValue = 0.f;
	float CurrentTiltPitch = 0.f;
	float CurrentTiltRoll = 0.f;
	float CurrentVisualScaleMultiplier = 1.f;
	FVector RestVisualScale = FVector::OneVector;
	FVector AbsorbStartLocation = FVector::ZeroVector;
	float CurrentAbsorbElapsed = 0.f;
	float AbsorbDetectCooldownRemaining = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<AActor> AbsorbingActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> AbsorbingComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> AbsorbingPhysicsComponent = nullptr;
};
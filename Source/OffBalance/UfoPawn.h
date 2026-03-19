#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UfoPawn.generated.h"

class AActor;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class OFFBALANCE_API AUfoPawn : public APawn
{
	GENERATED_BODY()

public:
	AUfoPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "UFO|Move")
	void Move(const FVector2D& Input);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UFO")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UFO")
	TObjectPtr<USceneComponent> VisualRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UFO")
	TObjectPtr<UStaticMeshComponent> UfoMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Move", meta = (ClampMin = "0"))
	float MoveSpeed = 900.f;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UFO|Absorb", meta = (ClampMin = "0"))
	float AbsorbSpeed = 6.f;

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

	FVector StartLocation = FVector::ZeroVector;
	FVector2D CurrentMoveInput = FVector2D::ZeroVector;
	float ForwardAxisValue = 0.f;
	float RightAxisValue = 0.f;
	float CurrentTiltPitch = 0.f;
	float CurrentTiltRoll = 0.f;
	float CurrentVisualScaleMultiplier = 1.f;
	FVector RestVisualScale = FVector::OneVector;
	FVector AbsorbStartLocation = FVector::ZeroVector;
	float CurrentAbsorbElapsed = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<AActor> AbsorbingActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> AbsorbingComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> AbsorbingPhysicsComponent = nullptr;
};

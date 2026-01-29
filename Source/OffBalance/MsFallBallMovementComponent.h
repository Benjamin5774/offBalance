// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "MsFallBallMovementComponent.generated.h"

class UPrimitiveComponent;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Acceleration = 2000.f;

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
};

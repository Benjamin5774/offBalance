// Fill out your copyright notice in the Description page of Project Settings.


#include "MsFallBall.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MsFallBallMovementComponent.h"

// Sets default values
AMsFallBall::AMsFallBall()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(RootComponent);
	BallMesh->SetSimulatePhysics(true);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	MovementComponent = CreateDefaultSubobject<UMsFallBallMovementComponent>(TEXT("Movement"));
}

// Called when the game starts or when spawned
void AMsFallBall::BeginPlay()
{
	Super::BeginPlay();
	if (MovementComponent && BallMesh)
	{
		MovementComponent->SetPhysicsBody(BallMesh);
	}
}

// Called every frame
void AMsFallBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCameraSway(DeltaTime);
	UpdateSwayActor(DeltaTime);
}

// Called to bind functionality to input
void AMsFallBall::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMsFallBall::Move(const FVector2D& Input)
{
	SetSwayInput(Input);
	SetAntiSwayInput(Input);
	if (MovementComponent)
	{
		MovementComponent->AddMovementInput(Input);
	}
}

void AMsFallBall::SetSwayCamera(UCameraComponent* InCamera)
{
	SwayCamera = InCamera;
	if (SwayCamera)
	{
		RestRelativeRotation = SwayCamera->GetRelativeRotation();
		CurrentSwayPitch = 0.f;
		CurrentSwayRoll = 0.f;
	}
}

void AMsFallBall::SetSwayInput(FVector2D InInput)
{
	SwayInput = InInput;
}

void AMsFallBall::SetAntiSwayInput(FVector2D InInput)
{
	AntiSwayInput = InInput;
}

void AMsFallBall::SetAntiSwayFromTag()
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsWithTag(this, AntiSwayActorTag, Found);
	if (Found.Num() > 0 && Found[0])
	{
		SetSwayActor(Found[0]);
	}
}

void AMsFallBall::SetSwayActor(AActor* InActor)
{
	SwayActor = InActor;
	bSwayActorRestCaptured = false;
	if (SwayActor && IsValid(SwayActor))
	{
		USceneComponent* ActorRootComp = SwayActor->GetRootComponent();
		bSwayActorAttachedToThis = (SwayActor->GetAttachParentActor() == this);
		RestSwayActorRotation = bSwayActorAttachedToThis && ActorRootComp
			? ActorRootComp->GetRelativeRotation()
			: SwayActor->GetActorRotation();
		CurrentActorPitch = 0.f;
		CurrentActorRoll = 0.f;
		bSwayActorRestCaptured = true;
	}
}

void AMsFallBall::UpdateCameraSway(float DeltaTime)
{
	if (!SwayCamera)
	{
		return;
	}
	// X 往右 -> Roll 正（屏幕往右）；Y 往前往上 -> Pitch 正。bSwayOnlyAD 为 true 时仅 X 左右生效。
	const float TargetPitch = bSwayOnlyAD ? 0.f : (SwayInput.Y * SwayValue);
	const float TargetRoll = SwayInput.X * SwayValue;

	if (SwaySmoothSpeed <= 0.f)
	{
		CurrentSwayPitch = TargetPitch;
		CurrentSwayRoll = TargetRoll;
	}
	else
	{
		CurrentSwayPitch = FMath::FInterpTo(CurrentSwayPitch, TargetPitch, DeltaTime, SwaySmoothSpeed);
		CurrentSwayRoll = FMath::FInterpTo(CurrentSwayRoll, TargetRoll, DeltaTime, SwaySmoothSpeed);
	}
	SwayCamera->SetRelativeRotation(RestRelativeRotation + FRotator(CurrentSwayPitch, 0.f, CurrentSwayRoll));
}

void AMsFallBall::UpdateSwayActor(float DeltaTime)
{
	if (!SwayActor || !IsValid(SwayActor))
	{
		bSwayActorRestCaptured = false;
		return;
	}

	USceneComponent* ActorRoot = SwayActor->GetRootComponent();
	if (!ActorRoot)
	{
		return;
	}

	// 首次有效时捕获“静止”旋转
	if (!bSwayActorRestCaptured)
	{
		bSwayActorAttachedToThis = (SwayActor->GetAttachParentActor() == this);
		RestSwayActorRotation = bSwayActorAttachedToThis
			? ActorRoot->GetRelativeRotation()
			: SwayActor->GetActorRotation();
		CurrentActorPitch = 0.f;
		CurrentActorRoll = 0.f;
		bSwayActorRestCaptured = true;
	}

	// 使用 AntiSwayInput，取反实现翻转；bSwayOnlyAD 时仅 X
	const float TargetPitch = bSwayOnlyAD ? 0.f : (-AntiSwayInput.Y * SwayValue);
	const float TargetRoll = -AntiSwayInput.X * SwayValue;

	if (SwaySmoothSpeed <= 0.f)
	{
		CurrentActorPitch = TargetPitch;
		CurrentActorRoll = TargetRoll;
	}
	else
	{
		CurrentActorPitch = FMath::FInterpTo(CurrentActorPitch, TargetPitch, DeltaTime, SwaySmoothSpeed);
		CurrentActorRoll = FMath::FInterpTo(CurrentActorRoll, TargetRoll, DeltaTime, SwaySmoothSpeed);
	}

	const FRotator Delta = FRotator(CurrentActorPitch, 0.f, CurrentActorRoll);
	if (bSwayActorAttachedToThis)
	{
		ActorRoot->SetRelativeRotation(RestSwayActorRotation + Delta);
	}
	else
	{
		SwayActor->SetActorRotation(RestSwayActorRotation + Delta);
	}
}


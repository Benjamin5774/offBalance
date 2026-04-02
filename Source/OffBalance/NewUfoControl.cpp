#include "NewUfoControl.h"

#include "InputCoreTypes.h"
#include "UfoPawn.h"

void ANewUfoControl::SetMoveInput(const FVector2D& Input)
{
	MoveInput.X = FMath::Clamp(Input.X, -1.f, 1.f);
	MoveInput.Y = FMath::Clamp(Input.Y, -1.f, 1.f);
}

void ANewUfoControl::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	AUfoPawn* UfoPawn = Cast<AUfoPawn>(GetPawn());
	if (!UfoPawn)
	{
		return;
	}

	FVector2D FinalInput = MoveInput;
	if (bReadKeyboardInput)
	{
		FinalInput += GetKeyboardMoveInput();
	}

	FinalInput.X = FMath::Clamp(FinalInput.X, -1.f, 1.f);
	FinalInput.Y = FMath::Clamp(FinalInput.Y, -1.f, 1.f);

	if (bNormalizeCombinedInput && FinalInput.SizeSquared() > 1.f)
	{
		FinalInput.Normalize();
	}

	UfoPawn->Move(FinalInput);
}

FVector2D ANewUfoControl::GetKeyboardMoveInput() const
{
	float Right = 0.f;
	float Forward = 0.f;

	if (IsInputKeyDown(EKeys::D))
	{
		Right += 1.f;
	}
	if (IsInputKeyDown(EKeys::A))
	{
		Right -= 1.f;
	}
	if (IsInputKeyDown(EKeys::W))
	{
		Forward += 1.f;
	}
	if (IsInputKeyDown(EKeys::S))
	{
		Forward -= 1.f;
	}

	if (bUseArrowKeys)
	{
		if (IsInputKeyDown(EKeys::Right))
		{
			Right += 1.f;
		}
		if (IsInputKeyDown(EKeys::Left))
		{
			Right -= 1.f;
		}
		if (IsInputKeyDown(EKeys::Up))
		{
			Forward += 1.f;
		}
		if (IsInputKeyDown(EKeys::Down))
		{
			Forward -= 1.f;
		}
	}

	return FVector2D(
		FMath::Clamp(Right, -1.f, 1.f),
		FMath::Clamp(Forward, -1.f, 1.f)
	);
}

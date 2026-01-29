#include "MsFallBallController.h"
#include "MsFallBall.h"

void AMsFallBallController::SetMoveInput(const FVector2D& Input)
{
	MoveInput = Input;
}

void AMsFallBallController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	AMsFallBall* BallPawn = Cast<AMsFallBall>(GetPawn());
	if (!BallPawn)
	{
		return;
	}

	BallPawn->Move(MoveInput);
}

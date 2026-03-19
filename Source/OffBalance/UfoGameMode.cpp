#include "UfoGameMode.h"

#include "UObject/ConstructorHelpers.h"
#include "UfoPawn.h"

AUfoGameMode::AUfoGameMode()
{
	DefaultPawnClass = AUfoPawn::StaticClass();

	// If a Blueprint pawn named BP_UFO exists at /Game/BP_UFO, prefer it automatically.
	static ConstructorHelpers::FClassFinder<APawn> UfoPawnBlueprintClass(TEXT("/Game/BP_UFO"));
	if (UfoPawnBlueprintClass.Succeeded() && UfoPawnBlueprintClass.Class != nullptr)
	{
		DefaultPawnClass = UfoPawnBlueprintClass.Class;
	}
}

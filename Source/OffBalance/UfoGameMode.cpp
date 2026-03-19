#include "UfoGameMode.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "OffBalanceGameOverWidget.h"
#include "OffBalanceGameplayWidget.h"
#include "OffBalanceScoreSaveGame.h"
#include "UObject/ConstructorHelpers.h"
#include "UfoPawn.h"

AUfoGameMode::AUfoGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AUfoPawn::StaticClass();

	// If a Blueprint pawn named BP_UFO exists at /Game/BP_UFO, prefer it automatically.
	static ConstructorHelpers::FClassFinder<APawn> UfoPawnBlueprintClass(TEXT("/Game/BP_UFO"));
	if (UfoPawnBlueprintClass.Succeeded() && UfoPawnBlueprintClass.Class != nullptr)
	{
		DefaultPawnClass = UfoPawnBlueprintClass.Class;
	}
}

void AUfoGameMode::BeginPlay()
{
	Super::BeginPlay();

	CurrentScore = 0;
	RemainingTime = GameDurationSeconds;
	bGameOver = false;

	SetupGameplayInputMode();
	CreateInGameWidget();
	UpdateInGameWidget();
}

void AUfoGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bGameOver)
	{
		return;
	}

	RemainingTime = FMath::Max(0.f, RemainingTime - DeltaSeconds);
	UpdateInGameWidget();

	if (RemainingTime <= 0.f)
	{
		HandleGameOver();
	}
}

void AUfoGameMode::RegisterCollectedItem(int32 ScoreAmount)
{
	if (bGameOver)
	{
		return;
	}

	CurrentScore += FMath::Max(1, ScoreAmount);
	UpdateInGameWidget();
}

void AUfoGameMode::RestartCurrentGame()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (GameOverWidget)
	{
		GameOverWidget->RemoveFromParent();
		GameOverWidget = nullptr;
	}

	if (InGameWidget)
	{
		InGameWidget->RemoveFromParent();
		InGameWidget = nullptr;
	}

	SetupGameplayInputMode();
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, FName(*World->GetName()));
}

void AUfoGameMode::CreateInGameWidget()
{
	if (!InGameWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	InGameWidget = CreateWidget<UOffBalanceGameplayWidget>(PlayerController, InGameWidgetClass);
	if (InGameWidget)
	{
		InGameWidget->AddToViewport();
	}
}

void AUfoGameMode::UpdateInGameWidget()
{
	if (!InGameWidget)
	{
		return;
	}

	InGameWidget->UpdateScore(CurrentScore);
	InGameWidget->UpdateTimerText(RemainingTime);
}

void AUfoGameMode::HandleGameOver()
{
	if (bGameOver)
	{
		return;
	}

	bGameOver = true;
	RemainingTime = 0.f;
	UpdateInGameWidget();

	if (InGameWidget)
	{
		InGameWidget->RemoveFromParent();
	}

	const TArray<int32> TopScores = SaveAndGetTopScores();
	ShowGameOverWidget(TopScores);

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}

	UGameplayStatics::SetGamePaused(this, true);
}

void AUfoGameMode::ShowGameOverWidget(const TArray<int32>& TopScores)
{
	if (!GameOverWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	GameOverWidget = CreateWidget<UOffBalanceGameOverWidget>(PlayerController, GameOverWidgetClass);
	if (!GameOverWidget)
	{
		return;
	}

	GameOverWidget->UpdateScoreHistory(TopScores);
	GameOverWidget->OnReplayClicked.AddDynamic(this, &AUfoGameMode::HandleReplayClicked);
	GameOverWidget->AddToViewport(10);
}

TArray<int32> AUfoGameMode::SaveAndGetTopScores() const
{
	UOffBalanceScoreSaveGame* SaveGameObject = nullptr;

	if (UGameplayStatics::DoesSaveGameExist(ScoreSaveSlotName, 0))
	{
		SaveGameObject = Cast<UOffBalanceScoreSaveGame>(UGameplayStatics::LoadGameFromSlot(ScoreSaveSlotName, 0));
	}

	if (!SaveGameObject)
	{
		SaveGameObject = Cast<UOffBalanceScoreSaveGame>(UGameplayStatics::CreateSaveGameObject(UOffBalanceScoreSaveGame::StaticClass()));
	}

	if (!SaveGameObject)
	{
		return {};
	}

	SaveGameObject->TopScores.Add(CurrentScore);
	SaveGameObject->TopScores.Sort([](int32 Left, int32 Right)
	{
		return Left > Right;
	});

	if (SaveGameObject->TopScores.Num() > MaxSavedScores)
	{
		SaveGameObject->TopScores.SetNum(MaxSavedScores);
	}

	UGameplayStatics::SaveGameToSlot(SaveGameObject, ScoreSaveSlotName, 0);
	return SaveGameObject->TopScores;
}

void AUfoGameMode::SetupGameplayInputMode()
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}
}

void AUfoGameMode::HandleReplayClicked()
{
	RestartCurrentGame();
}

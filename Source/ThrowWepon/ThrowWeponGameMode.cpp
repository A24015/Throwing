#include "ThrowWeponGameMode.h"
#include "ThrowWeponGameState.h"

AThrowWeponGameMode::AThrowWeponGameMode()
{
	PlayerSpawnLocations[0] = FVector(0, 0, 100);
	PlayerSpawnLocations[1] = FVector(500, 0, 100);
	PlayerSpawnLocations[2] = FVector(0, 500, 100);
	PlayerSpawnLocations[3] = FVector(500, 500, 100);

	PlayerCount = 0;

	// 使用するGameStateを指定
	GameStateClass = AThrowWeponGameState::StaticClass();
}

void AThrowWeponGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// プレイヤーが参加したらカウントダウン開始
	if (!GetWorldTimerManager().IsTimerActive(CountdownTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			CountdownTimerHandle,
			this,
			&AThrowWeponGameMode::UpdateCountdown,
			1.0f,
			true
		);
	}
}

void AThrowWeponGameMode::UpdateCountdown()
{
	AThrowWeponGameState* ThrowGameState =
		GetGameState<AThrowWeponGameState>();

	if (!ThrowGameState)
	{
		return;
	}

	ThrowGameState->Countdown--;

	if (ThrowGameState->Countdown <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

		// START!を表示
		ThrowGameState->bShowStart = true;

		// 1秒後にSTART!を消す
		GetWorldTimerManager().SetTimer(
			StartTimerHandle,
			this,
			&AThrowWeponGameMode::HideStartText,
			1.0f,
			false
		);
	}
}

void AThrowWeponGameMode::UpdateGameTime()
{
	AThrowWeponGameState* ThrowGameState =
		GetGameState<AThrowWeponGameState>();

	if (!ThrowGameState)
	{
		return;
	}

	ThrowGameState->RemainingTime--;

	if (ThrowGameState->RemainingTime <= 0)
	{
		ThrowGameState->RemainingTime = 0;

		// 3分経過
		GetWorldTimerManager().ClearTimer(GameTimerHandle);

		// ここにゲーム終了処理を入れる
	}
}

void AThrowWeponGameMode::HideStartText()
{
	AThrowWeponGameState* ThrowGameState =
		GetGameState<AThrowWeponGameState>();

	if (!ThrowGameState)
	{
		return;
	}

	// START!を消す
	ThrowGameState->bShowStart = false;

	// ゲーム開始
	ThrowGameState->bGameStarted = true;

	// 3分タイマー開始
	GetWorldTimerManager().SetTimer(
		GameTimerHandle,
		this,
		&AThrowWeponGameMode::UpdateGameTime,
		1.0f,
		true
	);
}

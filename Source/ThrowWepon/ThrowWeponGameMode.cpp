#include "ThrowWeponGameMode.h"
#include "ThrowWeponGameState.h"
#include "ThrowWeponPlayerState.h"
// PlayerStartを探すためのもの
#include "EngineUtils.h"

// PlayerStartを使うためのもの
#include "GameFramework/PlayerStart.h"

// プレイヤーキャラクターを使うためのもの
#include "ThrowWeponCharacter.h"

AThrowWeponGameMode::AThrowWeponGameMode()
{
	PlayerSpawnLocations[0] = FVector(0, 0, 100);
	PlayerSpawnLocations[1] = FVector(500, 0, 100);
	PlayerSpawnLocations[2] = FVector(0, 500, 100);
	PlayerSpawnLocations[3] = FVector(500, 500, 100);

	PlayerCount = 0;

	// 使用するGameStateを指定

	GameStateClass = AThrowWeponGameState::StaticClass();
	// このゲームで使うPlayerStateを指定する
	PlayerStateClass = AThrowWeponPlayerState::StaticClass();
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

// 死亡したプレイヤーをリスポーンさせる処理
void AThrowWeponGameMode::RespawnPlayer(AController* PlayerController)
{
	// Controllerが存在するか確認する
	if (!PlayerController)
	{
		return;
	}

	// 現在操作しているCharacterを取得する
	AThrowWeponCharacter* OldCharacter =
		Cast<AThrowWeponCharacter>(PlayerController->GetPawn());

	// リスポーンする場所を保存する変数
	AActor* RespawnPoint = nullptr;

	// レベル内のPlayerStartを探す
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		// 見つかったPlayerStartをリスポーン場所にする
		RespawnPoint = *It;

		// 1つ見つけたのでループを終了する
		break;
	}

	// PlayerStartが見つからなかった場合は終了する
	if (!RespawnPoint)
	{
		return;
	}

	// 古いCharacterが存在するか確認する
	if (OldCharacter)
	{
		// 古いCharacterを削除する
		OldCharacter->Destroy();
	}

	// Characterを生成するときの設定
	FActorSpawnParameters SpawnParams;

	// Controllerを新しいCharacterの所有者にする
	SpawnParams.Owner = PlayerController;

	// 新しいCharacterを生成する
	AThrowWeponCharacter* NewCharacter =
		GetWorld()->SpawnActor<AThrowWeponCharacter>(
			AThrowWeponCharacter::StaticClass(),
			RespawnPoint->GetActorLocation(),
			RespawnPoint->GetActorRotation(),
			SpawnParams
		);

	// Characterを正常に生成できたか確認する
	if (NewCharacter)
	{
		// Controllerを新しいCharacterに接続する
		PlayerController->Possess(NewCharacter);
	}
}

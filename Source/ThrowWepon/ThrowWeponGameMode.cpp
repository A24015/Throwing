#include "ThrowWeponGameMode.h"
#include "ThrowWeponGameState.h"
#include "ThrowWeponPlayerState.h"
// PlayerStartを探すためのもの
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
// PlayerStartを使うためのもの
#include "GameFramework/PlayerStart.h"
#include "UObject/ConstructorHelpers.h"


// プレイヤーキャラクターを使うためのもの
#include "ThrowWeponCharacter.h"
// プレイヤーの一覧を取得するためのもの
#include "GameFramework/GameStateBase.h"

AThrowWeponGameMode::AThrowWeponGameMode()
{
	PlayerSpawnLocations[0] = FVector(0, 0, 100);
	PlayerSpawnLocations[1] = FVector(500, 0, 100);
	PlayerSpawnLocations[2] = FVector(0, 500, 100);
	PlayerSpawnLocations[3] = FVector(500, 500, 100);

	PlayerCount = 0;

	GameStateClass = AThrowWeponGameState::StaticClass();
	PlayerStateClass = AThrowWeponPlayerState::StaticClass();

	// リスポーンに使うキャラクターのBlueprintを読み込む
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
		TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter")
	);

	if (PlayerPawnBPClass.Class)
	{
		// 生成するキャラクターをBP_ThirdPersonCharacterに指定
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AThrowWeponGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 実際に動いている GameMode と、生成対象のクラスを確認
	UE_LOG(
		LogTemp,
		Error,
		TEXT("実行中GameMode: %s"),
		*GetClass()->GetPathName()
	);

	UE_LOG(
		LogTemp,
		Error,
		TEXT("DefaultPawnClass: %s"),
		*GetNameSafe(DefaultPawnClass.Get())
	);

	if (DefaultPawnClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("DefaultPawnClassが抽象クラスか: %s"),
			DefaultPawnClass->HasAnyClassFlags(CLASS_Abstract)
			? TEXT("YES")
			: TEXT("NO")
		);
	}

	UE_LOG(
		LogTemp,
		Error,
		TEXT("CharacterClass: %s"),
		*GetNameSafe(CharacterClass.Get())
	);

	// 現在のマップ名を取得
	FString CurrentMapName = GetWorld()->GetMapName();

	// ★ メインメニュー（Lvl_MainMenu）やロビー（Lvl_Lobby）にいる場合は、戦闘処理を実行せずに終了する
	if (CurrentMapName.Contains(TEXT("Lvl_MainMenu")) || CurrentMapName.Contains(TEXT("Lvl_Lobby")))
	{
		return;
	}

	// -------------------------------------------------------------
	// ↓ ここから下は本番ステージ（Lvl_Stage1）だけで実行される
	// -------------------------------------------------------------
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

void AThrowWeponGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// ★ プレイヤーが入った瞬間の自動タイマー開始は削除（ホストのボタン押し待ちにするため）
}

//// ★ ホストがUIのボタンを押した時に呼ばれる処理
//void AThrowWeponGameMode::StartMatch()
//{
//	// まだタイマーが動いていなければカウントダウン開始！
//	if (!GetWorldTimerManager().IsTimerActive(CountdownTimerHandle))
//	{
//		GetWorldTimerManager().SetTimer(
//			CountdownTimerHandle,
//			this,
//			&AThrowWeponGameMode::UpdateCountdown,
//			1.0f,
//			true
//		);
//	}
//}

void AThrowWeponGameMode::StartMatch()
{
	// 全員を本番ステージ（Lvl_Stage1）へ遷移させる
	// ※ "?listen" をつけることでホスト（サーバー）として新しいレベルを開きます
	if (UWorld* World = GetWorld())
	{
		// 正しいパス (/Game/ThirdPerson/Lvl_Stage1) に変更
		World->ServerTravel(TEXT("/Game/ThirdPerson/Lvl_Stage1?listen"));
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
	// 試合の状態を管理するGameStateを取得
	AThrowWeponGameState* ThrowGameState =
		GetGameState<AThrowWeponGameState>();

	if (!ThrowGameState)
	{
		return;
	}

	// 試合がすでに終了していたら何もしない
	if (ThrowGameState->bGameEnded)
	{
		GetWorldTimerManager().ClearTimer(GameTimerHandle);
		return;
	}

	// 残り時間を1秒減らす
	ThrowGameState->RemainingTime--;

	// 残り時間が0以下になったら試合終了
	if (ThrowGameState->RemainingTime <= 0)
	{
		// 残り時間を0に固定
		ThrowGameState->RemainingTime = 0;

		// 3分タイマーを停止
		GetWorldTimerManager().ClearTimer(GameTimerHandle);

		// 試合終了処理を呼び出す
		ThrowGameState->EndMatch();

		// 順位を計算
		CreateResult();
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

	// 試合開始時に武器を 4 個ランダム配置する
	int32 InitialWeaponCount = 4;
	for (int32 i = 0; i < InitialWeaponCount; ++i)
	{
		SpawnWeaponWithTags();
	}
}

// 死亡したプレイヤーをリスポーンさせる処理
void AThrowWeponGameMode::RespawnPlayer(AController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (APawn* OldPawn = PlayerController->GetPawn())
	{
		PlayerController->UnPossess();
		OldPawn->Destroy();
	}

	// UE標準の方法でプレイヤーを復活させる
	RestartPlayer(PlayerController);
}

void AThrowWeponGameMode::SpawnWeaponWithTags()
{
	// 1. Tag (Weapon_Active) filtering
	TArray<TSubclassOf<AActor>> ValidWeaponClasses;

	for (TSubclassOf<AActor> WeaponClass : AllWeaponClasses)
	{
		if (!WeaponClass) continue;

		AActor* DefaultActor = WeaponClass->GetDefaultObject<AActor>();
		if (DefaultActor && DefaultActor->Tags.Contains(ActiveWeaponTag))
		{
			ValidWeaponClasses.Add(WeaponClass);
		}
	}

	if (ValidWeaponClasses.Num() == 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("[ERROR] No Weapon Class found! Add Tag '%s' to Weapon BP."), *ActiveWeaponTag.ToString()));
		return;
	}

	// 2. Tag (Spawn_Weapon) points search
	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnPointTag, FoundPoints);

	if (FoundPoints.Num() == 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("[ERROR] No Spawn Point found! Add Tag '%s' to SpwanTarget."), *SpawnPointTag.ToString()));
		return;
	}

	// Check empty points
	TArray<AActor*> AvailablePoints;
	for (AActor* Point : FoundPoints)
	{
		if (!Point) continue;

		// SpwanTarget 自身を判定から除外して自爆判定を回避
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Point);

		// 床の巻き込み防止のため少し上(Z+50)で半径50cmの判定を行う
		FCollisionShape Sphere = FCollisionShape::MakeSphere(50.0f);
		FVector CheckLocation = Point->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

		bool bIsOccupied = GetWorld()->OverlapAnyTestByChannel(
			CheckLocation,
			FQuat::Identity,
			ECC_WorldDynamic,
			Sphere,
			Params
		);

		if (!bIsOccupied)
		{
			AvailablePoints.Add(Point);
		}
	}

	if (AvailablePoints.Num() == 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[ERROR] All points are OCCUPIED or blocked by collision!"));
		return;
	}

	// 3. Spawn
	int32 PointIndex = FMath::RandRange(0, AvailablePoints.Num() - 1);
	int32 WeaponIndex = FMath::RandRange(0, ValidWeaponClasses.Num() - 1);

	AActor* SelectedPoint = AvailablePoints[PointIndex];
	TSubclassOf<AActor> SelectedWeaponClass = ValidWeaponClasses[WeaponIndex];

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		SelectedWeaponClass,
		SelectedPoint->GetActorTransform(),
		SpawnParams
	);

	if (SpawnedActor)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("[SUCCESS] Weapon Spawned Successfully!"));
	}
}

// ★ プレイヤー出現時に呼び出され、「被らないランダムな PlayerStart」を返す処理
AActor* AThrowWeponGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// 1. マップ内の全 PlayerStart を取得
	TArray<AActor*> AllSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), AllSpawnPoints);

	// 2. まだ使われていない PlayerStart だけをフィルタリング
	TArray<AActor*> UnusedSpawnPoints;
	for (AActor* SpawnPoint : AllSpawnPoints)
	{
		if (SpawnPoint && !UsedPlayerStarts.Contains(SpawnPoint))
		{
			UnusedSpawnPoints.Add(SpawnPoint);
		}
	}

	// 3. 未使用のポイントがあればそこからランダム選択
	if (UnusedSpawnPoints.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, UnusedSpawnPoints.Num() - 1);
		AActor* SelectedPoint = UnusedSpawnPoints[RandomIndex];

		// 使用済みリストに登録して返す
		UsedPlayerStarts.Add(SelectedPoint);
		return SelectedPoint;
	}

	// もし人数が PlayerStart の数を超えて使い切った場合は、通常のランダム処理（親クラス）にフォールバック
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AThrowWeponGameMode::CreateResult()
{
	// サーバーだけが順位を計算する
	if (!HasAuthority())
	{
		return;
	}

	// GameStateを取得
	AThrowWeponGameState* ThrowGameState =
		GetGameState<AThrowWeponGameState>();

	if (!ThrowGameState)
	{
		return;
	}

	// 前回のリザルトを削除
	ResultData.Empty();

	// PlayerStateを入れる配列
	TArray<AThrowWeponPlayerState*> Players;

	// 全プレイヤーを取得
	for (APlayerState* BasePlayerState : ThrowGameState->PlayerArray)
	{
		AThrowWeponPlayerState* PlayerState =
			Cast<AThrowWeponPlayerState>(BasePlayerState);

		if (PlayerState)
		{
			Players.Add(PlayerState);
		}
	}

	// スコアが高い順に並べる
	Players.Sort(
		[](const AThrowWeponPlayerState& A,
			const AThrowWeponPlayerState& B)
		{
			return A.GetResultScore() > B.GetResultScore();
		}
	);

	// 順位を作る
	for (int32 i = 0; i < Players.Num(); ++i)
	{
		FResultData NewResult;

		// 順位
		NewResult.Rank = i + 1;

		// プレイヤー名
		NewResult.PlayerName = Players[i]->PlayerName;

		// キル数
		NewResult.KillCount = Players[i]->KillCount;

		// デス数
		NewResult.DeathCount = Players[i]->DeathCount;

		// スコア
		NewResult.Score = Players[i]->GetResultScore();

		// リザルト配列に追加
		ResultData.Add(NewResult);
	}
}
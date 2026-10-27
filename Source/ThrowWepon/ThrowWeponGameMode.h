#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ThrowWeponGameMode.generated.h"

class AThrowWeponCharacter;

UCLASS()
class THROWWEPON_API AThrowWeponGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AThrowWeponGameMode();

	// 死亡したプレイヤーをリスポーンさせる処理
	void RespawnPlayer(AController* PlayerController);

	// 出現候補となる武器クラスの全リスト（プロジェクト内の武器BPを登録しておく）
	UPROPERTY(EditDefaultsOnly, Category = "Item Spawner")
	TArray<TSubclassOf<AActor>> AllWeaponClasses;

	// 今回使用する「武器のタグ」（デフォルトで "Weapon_Active"）
	UPROPERTY(EditAnywhere, Category = "Item Spawner")
	FName ActiveWeaponTag = FName("Weapon_Active");

	// スポーン場所のタグ（デフォルトで "Spawn_Weapon"）
	UPROPERTY(EditAnywhere, Category = "Item Spawner")
	FName SpawnPointTag = FName("Spawn_Weapon");

	// タグを判定してランダム出現させる処理
	void SpawnWeaponWithTags();

	// ホストがボタンを押した時に呼ぶゲーム開始関数
	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	void StartMatch();

	FVector PlayerSpawnLocations[4];

	int32 PlayerCount;

	// カウントダウン用
	FTimerHandle CountdownTimerHandle;

	// 3分タイマー用
	FTimerHandle GameTimerHandle;

	// カウントダウン処理
	void UpdateCountdown();

	// 残り時間の処理
	void UpdateGameTime();

	FTimerHandle StartTimerHandle;

	void HideStartText();


	// リザルト処理を行う関数
	void CreateResult();

	// すでに使われた PlayerStart を記録しておく配列
	UPROPERTY()
	TArray<AActor*> UsedPlayerStarts;
	// これはリスポーン時に生成するCharacterのクラス
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	TSubclassOf<AThrowWeponCharacter> CharacterClass;

protected:

	virtual void PostLogin(APlayerController* NewPlayer) override;

	// プレイヤーの初期出現位置を決定するUE標準関数のオーバーライド
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void BeginPlay() override;
private:

	
};
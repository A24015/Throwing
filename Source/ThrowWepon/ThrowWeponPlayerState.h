// Unreal Engineの基本機能を使うためのもの
#include "CoreMinimal.h"

// PlayerStateを使うためのもの
#include "GameFramework/PlayerState.h"

// Unreal Engineのクラスに必要な処理
#include "ThrowWeponPlayerState.generated.h"

// Unreal Engineのクラスとして登録するためのもの
UCLASS()

// プレイヤーごとの戦績を保存するPlayerStateクラス
class THROWWEPON_API AThrowWeponPlayerState : public APlayerState
{
	// Unreal Engineのクラスに必要な処理
	GENERATED_BODY()

public:

	// PlayerStateが作られたときに呼ばれる処理
	AThrowWeponPlayerState();

	// 死亡回数を保存する変数
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 DeathCount;

	// 死亡回数を1増やす処理
	void AddDeath();

	// キル数を保存する変数
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 KillCount;

	// キル数を1増やす処理
	void AddKill();

	// プレイヤーの名前を保存する変数
	UPROPERTY(Replicated, BlueprintReadOnly)
	FString PlayerName;

	// プレイヤーの名前を設定する処理
	void SetPlayerName(const FString& NewName);

	// ネットワークで同期する変数を登録する処理
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;
};
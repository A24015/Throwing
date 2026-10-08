// 自分で作ったPlayerStateクラスを使うためのもの
#include "ThrowWeponPlayerState.h"

// Replicatedをネットワーク同期させるためのもの
#include "Net/UnrealNetwork.h"

// PlayerStateが作られたときに呼ばれる処理
AThrowWeponPlayerState::AThrowWeponPlayerState()
{
	// 最初の死亡回数を0にする
	DeathCount = 0;
	// 最初のキル数を0にする
	KillCount = 0;
}

// 死亡回数を1増やす処理
void AThrowWeponPlayerState::AddDeath()
{
	// 死亡回数を1増やす
	DeathCount++;
}
void AThrowWeponPlayerState::AddKill()
{
	// キル数を1増やす
	KillCount++;
}

// ネットワークで同期する変数を登録する処理
void AThrowWeponPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	// 親クラスの同期処理を呼ぶ
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DeathCountをネットワーク同期する
	DOREPLIFETIME(AThrowWeponPlayerState, DeathCount);
	// KillCountをネットワーク同期する
	DOREPLIFETIME(AThrowWeponPlayerState, KillCount);
	// PlayerNameをネットワーク同期する
	DOREPLIFETIME(AThrowWeponPlayerState, PlayerName);
}

// プレイヤーの名前を設定する処理
void AThrowWeponPlayerState::SetPlayerName(const FString& NewName)
{
	// 受け取った名前をPlayerName変数に保存する
	PlayerName = NewName;
}
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ThrowWeponGameMode.generated.h"

UCLASS()
class THROWWEPON_API AThrowWeponGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AThrowWeponGameMode();

protected:

	virtual void PostLogin(APlayerController* NewPlayer) override;

private:

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
};
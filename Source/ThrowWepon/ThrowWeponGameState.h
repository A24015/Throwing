// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ThrowWeponGameState.generated.h"

/**
 * 
 */
UCLASS()
class THROWWEPON_API AThrowWeponGameState : public AGameStateBase
{
	GENERATED_BODY()
	

public:

	AThrowWeponGameState();

	UPROPERTY(Replicated,BlueprintReadOnly)
	int32 Countdown;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 RemainingTime;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bGameStarted;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bShowStart;

	// 試合終了状態を保存する変数
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
	bool bGameEnded = false;

	// 試合終了処理
	UFUNCTION(BlueprintCallable, Category = "Game")
	void EndMatch();
	// 試合終了状態を保存する変数
	

	// キル数を保存する変数
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 KillCount;

	// 死亡回数を保存する変数
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 DeathCount;

	// キル数を1増やす処理
	void AddKill();

	// 死亡回数を1増やす処理
	void AddDeath();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	)const override;
};




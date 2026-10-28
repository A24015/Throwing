// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowWeponGameState.h"
#include "Net/UnrealNetwork.h"


AThrowWeponGameState::AThrowWeponGameState()
{
	Countdown = 3;

	RemainingTime = 180;

	bGameStarted = false;

	bShowStart = false;

}

void AThrowWeponGameState::AddKill()
{
	KillCount++;
}

void AThrowWeponGameState::AddDeath()
{
	DeathCount++;
}

void AThrowWeponGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThrowWeponGameState, Countdown);
	DOREPLIFETIME(AThrowWeponGameState, RemainingTime);
	DOREPLIFETIME(AThrowWeponGameState, bGameStarted);
	DOREPLIFETIME(AThrowWeponGameState, bShowStart);
	// 試合終了状態を全プレイヤーに同期する
	DOREPLIFETIME(AThrowWeponGameState, bGameEnded);
}

void AThrowWeponGameState::EndMatch()
{
    // サーバー側だけで実行する
    if (!HasAuthority())
    {
        return;
    }

    // すでに試合が終わっていたら何もしない
    if (bGameEnded)
    {
        return;
    }

    // 試合終了にする
    bGameEnded = true;

    // 試合中ではない状態にする
    bGameStarted = false;

    // 残り時間を0にする
    RemainingTime = 0;
}

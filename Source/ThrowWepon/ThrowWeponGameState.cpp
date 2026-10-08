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
}

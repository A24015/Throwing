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

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	)const override;
};




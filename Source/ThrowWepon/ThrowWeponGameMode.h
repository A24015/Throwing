// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ThrowWeponGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AThrowWeponGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AThrowWeponGameMode();
};




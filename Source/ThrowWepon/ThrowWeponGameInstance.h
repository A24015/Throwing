#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ThrowWeponGameMode.h"
#include "ThrowWeponGameInstance.generated.h"

UCLASS()
class THROWWEPON_API UThrowWeponGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	// ‡Œ‹‰Ê‚ğ•Û‘¶‚·‚é”z—ñ
	UPROPERTY(BlueprintReadWrite, Category = "Result")
	TArray<FResultData> ResultData;
};
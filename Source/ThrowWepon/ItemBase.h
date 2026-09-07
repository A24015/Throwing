#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

UCLASS()
class THROWWEPON_API AItemBase : public AActor
{
	GENERATED_BODY()

public:
	AItemBase();

	// 重さ倍率（小さいほどよく飛び、大きいほど重い）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Stats")
	float WeightMultiplier = 1.0f;

	// 投げた時の基本ダメージ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Stats")
	float BaseDamage = 10.0f;

	// 持っている時の移動速度デバフ (0.2 = 20%遅くなる)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Stats")
	float MovementSpeedDebuff = 0.0f;

	// 実際の投擲力を計算するC++関数
	UFUNCTION(BlueprintCallable, Category = "Item Logic")
	float CalculateFinalLaunchForce(float RawChargeForce) const;
};
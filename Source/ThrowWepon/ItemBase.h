#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ItemBase.generated.h"

UCLASS()
class THROWWEPON_API AItemBase : public AActor
{
	GENERATED_BODY()

public:
	AItemBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* HitCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Stats")
	float BaseDamage = 10.0f;

	// 投げられている状態かどうかのフラグ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Logic")
	bool bIsThrown = false;

protected:
	virtual void BeginPlay() override;

	// 当たり判定（Hit）発生時に呼ばれるC++関数
	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

public:
	// 移動速度デバフ変数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Stats")
	float MovementSpeedDebuff = 0.0f;

	// 投擲力を計算する関数
	UFUNCTION(BlueprintCallable, Category = "Item Stats")
	float CalculateFinalLaunchForce(float RawChargeForce);
};
#include "ItemBase.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

// チャージされたパワーを「重さ」で割り算して最終的な力を出す
float AItemBase::CalculateFinalLaunchForce(float RawChargeForce) const
{
	if (WeightMultiplier <= 0.0f) return RawChargeForce;

	return RawChargeForce / WeightMultiplier;
}

#include "ItemBase.h"
#include "Kismet/GameplayStatics.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	// ルートコンポーネント（HitCollision）の初期化
	HitCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HitCollision"));
	RootComponent = HitCollision;
	HitCollision->SetSphereRadius(50.0f);

	// 物理ヒットイベントを発火させるための設定
	HitCollision->SetNotifyRigidBodyCollision(true); // BPの "Simulation Generates Hit Events" に相当

	// メッシュの初期化
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// OnComponentHit イベントに関数をバインド
	HitCollision->OnComponentHit.AddDynamic(this, &AItemBase::OnHit);
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
}

// 物理的にぶつかった瞬間の処理
void AItemBase::OnHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	// 投げられている最中、かつ当たったのが自分自身・所有者（投げた人）以外の場合
	if (bIsThrown && OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		// サーバー権限がある場合のみダメージを適用（マルチプレイ対策）
		if (HasAuthority())
		{
			UGameplayStatics::ApplyDamage(
				OtherActor,
				BaseDamage,
				GetInstigatorController(),
				this,
				UDamageType::StaticClass()
			);
		}

		// 一度当たったら投擲状態を解除
		bIsThrown = false;
	}
}

float AItemBase::CalculateFinalLaunchForce(float RawChargeForce)
{
	// チャージ力に応じた計算処理
	return RawChargeForce;
}
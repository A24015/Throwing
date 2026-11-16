
#include "ThrowWeponCharacter.h"
#include "ThrowWeponGameState.h"
#include "ThrowWeponGameMode.h"
#include "ThrowWeponPlayerState.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// ---------------------------------------------------------
// コンストラクタ（カメラとカプセルコンポーネントの初期化）
// ---------------------------------------------------------
AThrowWeponCharacter::AThrowWeponCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	// カプセルサイズの設定
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// カメラブーム（スプリングアーム）の生成と初期化
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// フォローカメラの生成と初期化
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->AirControl = 0.2f;                 // 空中で踏ん張りすぎない
	GetCharacterMovement()->BrakingDecelerationFalling = 0.0f; // 吹っ飛んだ勢いが空中で減衰しないようにする

	// 最初は生きている状態にする
	bIsDead = false;
	Health = MaxHealth;
}

// ---------------------------------------------------------
// インプット初期化・バインド処理
// ---------------------------------------------------------
void AThrowWeponCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AThrowWeponCharacter::DoJumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AThrowWeponCharacter::DoJumpEnd);
		}

		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AThrowWeponCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThrowWeponCharacter::Look);
		}

		if (MouseLookAction)
		{
			EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AThrowWeponCharacter::Look);
		}
	}

	// Kキーを押したら死亡処理を実行する
	PlayerInputComponent->BindKey(
		EKeys::K,
		IE_Pressed,
		this,
		&AThrowWeponCharacter::DieByButton
	);
}

void AThrowWeponCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AThrowWeponCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AThrowWeponCharacter::DoMove(float Right, float Forward)
{
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AThrowWeponCharacter::DoLook(float Yaw, float Pitch)
{
	if (Controller != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AThrowWeponCharacter::DoJumpStart()
{
	Jump();
}

void AThrowWeponCharacter::DoJumpEnd()
{
	StopJumping();
}

// ---------------------------------------------------------
// 死亡時のマルチキャスト処理
// ---------------------------------------------------------
void AThrowWeponCharacter::Multicast_OnDeath_Implementation()
{
	// 人間プレイヤーなら入力を無効化
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
	// AI（敵Bot）なら移動を停止
	else if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}
}

// ---------------------------------------------------------
// ダメージ & ノックバック処理
// ---------------------------------------------------------
float AThrowWeponCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	UE_LOG(LogTemp, Warning, TEXT("★★★ TakeDamage呼ばれた ★★★"));
	UE_LOG(LogTemp, Warning, TEXT("Damage = %f / HP = %f"), DamageAmount, Health);

	if (ActualDamage > 0.0f)
	{
		if (EventInstigator && EventInstigator != GetController())
		{
			LastInstigator = EventInstigator;
		}

		Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

		// 1. 被弾方向の計算
		FVector HitDirection = GetActorForwardVector() * -1.0f;
		if (DamageCauser)
		{
			HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
		}

		// 2. 慣性のリセット（奈落落下の時は落下速度を止めない）
		if (DamageCauser && GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
		}

		// 3. アイテムからノックバック基本値の取得
		float BaseKnockback = 800.0f;
		if (DamageCauser)
		{
			if (FProperty* Prop = DamageCauser->GetClass()->FindPropertyByName(TEXT("KnockbackForce")))
			{
				if (FNumericProperty* NumProp = CastField<FNumericProperty>(Prop))
				{
					void* ValuePtr = NumProp->ContainerPtrToValuePtr<void>(DamageCauser);
					BaseKnockback = static_cast<float>(NumProp->GetFloatingPointPropertyValue(ValuePtr));
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[Knockback Fix] DamageCauser: %s | BaseKnockback: %f"),
			DamageCauser ? *DamageCauser->GetName() : TEXT("NULL"), BaseKnockback);

		// プレイヤー操作以外のキャラ（敵Bot）は吹っ飛び3倍
		if (!IsPlayerControlled())
		{
			BaseKnockback *= 3.0f;
		}

		// 4. 生存 / 死亡によるノックバックと処理の分岐
		if (Health <= 0.0f)
		{
			if (!bIsDead)
			{
				bIsDead = true;

				AThrowWeponPlayerState* ThrowPlayerState = GetPlayerState<AThrowWeponPlayerState>();
				AThrowWeponGameMode* ThrowGameMode = GetWorld()->GetAuthGameMode<AThrowWeponGameMode>();

				if (ThrowPlayerState)
				{
					ThrowPlayerState->AddDeath();
				}

				if (EventInstigator)
				{
					AThrowWeponPlayerState* AttackerPlayerState = EventInstigator->GetPlayerState<AThrowWeponPlayerState>();
					if (AttackerPlayerState)
					{
						AttackerPlayerState->AddKill();
					}
				}

				LastInstigator = nullptr;

				// 攻撃（DamageCauserあり）で死んだ時だけトドメの特大吹っ飛びを実行する
				if (DamageCauser)
				{
					float FatalMultiplier = 4.5f;
					FVector FatalKnockback = (HitDirection + FVector(0.f, 0.f, 0.6f)).GetSafeNormal() * (BaseKnockback * FatalMultiplier);

					LaunchCharacter(FatalKnockback, true, true);
				}

				Multicast_OnDeath();

				AController* PlayerController = GetController();
				if (PlayerController && IsPlayerControlled())
				{
					FTimerHandle RespawnTimer;
					if (UWorld* World = GetWorld())
					{
						World->GetTimerManager().SetTimer(
							RespawnTimer,
							[ThrowGameMode, PlayerController]()
							{
								if (IsValid(ThrowGameMode) && IsValid(PlayerController))
								{
									ThrowGameMode->RespawnPlayer(PlayerController);
								}
							},
							3.0f,
							false
						);
					}
				}
			}
		}
		else
		{
			FVector KnockbackVelocity = (HitDirection + FVector(0.f, 0.f, 0.35f)).GetSafeNormal() * BaseKnockback;
			LaunchCharacter(KnockbackVelocity, true, true);
		}
	}

	return ActualDamage;
}

// ---------------------------------------------------------
// ライン判定処理（コンバット）
// ---------------------------------------------------------
bool AThrowWeponCharacter::PerformLineTrace(FHitResult& OutHitResult, float TraceDistance)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector Start;
	FRotator Rotation;

	if (FollowCamera)
	{
		Start = FollowCamera->GetComponentLocation();
		Rotation = FollowCamera->GetComponentRotation();
	}
	else
	{
		Start = GetActorLocation();
		Rotation = GetActorRotation();
	}

	FVector End = Start + (Rotation.Vector() * TraceDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = World->LineTraceSingleByChannel(
		OutHitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	if (bHit && OutHitResult.GetActor() != nullptr)
	{
		return true;
	}

	OutHitResult = FHitResult();
	return false;
}

void AThrowWeponCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Health 変数をネットワーク同期対象として登録
	DOREPLIFETIME(AThrowWeponCharacter, Health);
}

void AThrowWeponCharacter::FellOutOfWorld(const UDamageType& dmgType)
{
	// サーバー側で、生存中の場合は死亡処理を実行
	if (HasAuthority() && !bIsDead)
	{
		FDamageEvent DamageEvent;

		TakeDamage(
			Health > 0.0f ? Health : 100.0f,
			DamageEvent,
			LastInstigator,
			nullptr
		);
	}
}

// Kキーで自分にダメージを与える処理
void AThrowWeponCharacter::DieByButton()
{
	UE_LOG(LogTemp, Warning, TEXT("K入力：現在HP = %f"), Health);

	TakeDamage(
		Health + 1.0f,
		FDamageEvent(),
		GetController(),
		this
	);
}

void AThrowWeponCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// サーバー側で生存中、かつ高さ(Z)が -1000.0f 以下になったら奈落即死
	if (HasAuthority() && !bIsDead && GetActorLocation().Z < -1000.0f)
	{
		FDamageEvent DamageEvent;
		TakeDamage(
			Health > 0.0f ? Health : 100.0f,
			DamageEvent,
			LastInstigator,
			nullptr
		);
	}
}
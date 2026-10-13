#include "ThrowWeponCharacter.h"
#include"ThrowWeponGameState.h"
#include"ThrowWeponGameMode.h"
#include "ThrowWeponPlayerState.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h" // ノックバック同期用

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

	// 最初は生きている状態にする
	bIsDead = false;
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
// ダメージ & ノックバック処理
// ---------------------------------------------------------

void AThrowWeponCharacter::Multicast_OnDeath_Implementation()
{
	// 死亡したので入力を無効化（操作不能にする）
	DisableInput(Cast<APlayerController>(GetController()));

	// ※以前のラグドール化処理（SetSimulatePhysicsなど）は完全にカットし、
	// CharacterMovement による綺麗な同期状態を維持します
}

// ---------------------------------------------------------
// ダメージ & ノックバック処理
// ---------------------------------------------------------
float AThrowWeponCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.0f)
	{
		Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

		// --- 1. 被弾方向の計算 ---
		FVector HitDirection = GetActorForwardVector() * -1.0f; // 基本は後ろ方向
		if (DamageCauser)
		{
			// ボール（DamageCauser）から被弾者へのベクトル（吹き飛び方向）
			HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
		}

		// --- 2. 慣性のリセット（ヒットストップ効果） ---
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
		}

		// --- 3. 生存 / 死亡によるノックバックの分岐 ---
		if (Health <= 0.0f)
		{
			// まだ死亡処理をしていないか確認する
			if (!bIsDead)
			{
				// 死亡済みの状態にする
				bIsDead = true;

				// 自分のPlayerStateを取得する
				AThrowWeponPlayerState* ThrowPlayerState =
					GetPlayerState<AThrowWeponPlayerState>();

				// このゲームで使っているGameModeを取得する
				AThrowWeponGameMode* ThrowGameMode =
					GetWorld()->GetAuthGameMode<AThrowWeponGameMode>();

				// PlayerStateを取得できたか確認する
				if (ThrowPlayerState)
				{
					// 死亡回数を1増やす
					ThrowPlayerState->AddDeath();
				}

				// 攻撃したプレイヤーが存在するか確認する
				if (EventInstigator)
				{
					// 攻撃したプレイヤーのPlayerStateを取得する
					AThrowWeponPlayerState* AttackerPlayerState =
						EventInstigator->GetPlayerState<AThrowWeponPlayerState>();

					// 攻撃したプレイヤーのPlayerStateを取得できたか確認する
					if (AttackerPlayerState)
					{
						// キル数を1増やす
						AttackerPlayerState->AddKill();
					}
				}

				// 死亡時のノックバックを計算する
				FVector FatalKnockback =
					(HitDirection + FVector(0.f, 0.f, 0.6f)).GetSafeNormal() * 4500.0f;

				// キャラクターを吹き飛ばす
				LaunchCharacter(FatalKnockback, true, true);

				// 全端末に死亡処理を通知する
				Multicast_OnDeath();

				// 自分を操作しているプレイヤーのControllerを取得する
				AController* PlayerController = GetController();

				// Controllerが存在するか確認する
				if (PlayerController)
				{
					// 3秒後にリスポーン処理を実行する
					FTimerHandle RespawnTimer;

					// タイマーにリスポーン処理を登録する
					GetWorldTimerManager().SetTimer(
						RespawnTimer,
						[ThrowGameMode, PlayerController]()
						{
							// 指定したプレイヤーをリスポーンさせる
							ThrowGameMode->RespawnPlayer(PlayerController);
						},
						3.0f,
						false
					);
				}
			}
		}
		else
		{
			// 【通常被弾時】：軽めの通常ノックバック（800.0f）
			FVector KnockbackVelocity = (HitDirection + FVector(0.f, 0.f, 0.35f)).GetSafeNormal() * 800.0f;
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
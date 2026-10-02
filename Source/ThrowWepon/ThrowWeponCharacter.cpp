#include "ThrowWeponCharacter.h"
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
	// 入力を無効化
	DisableInput(Cast<APlayerController>(GetController()));

	// 被弾直前の移動速度（勢い）を記録
	FVector LastVelocity = GetVelocity();

	// カプセルの当たり判定を消してラグドール化
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	// ラグドール化した瞬間に被弾時の吹っ飛び力（インパルス）を加える
	FVector DeathLaunchForce = (LastVelocity.GetSafeNormal() + FVector(0.f, 0.f, 0.5f)).GetSafeNormal() * 1500.0f;
	GetMesh()->AddImpulse(DeathLaunchForce, NAME_None, true);
}

float AThrowWeponCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.0f)
	{
		Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

		// --- 被弾方向の計算 ---
		FVector HitDirection = GetActorForwardVector() * -1.0f; // 基本は後ろ方向
		if (DamageCauser)
		{
			// ボール（DamageCauser）から被弾者へのベクトル
			HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
		}

		// 死亡判定
		if (Health <= 0.0f)
		{
			// 被弾方向の強大なベクトルを一度 Launch して勢いをつけた直後に Multicast_OnDeath を呼ぶ
			FVector FatalKnockback = (HitDirection + FVector(0.f, 0.f, 0.6f)).GetSafeNormal() * 2000.0f;
			LaunchCharacter(FatalKnockback, true, true);

			// 全員にラグドール死亡通知
			Multicast_OnDeath();
		}
		else
		{
			// 通常被弾ノックバック（斜め後ろ上に押し出す）
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
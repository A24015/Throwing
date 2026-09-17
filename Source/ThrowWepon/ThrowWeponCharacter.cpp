#include "ThrowWeponCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

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

	// Enhanced Input Component へキャストしてバインドを設定
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jump
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AThrowWeponCharacter::DoJumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AThrowWeponCharacter::DoJumpEnd);
		}

		// Move
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AThrowWeponCharacter::Move);
		}

		// Look
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThrowWeponCharacter::Look);
		}

		// Mouse Look
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
// ダメージ処理
// ---------------------------------------------------------

void AThrowWeponCharacter::Multicast_OnDeath_Implementation()
{
	// ホスト・参加者双方の画面で物理（ラグドール）を有効化
	DisableInput(Cast<APlayerController>(GetController()));
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

float AThrowWeponCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.0f)
	{
		Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

		// 死亡判定
		if (Health <= 0.0f)
		{
			// サーバーから全員へ「死亡処理を実行しろ」と命令を投げる
			Multicast_OnDeath();
		}
	}
	return ActualDamage;
}
//float AThrowWeponCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
//{
//	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
//
//	if (ActualDamage > 0.0f)
//	{
//		// HPを減らす
//		Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);
//
//		// --- 画面上に「○○ ダメージヒット！」と表示 ---
//		if (GEngine)
//		{
//			FString Message = FString::Printf(TEXT("%.1f ダメージヒット！ (残HP: %.1f)"), ActualDamage, Health);
//
//			// Key: -1 (新規行として追加), DisplayTime: 3.0秒, Color: 赤色
//			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Message);
//		}
//
//		// --- 死亡判定（HPが0になった時の処理） ---
//		if (Health <= 0.0f)
//		{
//			if (GEngine)
//			{
//				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("プレイヤー死亡！"));
//			}
//
//			// ラグドール化（物理で倒れる処理）
//			DisableInput(Cast<APlayerController>(GetController()));
//			GetMesh()->SetSimulatePhysics(true);
//			GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
//			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//		}
//	}
//
//	return ActualDamage;
//}

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


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ThrowWeponGameMode.h"
#include "ThrowWeponGameInstance.generated.h"

UCLASS()
class THROWWEPON_API UThrowWeponGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:

    // タイトル画面で入力したプレイヤー名を保存する変数
    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString MyPlayerName;

    // 試合結果を保存する配列
    UPROPERTY(BlueprintReadWrite, Category = "Result")
    TArray<FResultData> ResultData;
};
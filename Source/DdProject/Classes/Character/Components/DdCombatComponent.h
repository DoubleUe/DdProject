#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DdCombatComponent.generated.h"

class UAnimMontage;
class UDdTableSubsystem;
struct FDdActionTableRow;

// 콤보 한 단계의 몽타주와 블렌드 정보
USTRUCT(BlueprintType)
struct FDdComboStep
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bUseBlend = false;
};

// 하나의 콤보 세트 (ActionTable 한 행에 대응)
USTRUCT(BlueprintType)
struct FDdComboSet
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ActionId = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FDdComboStep> Steps;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DDPROJECT_API UDdCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDdCombatComponent();

	// 무기의 ComboIds로 액션테이블에서 콤보 데이터 로드
	void LoadComboData(const TArray<int32>& ComboIds, UDdTableSubsystem* TableSubsystem);

	UFUNCTION(BlueprintPure, Category = "Combat")
	const TArray<FDdComboSet>& GetComboSets() const { return ComboSets; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetComboSetCount() const { return ComboSets.Num(); }

protected:
	virtual void BeginPlay() override;

private:
	bool LoadComboStepsFromActionRow(const FDdActionTableRow& ActionRow, UDdTableSubsystem* TableSubsystem, FDdComboSet& OutComboSet);

	UPROPERTY(Transient)
	TArray<FDdComboSet> ComboSets;
};

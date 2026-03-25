#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DdBaseCharacter.generated.h"

// 플레이어와 몬스터의 공통 베이스 캐릭터 클래스
UCLASS(Abstract)
class DDPROJECT_API ADdBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADdBaseCharacter();

	// 이동 입력 차단 설정 (노티파이에서 호출)
	virtual void SetAttackMovementInputBlocked(bool bBlocked) PURE_VIRTUAL(ADdBaseCharacter::SetAttackMovementInputBlocked, );

	// 공격 입력 차단 설정 (노티파이에서 호출)
	virtual void SetAttackInputBlocked(bool bBlocked) PURE_VIRTUAL(ADdBaseCharacter::SetAttackInputBlocked, );
};

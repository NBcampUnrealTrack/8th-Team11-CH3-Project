#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZombieAIController.generated.h"

class AZombieCharacter;

// BlackBoard 변수를 안전하게 담는 네임 변수모음
namespace BBKeys
{
	const FName DistanceToPlayer = TEXT("DistanceToPlayer");
	const FName bIsInSight = TEXT("bIsInSight");
	const FName TargetActor = TEXT("TargetActor");
}

UCLASS()
class NBC_CH3_TEAMPROJECT_API AZombieAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AZombieAIController();

	// 몽타주 시간을 저장할 변수
	float MontageTime;
	// BT 에셋 변수(블루프린트에서 설정)
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBehaviorTree> BTAsset;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	bool IsCanAttackSight();

private:
	// 공격중인지 판단하는 변수
	bool bIsAttaking;
	// 공격 모션동안 실행 될 타이머핸들
	FTimerHandle AttackTimer;

	UPROPERTY()
	TObjectPtr<ACharacter> PlayerCharacter;

	UPROPERTY()
	TObjectPtr<AZombieCharacter> ZombieCharacter;
};

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossAIController.generated.h"

class ABossCharacter;

UCLASS()
class NBC_CH3_TEAMPROJECT_API ABossAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABossAIController();

	// 몽타주 시간을 저장할 변수(ST에서 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "STComponent")
	float MontageTime;
	// 보스가 시야에 있는지 판단(ST에서 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "STComponent")
	bool bIsInSightBoss;
	// 공격중인지 판단하는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ABPComponent")
	bool bIsAttaking;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ABPComponent")
	bool bIsCanWalk;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ABPComponent")
	float BossXYSpeed;
	float Distance;
	int32 PhaseTwoJumpRandomIndex;
	int32 PhaseThreeJumpRandomIndex;
	

	UFUNCTION(BlueprintCallable)
	void BossMoveToActor();
	UFUNCTION(BlueprintCallable)
	void BossAttack();
	UFUNCTION(BlueprintCallable)
	void ChangePhaseTwo();
	UFUNCTION(BlueprintCallable)
	void ChangePhaseThree();
	void BossJumpAttack();
	void TriggerPhaseTransition();

protected:

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	bool IsCanAttackSight();
	bool IsCanJumpAttackSight();
	void TurnToPlayer(float DeltaSeconds);
	
	// 공격 모션동안 실행 될 타이머핸들
	FTimerHandle AttackTimer;

	UPROPERTY()
	TObjectPtr<ACharacter> PlayerCharacter;
	UPROPERTY()
	TObjectPtr<ABossCharacter> BossCharacter;
	
};

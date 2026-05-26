#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossAttackComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NBC_CH3_TEAMPROJECT_API UBossAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UBossAttackComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 공격 시작 (ANS_Begin에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Boss | Attack")
	void StartMeleeTrace(FName InSocketName, float InRadius);

	// 공격 추적 연산을 수행 (ANS_Tick에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Boss | Attack")
	void ExecuteMeleeTrace();

	// 공격 종료 (ANS_End에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Boss | Attack")
	void EndMeleeTrace();

	// 광역 공격 함수
	UFUNCTION(BlueprintCallable, Category = "Boss | Attack")
	void ExecuteRadialSlam(FVector CenterLocation, float Radius, float DamageAmount);

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY()
	TObjectPtr<ACharacter> BossCharacter;

	// 현재 공격 추적 중인지 여부
	bool bIsAttacking;

	// 추적 타겟 소켓 이름 및 반경
	FName TargetSocketName;
	float TraceRadius;

	// 지난 프레임의 소켓 위치 기록 (궤적을 그리기 위함)
	FVector LastSocketLocation;

	// 한 번의 공격 콤보 내에서 이미 대미지를 입은 액터들의 목록 (중복 피격 방지)
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AlreadyHitActors;
};

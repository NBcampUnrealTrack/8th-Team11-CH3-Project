#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"

class UAnimMontage;
class UHealthComponent;
class UHitReactComponent;

UCLASS()
class NBC_CH3_TEAMPROJECT_API AZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AZombieCharacter();

	// 공격 애니메이션 몽타주들을 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage")
	TArray<UAnimMontage*> AttackMontage;

	// [장식 추가] 좀비별 공격 파라미터 (BT 진입 시점에 데미지 적용 게이트로 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta=(ClampMin="0.0", ClampMax="180.0"))
	float AttackHalfAngleDeg = 35.f;
	// 히트 애니메이션 몽타주들을 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage")
	TArray<UAnimMontage*> HitMontage;
	// 사망 애니메이션 몽타주들을 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage")
	TArray<UAnimMontage*> DeathMontage;
	// HealthComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;
	// HitReactComponent (본 단위 물리 블렌드 피격 반응)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHitReactComponent> HitReactComponent;
	
	// GameplayStatics::ApplyDamage와 같은 파이프라인
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void OnHit(float CurrentHealth);
	UFUNCTION()
	void OnDeath();

protected:

	virtual void BeginPlay() override;
};

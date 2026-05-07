#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"

class UAnimMontage;

UCLASS()
class NBC_CH3_TEAMPROJECT_API AZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AZombieCharacter();

	// 공격 애니메이션 몽타주들을 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage")
	TArray<UAnimMontage*> AttackMontage;
	// 사망 애니메이션 몽타주들을 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage")
	TArray<UAnimMontage*> DeathMontage;

	// HP와 TakeDamage
	float MaxHP = 100.f;
	float CurrentHP = 100.f;
	// GameplayStatics::ApplyDamage와 같은 파이프라인
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	//void OnDeath();
	
};

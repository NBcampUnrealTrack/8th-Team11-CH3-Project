#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BossCharacter.generated.h"

class UHealthComponent;
class UHitReactComponent;
class UAnimMontage;
class ABossAIController;

UENUM(BlueprintType)
enum class EBossPhase : uint8 
{ 
	Phase1, 
	Phase2,
	Phase3
};

UCLASS()
class NBC_CH3_TEAMPROJECT_API ABossCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABossCharacter();

	// HealthComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;
	// HitReactComponent (본 단위 물리 블렌드 피격 반응)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHitReactComponent> HitReactComponent;

	// 페이즈1 공격 애니메이션 몽타주들을 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | Attack")
	TArray<UAnimMontage*> AttackMontagePhaseOne;
	// 페이즈2, 3 공격 애니메이션 몽타주들을 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | Attack")
	TArray<UAnimMontage*> AttackMontagePhaseTwoToThree;
	// 히트 애니메이션 몽타주들을 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | Hit")
	TArray<UAnimMontage*> HitMontage;
	// 사망 애니메이션 몽타주들을 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | Death")
	TArray<UAnimMontage*> DeathMontage;
	// 점프 공격 애니메이션 몽타주를 담는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | JumpAttack")
	TObjectPtr<UAnimMontage> JumpAttackMontageOne;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | JumpAttack")
	TObjectPtr<UAnimMontage> JumpAttackMontageTwo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | JumpAttack")
	TObjectPtr<UAnimMontage> JumpAttackMontageThree;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | JumpAttack")
	TObjectPtr<UAnimMontage> JumpAttackMontageFour;
	// 페이즈 전환시 실행할 애니메이션 몽타주를 담는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | PhaseChange")
	TObjectPtr<UAnimMontage> ChangePhaseTwo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationMontage | PhaseChange")
	TObjectPtr<UAnimMontage> ChangePhaseThree;

	// ST에서 사용할 페이즈 확인 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EBossPhase BossPhase;
	// 에디터에서 페이즈 테스트용으로 리플렉션 시스템에 등록
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BossCurrentHealth;
	float BossMaxHealth;

	// GameplayStatics::ApplyDamage와 같은 파이프라인
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void OnHit(float CurrentHealth);
	UFUNCTION()
	void OnDeath();

protected:
	virtual void BeginPlay() override;

	TObjectPtr<ABossAIController> BossAI;
	int32 CurrentPhase;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// [장식 추가] 좀비/플레이어 피격 흐름 디버그용 로그 카테고리. Output Log에서 'LogCombat:'으로 필터링.
DECLARE_LOG_CATEGORY_EXTERN(LogCombat, Log, All);

// 체력 변동 / 사망 시 HUD·디버프·GameMode가 구독하는 멀티캐스트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

// 액터에 부착하면 체력 관리 + 데미지 수신 + 사망 이벤트를 제공한다.
// 플레이어/좀비/파괴 가능 오브젝트 공통.
UCLASS(Blueprintable, ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class NBC_CH3_TEAMPROJECT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	// 데미지 적용. 체력 감소 + Broadcast, 0 이하 시 OnDeath 발동.
	UFUNCTION(BlueprintCallable, Category="Health")
	void ApplyDamage(float Amount);

	// 회복 (MaxHealth 상한 클램프).
	UFUNCTION(BlueprintCallable, Category="Health")
	void Heal(float Amount);
	
	UFUNCTION(BlueprintCallable, Category="Health")
	void IncreaseMaxHealth(float Amount);

	UFUNCTION(BlueprintPure, Category="Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="Health")
	bool IsDead() const { return CurrentHealth <= 0.0f; }


	// 체력 변경 이벤트 — HUD HealthBar 구독.
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnHealthChanged OnHealthChanged;

	// 사망 이벤트 — PlayerController/좀비가 구독해 후처리 위임.
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnDeath OnDeath;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	float CurrentHealth = 100.0f;
};

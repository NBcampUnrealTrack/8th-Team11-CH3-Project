#include "Combat/HealthComponent.h"

// [장식 추가] LogCombat 카테고리 정의 (선언은 헤더)
DEFINE_LOG_CATEGORY(LogCombat);

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void UHealthComponent::ApplyDamage(float Amount)
{
	if (IsDead()) { return; }

	const float Before = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth);

	UE_LOG(LogCombat, Warning, TEXT("[Health] %s ApplyDamage(%.1f) — HP %.1f → %.1f / %.1f%s"),
		*GetNameSafe(GetOwner()), Amount, Before, CurrentHealth, MaxHealth,
		IsDead() ? TEXT(" [DEAD]") : TEXT(""));

	if (IsDead())
	{
		OnDeath.Broadcast();
	}
}

void UHealthComponent::Heal(float Amount)
{
	if (IsDead()) { return; }

	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth);
}

// [창욱 수정] 합연산에서 곱연산으로 수정
void UHealthComponent::IncreaseMaxHealth(float Amount)
{
	if (Amount <= 0.0f) return;

	MaxHealth *= Amount;
	CurrentHealth = FMath::Clamp(CurrentHealth * Amount, 0.0f, MaxHealth);
	OnMaxHealthChanged.Broadcast(MaxHealth); 
	OnHealthChanged.Broadcast(CurrentHealth);
}



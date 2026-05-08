#include "Combat/HealthComponent.h"

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

	UE_LOG(LogTemp, Warning, TEXT("[Health] %s ApplyDamage(%.1f) — HP %.1f → %.1f / %.1f%s"),
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

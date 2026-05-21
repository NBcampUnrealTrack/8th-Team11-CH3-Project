// Fill out your copyright notice in the Description page of Project Settings.
#include "Player/WeaponRewardComponent.h"
#include "Player/BaseWeapon.h"
#include "Player/PlayerCharacter.h"



// Sets default values for this component's properties
UWeaponRewardComponent::UWeaponRewardComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


APlayerCharacter* UWeaponRewardComponent::GetOwnerPlayer() const
{
	return Cast<APlayerCharacter>(GetOwner());
}

void UWeaponRewardComponent::GenerateRewardChoices()
{
	CurrentRewardChoices.Empty();
	
	TArray<TSubclassOf<ABaseWeapon>> Pool = RewardWeaponPool;
	
	while (Pool.Num() > 0 && CurrentRewardChoices.Num() < RewardChoiceCount)
	{
		const int32 RandomIndex = FMath::RandRange(0, Pool.Num() - 1);
		
		CurrentRewardChoices.Add(Pool[RandomIndex]);
		Pool.RemoveAt(RandomIndex);
	}
	OnRewardChoicesGenerated.Broadcast();
}

void UWeaponRewardComponent::SelectRewardChoice(int32 ChoiceIndex)
{
	if (!CurrentRewardChoices.IsValidIndex(ChoiceIndex)) return;
	
	APlayerCharacter* Player = GetOwnerPlayer();
	if (!Player) return;
	
	TSubclassOf<ABaseWeapon> SelectWeaponClass = CurrentRewardChoices[ChoiceIndex];
	if (!SelectWeaponClass) return;
	
	bool bWasUpgrade = false;
	ABaseWeapon* ResultWeapon = FindOwnedWeaponByClass(SelectWeaponClass);
	
	if (ResultWeapon)
	{
		ApplyUpgradeToWeapon(ResultWeapon);
		bWasUpgrade = true;
	}
	else
	{
		const int32 NewWeaponIndex = Player->WeaponInventory.Num();
		
		if (Player->AddWeaponToInventory(SelectWeaponClass))
		{
			if (Player->WeaponInventory.IsValidIndex(NewWeaponIndex))
			{
				ResultWeapon = Player->WeaponInventory[NewWeaponIndex];
			}
			if (Player->CurrentWeaponIndex == -1)
			{
				Player->SwitchWeapon(NewWeaponIndex);
			}
		}
	}
	OnRewardApplied.Broadcast(ResultWeapon, bWasUpgrade);
	
	CurrentRewardChoices.Empty();
}

ABaseWeapon* UWeaponRewardComponent::FindOwnedWeaponByClass(TSubclassOf<ABaseWeapon> WeaponClass) const
{
	APlayerCharacter* Player = GetOwnerPlayer();
	if (!Player || !WeaponClass) return nullptr;

	for (ABaseWeapon* Weapon : Player->WeaponInventory)
	{
		if (Weapon && Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	return nullptr;
}

void UWeaponRewardComponent::ApplyUpgradeToWeapon(ABaseWeapon* Weapon)
{
	if (!Weapon) return;

	Weapon->FireRate *= FireRateUpgradeMultiplier;
	Weapon->FireRate = FMath::Max(Weapon->FireRate, 0.03f);

	Weapon->MaxBulletCount += MaxBulletUpgradeAmount;
	Weapon->CurrentBulletCount = Weapon->MaxBulletCount;

	UE_LOG(LogTemp, Warning, TEXT("[Reward] Upgraded %s"), *Weapon->GetName());
}

bool UWeaponRewardComponent::IsChoiceUpgrade(int32 ChoiceIndex) const
{
	if (!CurrentRewardChoices.IsValidIndex(ChoiceIndex)) return false;

	return FindOwnedWeaponByClass(CurrentRewardChoices[ChoiceIndex]) != nullptr;
}

FText UWeaponRewardComponent::GetChoiceDisplayName(int32 ChoiceIndex) const
{
	if (!CurrentRewardChoices.IsValidIndex(ChoiceIndex))
	{
		return FText::GetEmpty();
	}

	TSubclassOf<ABaseWeapon> WeaponClass = CurrentRewardChoices[ChoiceIndex];
	if (!WeaponClass)
	{
		return FText::GetEmpty();
	}

	const ABaseWeapon* DefaultWeapon = WeaponClass->GetDefaultObject<ABaseWeapon>();
	if (DefaultWeapon && !DefaultWeapon->WeaponName.IsNone())
	{
		return FText::FromName(DefaultWeapon->WeaponName);
	}

	return FText::FromString(WeaponClass->GetName());
}

void UWeaponRewardComponent::ApplyWeaponConfigReward(UWeaponConfig* Config)
{
	if (!Config) return;

	APlayerCharacter* Player = GetOwnerPlayer();
	if (!Player) return;

	if (!Player->WeaponInventory.IsValidIndex(Player->CurrentWeaponIndex)) return;

	ABaseWeapon* CurrentWeapon = Player->WeaponInventory[Player->CurrentWeaponIndex];
	if (!CurrentWeapon) return;

	ApplyUpgradeToWeapon(CurrentWeapon);
	OnRewardApplied.Broadcast(CurrentWeapon, true);
}
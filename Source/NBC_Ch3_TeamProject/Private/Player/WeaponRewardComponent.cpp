// Fill out your copyright notice in the Description page of Project Settings.
#include "Player/WeaponRewardComponent.h"
#include "Player/BaseWeapon.h"
#include "Combat/WeaponConfig.h"
#include "Player/PlayerCharacter.h"
#include "System/NBC_GameInstance.h"



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

	// 무기가 없으면 풀에서 매칭되는 무기 획득 
	if (Player->WeaponInventory.IsEmpty())
	{
		TSubclassOf<ABaseWeapon> WeaponToGive = nullptr;

		// RewardWeaponPool에서 Config의 WeaponClass(Rifle/Shotgun/Pistol)와 일치하는 무기 탐색
		for (TSubclassOf<ABaseWeapon> WeaponClass : RewardWeaponPool)
		{
			if (!WeaponClass) continue;

			const ABaseWeapon* DefaultWeapon = WeaponClass->GetDefaultObject<ABaseWeapon>();
			if (DefaultWeapon && DefaultWeapon->WeaponConfig
				&& DefaultWeapon->WeaponConfig->WeaponClass == Config->WeaponClass)
			{
				WeaponToGive = WeaponClass;
				break;
			}
		}

		if (!WeaponToGive) return; // 매칭 무기 없음

		const int32 NewIndex = Player->WeaponInventory.Num();
		if (Player->AddWeaponToInventory(WeaponToGive))
		{
			ABaseWeapon* NewWeapon = Player->WeaponInventory.IsValidIndex(NewIndex)
				? Player->WeaponInventory[NewIndex] : nullptr;

			Player->SwitchWeapon(NewIndex);
			OnRewardApplied.Broadcast(NewWeapon, false);
		}
		return;
	}

	// 무기가 있으면 현재 무기 업그레이드
	if (!Player->WeaponInventory.IsValidIndex(Player->CurrentWeaponIndex)) return;

	ABaseWeapon* CurrentWeapon = Player->WeaponInventory[Player->CurrentWeaponIndex];
	if (!CurrentWeapon) return;

	ApplyUpgradeToWeapon(CurrentWeapon);
	OnRewardApplied.Broadcast(CurrentWeapon, true);
}

// GameInstance에 무기 저장 
void UWeaponRewardComponent::SaveWeaponsToInstance()
{
	APlayerCharacter* Player = GetOwnerPlayer();
	if (!Player) return;

	UNBC_GameInstance* GI = Cast<UNBC_GameInstance>(GetWorld()->GetGameInstance());
	if (!GI) return;

	GI->ClearSavedData();

	TArray<TSubclassOf<ABaseWeapon>> TempClasses;
	for (ABaseWeapon* WeaponActor : Player->WeaponInventory)
	{
		if (WeaponActor)
		{	
			TSubclassOf<ABaseWeapon> WeaponClass = WeaponActor->GetClass();
			TempClasses.Add(WeaponClass);

			GI->SetSavedFireRate(WeaponClass, WeaponActor->FireRate);
			GI->SetSavedMaxBullets(WeaponClass, WeaponActor->MaxBulletCount);
		}
	}

	GI->SetSavedWeaponClasses(TempClasses);
	GI->SetSavedCurrentWeaponIndex(Player->CurrentWeaponIndex);

	UE_LOG(LogTemp, Warning, TEXT("Weapon Saved"));
}

// 무기 로드 
void UWeaponRewardComponent::LoadWeaponsFromInstance()
{
	APlayerCharacter* Player = GetOwnerPlayer();
	if (!Player) return;

	UNBC_GameInstance* GI = Cast<UNBC_GameInstance>(GetWorld()->GetGameInstance());
	if (!GI || GI->GetSavedWeaponClasses().IsEmpty()) return;

	Player->WeaponInventory.Empty();
	Player->CurrentWeaponIndex = -1;
	
	for (TSubclassOf<ABaseWeapon> WeaponClass : GI->GetSavedWeaponClasses())
	{
		if (!WeaponClass) continue;
		Player->AddWeaponToInventory(WeaponClass);

		if (!Player->WeaponInventory.IsEmpty())
		{
			ABaseWeapon* SpawnedWeapon = Player->WeaponInventory.Last();
			if (SpawnedWeapon)
			{
				float SavedFireRate = GI->GetSavedFireRate(WeaponClass);
				int32 SavedMaxBullets = GI->GetSavedMaxBullets(WeaponClass);

				if (SavedFireRate > 0.0f) SpawnedWeapon->FireRate = SavedFireRate;
				if (SavedMaxBullets > 0.0f) SpawnedWeapon->MaxBulletCount = SavedMaxBullets;

				SpawnedWeapon->CurrentBulletCount = SpawnedWeapon->MaxBulletCount;
			}
		}
	}

	int32 TargetIndex = GI->GetSavedCurrentWeaponIndex();
	if (Player->WeaponInventory.IsValidIndex(TargetIndex))
	{
		Player->SwitchWeapon(TargetIndex);
	}
	else if (!Player->WeaponInventory.IsEmpty())
	{
		Player->SwitchWeapon(0);
	}

	UE_LOG(LogTemp, Warning, TEXT("Weapon Loaded"));
}
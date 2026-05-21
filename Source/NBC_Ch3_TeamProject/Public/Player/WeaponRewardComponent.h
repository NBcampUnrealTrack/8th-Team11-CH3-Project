// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponRewardComponent.generated.h"

class ABaseWeapon;
class APlayerCharacter;
class UWeaponConfig; 

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponRewardChoicesGenerated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWeaponRewardApplied,
	ABaseWeapon*,
	Weapon,
	bool,
	bWasUpgrade);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NBC_CH3_TEAMPROJECT_API UWeaponRewardComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponRewardComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TArray<TSubclassOf<ABaseWeapon>> RewardWeaponPool;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reward")
	TArray<TSubclassOf<ABaseWeapon>> CurrentRewardChoices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 RewardChoiceCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward|Upgrade")
	float FireRateUpgradeMultiplier = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward|Upgrade")
	int32 MaxBulletUpgradeAmount = 5;

	UPROPERTY(BlueprintAssignable, Category = "Reward")
	FOnWeaponRewardChoicesGenerated OnRewardChoicesGenerated;

	UPROPERTY(BlueprintAssignable, Category = "Reward")
	FOnWeaponRewardApplied OnRewardApplied;

	
	// 함수
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void GenerateRewardChoices();

	UFUNCTION(BlueprintCallable, Category = "Reward")
	void SelectRewardChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintPure, Category = "Reward")
	bool IsChoiceUpgrade(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "Reward")
	FText GetChoiceDisplayName(int32 ChoiceIndex) const;
	
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void ApplyWeaponConfigReward(UWeaponConfig* Config);

	// GameInstance에 무기 저장을 위한 함수
	// 현재 무기 목록을 백업
	UFUNCTION(BlueprintCallable, Category = "SaveWeapon")
	void SaveWeaponsToInstance();

	// 무기 목록 로드
	UFUNCTION(BlueprintCallable, Category = "LoadWeapon")
	void LoadWeaponsFromInstance();

protected:
	APlayerCharacter* GetOwnerPlayer() const;

	ABaseWeapon* FindOwnedWeaponByClass(TSubclassOf<ABaseWeapon> WeaponClass) const;

	void ApplyUpgradeToWeapon(ABaseWeapon* Weapon);

};

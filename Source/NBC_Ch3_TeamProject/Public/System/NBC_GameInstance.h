// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Player/BaseWeapon.h"
#include "NBC_GameInstance.generated.h"

UCLASS()
class NBC_CH3_TEAMPROJECT_API UNBC_GameInstance : public UGameInstance
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameData")
	int32 SavedTotalScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameData")
	int32 SavedCurrentWave = 1;

	// [장식 복원] 무기 BP 클래스 임시 저장 배열
	UPROPERTY(BlueprintReadOnly, Category = "Save Data", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<ABaseWeapon>> SavedWeaponClasses;

	// [장식 복원] 맵 이동 시 들고 있던 무기의 인덱스 번호
	UPROPERTY(BlueprintReadOnly, Category = "Save Data", meta = (AllowPrivateAccess = "true"))
	int32 SavedCurrentWeaponIndex = -1;

	// [장식 추가] 노멀 레벨을 포털로 통과한 누적 횟수 (보스 진입 조건 판정용)
	UPROPERTY(BlueprintReadOnly, Category = "Save Data", meta = (AllowPrivateAccess = "true"))
	int32 NormalLevelsCleared = 0;

	// [장식 추가] 현재 플레이어가 보상맵에 있는지 여부 (노멀↔보상 토글)
	UPROPERTY(BlueprintReadOnly, Category = "Save Data", meta = (AllowPrivateAccess = "true"))
	bool bIsInRewardLevel = false;

	// 플레이어 스탯
	UPROPERTY(BlueprintReadOnly, Category = "Save Data | Player", meta = (AllowPrivateAccess = "true"))
	float SavedCurrentHP = -1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Save Data | Player", meta = (AllowPrivateAccess = "true"))
	float SavedMaxHP = -1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Save Data | Player", meta = (AllowPrivateAccess = "true"))
	float SavedMoveSpeed = -1.0f;

	// 총기 업그레이드 
	UPROPERTY(BlueprintReadOnly, Category = "Save Data | Weapon", meta = (AllowPrivateAccess = "true"))
	TMap<TSubclassOf<ABaseWeapon>, float> SavedWeaponFireRates;

	UPROPERTY(BlueprintReadOnly, Category = "Save Data | Weapon", meta = (AllowPrivateAccess = "true"))
	TMap<TSubclassOf<ABaseWeapon>, int32> SavedWeaponMaxBullets;

public:
	UFUNCTION(BlueprintPure)
	int32 GetSavedTotalScore() { return SavedTotalScore; }

	UFUNCTION(BlueprintPure)
	int32 GetSavedCurrentWave() { return SavedCurrentWave; }

	UFUNCTION(BlueprintCallable)
	void SetSavedTotalScore(int32 Score) { SavedTotalScore = Score; }

	UFUNCTION(BlueprintCallable)
	void SetSavedCurrentWave(int32 Wave) { SavedCurrentWave = Wave; }

	// [장식 복원] 저장된 무기 클래스 가져오기
	const TArray<TSubclassOf<ABaseWeapon>>& GetSavedWeaponClasses() const { return SavedWeaponClasses; }

	// [장식 복원] 저장된 무기 클래스 설정
	void SetSavedWeaponClasses(const TArray<TSubclassOf<ABaseWeapon>>& Classes) { SavedWeaponClasses = Classes; }

	// [장식 복원] 저장된 무기 인덱스 가져오기
	int32 GetSavedCurrentWeaponIndex() const { return SavedCurrentWeaponIndex; }

	// [장식 복원] 저장된 무기 인덱스 설정
	void SetSavedCurrentWeaponIndex(int32 Index) { SavedCurrentWeaponIndex = Index; }

	// [장식 추가] NormalLevelsCleared 접근자
	UFUNCTION(BlueprintPure)
	int32 GetNormalLevelsCleared() const { return NormalLevelsCleared; }

	UFUNCTION(BlueprintCallable)
	void IncNormalLevelsCleared() { ++NormalLevelsCleared; }

	UFUNCTION(BlueprintCallable)
	void ResetNormalLevelsCleared() { NormalLevelsCleared = 0; }

	// [장식 추가] 보상맵 위치 플래그 접근자
	UFUNCTION(BlueprintPure)
	bool IsInRewardLevel() const { return bIsInRewardLevel; }

	UFUNCTION(BlueprintCallable)
	void SetInRewardLevel(bool bInReward) { bIsInRewardLevel = bInReward; }

	// [장식 복원] 게임오버 시 클리어 데이터 초기화 (무기 한정)
	// [창욱 추가] 플레이어 스탯, 무기 강화 초기화
	void ClearSavedData();

	// [장식 추가] 진행도(노멀 카운트 + 보상맵 플래그) 초기화. 게임오버/메뉴 복귀 시 호출.
	UFUNCTION(BlueprintCallable)
	void ResetProgression();

	// 플레이어 스탯 
	// CurrentHP
	float GetSavedCurrentHP() const { return SavedCurrentHP; }
	void SetSavedCurrentHP(float HP) { SavedCurrentHP = HP; }

	// MaxHP
	float GetSavedMaxHP() const { return SavedMaxHP; }
	void SetSavedMaxHP(float MaxHP) { SavedMaxHP = MaxHP; }	

	// Speed
	float GetSavedMoveSpeed() const { return SavedMoveSpeed; }
	void SetSavedMoveSpeed(float Speed) { SavedMoveSpeed = Speed; }

	// 특정 무기 클래스에 대해 강화 값 누적 
	// FireRates
	float GetSavedFireRate(TSubclassOf<ABaseWeapon> WeaponClass) const
	{
		return SavedWeaponFireRates.Contains(WeaponClass) ? SavedWeaponFireRates[WeaponClass] : -1.0f;
	}
	void SetSavedFireRate(TSubclassOf<ABaseWeapon> WeaponClass, float FireRate) { SavedWeaponFireRates.FindOrAdd(WeaponClass) = FireRate; }

	// Bullets
	float GetSavedMaxBullets(TSubclassOf<ABaseWeapon> WeaponClass) const
	{
		return SavedWeaponMaxBullets.Contains(WeaponClass) ? SavedWeaponMaxBullets[WeaponClass] : -1.0f;
	}
	void SetSavedMaxBullets(TSubclassOf<ABaseWeapon> WeaponClass, int32 Bullets) { SavedWeaponMaxBullets.FindOrAdd(WeaponClass) = Bullets; }
};

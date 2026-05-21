// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NBC_GameInstance.generated.h"

class ABaseWeapon;

UCLASS()
class NBC_CH3_TEAMPROJECT_API UNBC_GameInstance : public UGameInstance
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameData")
	int32 SavedTotalScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameData")
	int32 SavedCurrentWave = 1;
	
	// 무기 BP들을 임시저장할 배열 
	UPROPERTY(BlueprintReadOnly, Category = "Save Data", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<ABaseWeapon>> SavedWeaponClasses;

	// 맵 이동 전 들고 있던 무기의 인덱스 번호
	UPROPERTY(BlueprintReadOnly, Category = "Save Data", meta = (AllowPrivateAccess = "true"))
	int32 SavedCurrentWeaponIndex = -1;

public:
	UFUNCTION(BlueprintPure)
	int32 GetSavedTotalScore() { return SavedTotalScore; }

	UFUNCTION(BlueprintPure)
	int32 GetSavedCurrentWave() { return SavedCurrentWave; }

	UFUNCTION(BlueprintCallable)
	void SetSavedTotalScore(int32 Score) { SavedTotalScore = Score; }

	UFUNCTION(BlueprintCallable)
	void SetSavedCurrentWave(int32 Wave) { SavedCurrentWave = Wave; }

	// 무기 목록 데이터 가져오기 
	const TArray<TSubclassOf<ABaseWeapon>>& GetSavedWeaponClasses() const { return SavedWeaponClasses; }

	// 무기 목록 저장 
	void SetSavedWeaponClasses(const TArray<TSubclassOf<ABaseWeapon>>& Classes) { SavedWeaponClasses = Classes; }

	// 현재 무기 인덱스 가져오기 
	int32 GetSavedCurrentWeaponIndex() const { return SavedCurrentWeaponIndex; }

	// 현재 무기 인덱스 설정 
	void SetSavedCurrentWeaponIndex(int32 Index) { SavedCurrentWeaponIndex = Index;           }

	// 게임오버나 클리어시 데이터 초기화
	void ClearSavedData();

};

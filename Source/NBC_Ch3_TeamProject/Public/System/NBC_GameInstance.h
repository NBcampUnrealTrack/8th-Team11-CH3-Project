// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NBC_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class NBC_CH3_TEAMPROJECT_API UNBC_GameInstance : public UGameInstance
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameData")
	int32 SavedTotalScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameData")
	int32 SavedCurrentWave = 1;
	
	// 추후 카드 획득한 것도 저장 

public:
	UFUNCTION(BlueprintPure)
	int32 GetSavedTotalScore() { return SavedTotalScore; }

	UFUNCTION(BlueprintPure)
	int32 GetSavedCurrentWave() { return SavedCurrentWave; }

	UFUNCTION(BlueprintCallable)
	void SetSavedTotalScore(int32 Score) { SavedTotalScore = Score; }

	UFUNCTION(BlueprintCallable)
	void SetSavedCurrentWave(int32 Wave) { SavedCurrentWave = Wave; }
};

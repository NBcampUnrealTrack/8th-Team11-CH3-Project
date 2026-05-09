// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NBC_Ch3_TeamProject/Public/System/NBC_GameState.h"
#include "NBC_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class NBC_CH3_TEAMPROJECT_API ANBC_GameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ANBC_GameMode();

	// 페이즈 전환 
	void ChangePhase(EGamePhase NewPhase);

	// 몹이 죽었을 때 호출 
	void OnMonsterKilled();

	// 플레이어가 죽었을 떄 호출 
	void OnPlayerDied();

protected:
	virtual void BeginPlay() override;

	// 웨이브마다 목표 처치 수 
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category="GameRules")
	int32 TargetKillCount;

	// 현재 처치 수 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="GameRules")
	int32 CurrentKillCount;

	// 클리어 조건 웨이브 수 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameRules")
	int32 MaxWaves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Score")
	int32 ScorePerKill = 100;
};

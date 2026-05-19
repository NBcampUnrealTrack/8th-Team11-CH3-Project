// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "NBC_GameState.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8 { Battle, Reward, GameOver, GameClear };

UCLASS()
class NBC_CH3_TEAMPROJECT_API ANBC_GameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;

	EGamePhase CurrentPhase = EGamePhase::Battle;
	int32 CurrentWave = 1;
	float ElapsedTime = 0.0f;
	float DifficultyMultiplier = 1.0f;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category="Score")
	int32 TotalScore = 0;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "NBC_Ch3_TeamProject/Public/System/NBC_GameState.h"

void ANBC_GameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentPhase == EGamePhase::Battle)
	{
		ElapsedTime += DeltaTime;

		// 매 Tick마다 난이도 배율 상승, 추후 밸런싱 
		DifficultyMultiplier = 1.0f + (CurrentWave * 0.5f) + (ElapsedTime * 0.01f);
	}
}

void ANBC_GameState::AddScore(int32 Score)
{
	TotalScore += Score;

	if (OnScoreChanged.IsBound())
	{
		OnScoreChanged.Broadcast(TotalScore);
	}
}
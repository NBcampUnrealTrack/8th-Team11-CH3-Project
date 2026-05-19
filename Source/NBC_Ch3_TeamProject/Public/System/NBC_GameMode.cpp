// Fill out your copyright notice in the Description page of Project Settings.


#include "NBC_Ch3_TeamProject/Public/System/NBC_GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "AI/WaveSpawnManager.h"

ANBC_GameMode::ANBC_GameMode()
{
	TargetKillCount = 30;
	CurrentKillCount = 0;
	MaxWaves = 5;

	// WaveSpawnManager 컴포넌트 생성 및 등록
	WaveSpawnManager = CreateDefaultSubobject<UWaveSpawnManager>(TEXT("WaveSpawnManager"));
}

void ANBC_GameMode::BeginPlay()
{
	Super::BeginPlay();

	ChangePhase(EGamePhase::Battle);

	// 테스트용
	WaveSpawnManager->SpawnZombie();
}

void ANBC_GameMode::OnMonsterKilled()
{
	ANBC_GameState* GS = GetGameState<ANBC_GameState>();

	if (!GS || GS->CurrentPhase != EGamePhase::Battle) return;

	++CurrentKillCount;

	if (OnKillCountChanged.IsBound())
	{
		OnKillCountChanged.Broadcast(CurrentKillCount);
	}

	int32 Score = FMath::RoundToInt(ScorePerKill * GS->DifficultyMultiplier);
	GS->AddScore(Score);

	UE_LOG(LogTemp, Log, TEXT("Monster Killed: %d / %d"), CurrentKillCount, TargetKillCount);

	if (CurrentKillCount >= TargetKillCount)
	{
		if (GS->CurrentWave >= MaxWaves)
		{
			ChangePhase(EGamePhase::GameClear);
		}
		else
		{
			ChangePhase(EGamePhase::Reward);
		}
	}
}

void ANBC_GameMode::ChangePhase(EGamePhase NewPhase)
{
	ANBC_GameState* GS = GetGameState<ANBC_GameState>();

	if (!GS) return;

	GS->CurrentPhase = NewPhase;

	switch (NewPhase)
	{
	case EGamePhase::Battle:
		CurrentKillCount = 0;
		TargetKillCount += 5;
		UE_LOG(LogTemp, Warning, TEXT("Phase Changed: Battle (Wave %d )"), GS->CurrentWave);
		break;

	case EGamePhase::Reward:
		UE_LOG(LogTemp, Warning, TEXT("Phase Changed: Rewawrd"));
		// 보상 포탈 따위 소환 
		break;

	case EGamePhase::GameOver:
		UE_LOG(LogTemp, Error, TEXT("Phase Changed: Game Over"));
		// 게임 정지 밑 사망 데이터, UI등 
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		break;

	case EGamePhase::GameClear:
		UE_LOG(LogTemp, Warning, TEXT("Phase Changed: Game Clear"));
		// 게임 정지 및 클리어 데이터, UI등 
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		break;
	}
}

void ANBC_GameMode::OnPlayerDied()
{
	ChangePhase(EGamePhase::GameOver);
}
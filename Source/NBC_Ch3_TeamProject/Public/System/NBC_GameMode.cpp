// Fill out your copyright notice in the Description page of Project Settings.


#include "NBC_Ch3_TeamProject/Public/System/NBC_GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "AI/WaveSpawnManager.h"
#include "Blueprint/UserWidget.h"

ANBC_GameMode::ANBC_GameMode()
{
	TargetKillCount = 5;
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

	int32 Score = FMath::RoundToInt(ScorePerKill * GS->DifficultyMultiplier);
	GS->TotalScore += Score;

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
		WaveSpawnManager->SpawnZombie();
		break;

	case EGamePhase::Reward:
		UE_LOG(LogTemp, Warning, TEXT("Phase Changed: Rewawrd"));
		// 보상 포탈 따위 소환 
		break;

	case EGamePhase::GameOver:
	{
		UE_LOG(LogTemp, Error, TEXT("Phase Changed: Game Over"));

		if (!GameOverWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("게임 오버 위젯 미등록"));
			return;
		}

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC) return;

		if (!GameOverWidgetInstance)
		{
			GameOverWidgetInstance = CreateWidget<UUserWidget>(PC, GameClearWidgetClass);
		}

		if (GameOverWidgetInstance)
		{
			GameOverWidgetInstance->AddToViewport();

			// 점수 업데이트
			UFunction* UpdateScoreFunc = GameOverWidgetInstance->FindFunction(FName("UpdateScoreUI"));

			if (UpdateScoreFunc)
			{
				struct FScoreParameters
				{
					int32 NewScore;
				};

				FScoreParameters Params;
				Params.NewScore = GS->TotalScore;

				GameOverWidgetInstance->ProcessEvent(UpdateScoreFunc, &Params);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("UpdateScoreUI 이벤트 발견 불가"))
			}

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(GameOverWidgetInstance->GetCachedWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;

			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
		break;
	}

	case EGamePhase::GameClear:
	{

		UE_LOG(LogTemp, Warning, TEXT("Phase Changed: Game Clear"));

		if (!GameClearWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("게임 클리어 위젯 미등록"));
			return;
		}

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC) return;

		if (!GameClearWidgetInstance)
		{
			GameClearWidgetInstance = CreateWidget<UUserWidget>(PC, GameClearWidgetClass);
		}

		if (GameClearWidgetInstance)
		{
			GameClearWidgetInstance->AddToViewport();

			// UI 점수 업데이트 함수 
			UFunction* UpdateScoreFunc = GameClearWidgetInstance->FindFunction(FName("UpdateScoreUI"));

			// 점수 업데이트 
			if (UpdateScoreFunc)
			{
				struct FScoreParameters
				{
					int32 NewScore;
				};

				FScoreParameters Params;
				Params.NewScore = GS->TotalScore;

				GameClearWidgetInstance->ProcessEvent(UpdateScoreFunc, &Params);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("위젯에서 UpdateScoreUI 이벤트를 찾을 수 없습니다."))
			}

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(GameClearWidgetInstance->GetCachedWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;

			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
		break;
	}
	
	}
}

void ANBC_GameMode::OnPlayerDied()
{
	ChangePhase(EGamePhase::GameOver);
}

void ANBC_GameMode::QuitGame()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (PC)
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
}

void ANBC_GameMode::RequestRestartGame()
{
	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());

	UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevelName));
}
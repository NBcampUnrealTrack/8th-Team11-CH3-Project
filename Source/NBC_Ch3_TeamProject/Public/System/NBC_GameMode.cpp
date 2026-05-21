// Fill out your copyright notice in the Description page of Project Settings.


#include "NBC_Ch3_TeamProject/Public/System/NBC_GameMode.h"
#include "Player/WeaponRewardComponent.h"
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

	WaveSpawnManager->SpawnZombie();
}
 
void ANBC_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	FTimerHandle TempTimerHandle;
	GetWorldTimerManager().SetTimer(
		TempTimerHandle,
		FTimerDelegate::CreateUObject(this, &ANBC_GameMode::InitializeLoadedGame, NewPlayer),
		0.2f,
		false
	);
}

void ANBC_GameMode::InitializeLoadedGame(APlayerController* TargetPlayer)
{
	if (TargetPlayer && TargetPlayer->GetPawn())
	{
		UWeaponRewardComponent* RewardComp = TargetPlayer->GetPawn()->FindComponentByClass<UWeaponRewardComponent>();

		if (RewardComp)
		{
			RewardComp->LoadWeaponsFromInstance();
		}
	}

	ChangePhase(EGamePhase::Battle);
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
		if (GS)
		{
			++GS->CurrentWave;
		}

		CurrentKillCount = 0;
		TargetKillCount += 5;
		UE_LOG(LogTemp, Warning, TEXT("Phase Changed: Battle (Wave %d )"), GS->CurrentWave);
		//WaveSpawnManager->SpawnZombie();
		break;

	case EGamePhase::Reward:
		UE_LOG(LogTemp, Warning, TEXT("Phase Changed: Rewawrd"));
		
		if (PortalClass)
		{
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC && PC->GetPawn())
			{
				AActor* PlayerActor = PC->GetPawn();

				// 바라보는 방향으로 포탈 생성 위치 잡기 
				FVector SpawnLocation = PlayerActor->GetActorLocation() + (PlayerActor->GetActorForwardVector() * 300.0f);

				// 포탈 높이 맞추기 
				SpawnLocation.Z = PlayerActor->GetActorLocation().Z;

				// 플레이어 정면으로 바라보게 설정
				FRotator SpawnRotation = PlayerActor->GetActorRotation();
				SpawnRotation.Yaw += 180.0f;

				// 포탈 스폰 
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				GetWorld()->SpawnActor<AActor>(PortalClass, SpawnLocation, SpawnRotation, SpawnParams);

				UE_LOG(LogTemp, Log, TEXT("GameMode: 보상 포탈 소환"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: PortalClass를 찾을 수 없습니다."))
		}
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
			GameOverWidgetInstance = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
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
				Params.NewScore = GS->GetCurrentScore();

				GameOverWidgetInstance->ProcessEvent(UpdateScoreFunc, &Params);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Can't find UpdateScoreUI"));
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
			UE_LOG(LogTemp, Warning, TEXT("Can't Find GameClear Widget"));
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
				Params.NewScore = GS->GetCurrentScore();

				GameClearWidgetInstance->ProcessEvent(UpdateScoreFunc, &Params);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Can't find UpdateScoreUI"))
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

	UE_LOG(LogTemp, Warning, TEXT("Restart Level"));

	UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevelName));
}
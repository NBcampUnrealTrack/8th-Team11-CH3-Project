// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NBC_Ch3_TeamProject/Public/System/NBC_GameState.h"
#include "NBC_GameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillCountChangedDelegate, int32, NewKillCount);

UCLASS()
class NBC_CH3_TEAMPROJECT_API ANBC_GameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ANBC_GameMode();

	// 페이즈 전환 
	void ChangePhase(EGamePhase NewPhase);

	// 몹이 죽었을 때 호출 
	UFUNCTION()
	void OnMonsterKilled();

	// 플레이어가 죽었을 떄 호출 
	void OnPlayerDied();

	UPROPERTY(BlueprintAssignable, Category = "Game Flow | UI")
	FOnKillCountChangedDelegate OnKillCountChanged;

	UFUNCTION(BlueprintPure, Category = "Game Flow")
	int32 GetCurrentKillCount() const { return CurrentKillCount; }

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void QuitGame();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void RequestRestartGame();

	virtual void PostLogin(APlayerController* NewPlayer) override;

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

	// 웨이브 스폰 매니저
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WaveSpawnManager")
	TObjectPtr<class UWaveSpawnManager> WaveSpawnManager;

	// 게임 클리어 UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameClearWidgetClass;

	UPROPERTY()
	UUserWidget* GameClearWidgetInstance;

	// 게임 오버 UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY()
	UUserWidget* GameOverWidgetInstance;

	// BP 포탈 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Flow | Spawn")
	TSubclassOf<AActor> PortalClass;

private:
	void InitializeLoadedGame(APlayerController* TargetPlayer);
};

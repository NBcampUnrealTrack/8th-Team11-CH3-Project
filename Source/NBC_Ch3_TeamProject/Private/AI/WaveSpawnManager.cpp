#include "AI/WaveSpawnManager.h"
#include "AI/ZombieSpawnVolume.h"
#include "AI/BossSpawnVolume.h"
#include "System/NBC_GameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AI/BossSpawnRow.h"

UWaveSpawnManager::UWaveSpawnManager()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UWaveSpawnManager::SpawnZombie()
{
	// 좀비 데이터 테이블에서 가져오기
	if (!ZombieDataTable.Num() || !GetWorld())
	{
		return;
	}

	// 현재 웨이브 가져오기
	int32 CurrentWave = 1;
	ANBC_GameState* GS = Cast<ANBC_GameState>(GetWorld()->GetGameState());
	if (GS)
	{
		CurrentWave = GS->CurrentWave;
	}

	// 모든 데이터 가져오기
	TArray<FZombieSpawnRow*> AllRows;
	static const FString ContextString(TEXT("No Zombie Data"));
	ZombieDataTable[CurrentWave - 1]->GetAllRows(ContextString, AllRows);

	// 모든 스폰 볼륨 찾아서 저장
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieSpawnVolume::StaticClass(), FoundVolumes);

	if (AllRows.IsEmpty() || FoundVolumes.Num() <= 0)
	{
		return;
	}

	// 좀비 종류마다 
	for (FZombieSpawnRow* Row : AllRows)
	{
		if (!Row)
		{
			break;
		}

		// 스폰 위치마다
		for (const auto& Volume : FoundVolumes)
		{
			UClass* ZombieClass = Row->ZombieClass.Get();
			AZombieSpawnVolume* ZombieSpawnVolume = Cast<AZombieSpawnVolume>(Volume);
			if (!ZombieClass || !ZombieSpawnVolume)
			{
				break;
			}

			for (int32 i = 0; i < Row->SpawnNum / FoundVolumes.Num(); ++i)
			{
				ZombieSpawnVolume->SpawnZombie(ZombieClass);
			}
		}
	
	}
}

void UWaveSpawnManager::SpawnBoss()
{
	if (!BossDataTable || !GetWorld())
	{
		return;
	}

	// 보스 데이터 테이블에서 딱 하나의 Row만 가져오기
	// 보스 데이터가 들어있는 행의 이름을 직접 찾음
	static const FString ContextString(TEXT("No Boss Data"));
	FBossSpawnRow* BossRow = BossDataTable->FindRow<FBossSpawnRow>(FName("Boss"), ContextString);

	// 이름 대신 AllRows 이용
	//if (!BossRow)
	//{
	//	TArray<FBossSpawnRow*> AllBossRows;
	//	BossDataTable->GetAllRows(ContextString, AllBossRows);
	//	if (!AllBossRows.IsEmpty())
	//	{
	//		BossRow = AllBossRows[0];
	//	}
	//}

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABossSpawnVolume::StaticClass(), FoundVolumes);

	if (FoundVolumes.IsEmpty())
	{
		return;
	}

	ABossSpawnVolume* TargetVolume = Cast<ABossSpawnVolume>(FoundVolumes[0]);
	UClass* BossClass = BossRow->BossClass.Get();

	if (TargetVolume && BossClass)
	{
		TargetVolume->BossZombie(BossClass);
	}
}

void UWaveSpawnManager::BeginPlay()
{
	Super::BeginPlay();

}


void UWaveSpawnManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


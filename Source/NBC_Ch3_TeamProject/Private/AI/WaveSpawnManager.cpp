#include "AI/WaveSpawnManager.h"
#include "AI/ZombieSpawnVolume.h"
#include "System/NBC_GameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

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

void UWaveSpawnManager::BeginPlay()
{
	Super::BeginPlay();

}


void UWaveSpawnManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


#include "AI/ZombieSpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AZombieSpawnVolume::AZombieSpawnVolume()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);
}

// 일단 테스트 용으로 여기에 기본 구현(void), 나중에 GameState 등에서 호출해서 사용하게 바꿀 예정
void AZombieSpawnVolume::SpawnZombie()
{
	// 스폰 설정 저장
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	// 모든 스폰 볼륨 찾아서 저장(GameState에서 사용)
	//TArray<AActor*> FoundVolumes;
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieSpawnVolume::StaticClass(), FoundVolumes);

	//// 볼륨마다 10마리씩 스폰
	//for (int32 i = 0; i < 10; ++i)
	//{
	//	if (FoundVolumes.Num() > 0)
	//	{
	//		for (const auto& Zombie : FoundVolumes)
	//		{
	//			// 스폰
	//		}
	//	}
	//}

	// 좀비 데이터 테이블에서 가져오기
	if (!ZombieDataTable.Num())
	{
		return;
	}

	// 모든 데이터 가져오기
	TArray<FZombieSpawnRow*> AllRows;
	static const FString ContextString(TEXT("No Zombie Data"));
	ZombieDataTable[0]->GetAllRows(ContextString, AllRows);

	if (AllRows.IsEmpty())
	{
		return;
	}

	UClass* ZombieClass = AllRows[0]->ZombieClass.Get();

	if (!ZombieClass)
	{
		return;
	}

	ACharacter* SpawnedZombie = GetWorld()->SpawnActor<ACharacter>(
		ZombieClass,
		GetPointInVolume(),
		FRotator::ZeroRotator,
		SpawnParams);

	if (SpawnedZombie)
	{
		// 스폰 후 사운드, 이펙트 등등..
	}

}

void AZombieSpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	for (int32 i = 0; i < 10; ++i)
	{
		// 바로 호출하지 말고, 0.5초 뒤에 실행되도록 지연
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle,
			this,
			&AZombieSpawnVolume::SpawnZombie,
			0.5f,
			false);
	}

}

FVector AZombieSpawnVolume::GetPointInVolume() const
{
	FVector BoxOrigin = SpawningBox->GetComponentLocation();

	// 박스 중앙 위치만 불러옴. 추후 조정

	return BoxOrigin;
}




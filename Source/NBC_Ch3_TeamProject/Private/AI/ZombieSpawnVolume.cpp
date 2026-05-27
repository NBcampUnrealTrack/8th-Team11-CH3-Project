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

void AZombieSpawnVolume::SpawnZombie(TSubclassOf<ACharacter> ZombieClass)
{
	// 스폰 설정 저장
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
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


}

FVector AZombieSpawnVolume::GetPointInVolume() const
{
	FVector BoxOrigin = SpawningBox->GetComponentLocation();

	// 박스 중앙 위치만 불러옴. 추후 조정

	return BoxOrigin;
}




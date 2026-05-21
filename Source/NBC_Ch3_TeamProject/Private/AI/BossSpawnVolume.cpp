#include "AI/BossSpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ABossSpawnVolume::ABossSpawnVolume()
{

	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);
}

void ABossSpawnVolume::BossZombie(TSubclassOf<ACharacter> BossClass)
{
	// 스폰 설정 저장
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (!BossClass)
	{
		return;
	}

	ACharacter* SpawnedBoss = GetWorld()->SpawnActor<ACharacter>(
		BossClass,
		GetPointInVolume(),
		FRotator::ZeroRotator,
		SpawnParams);

	if (SpawnedBoss)
	{
		// 스폰 후 사운드, 이펙트 등등..
	}

}

void ABossSpawnVolume::BeginPlay()
{
	Super::BeginPlay();


}

FVector ABossSpawnVolume::GetPointInVolume() const
{
	FVector BoxOrigin = SpawningBox->GetComponentLocation();

	// 박스 중앙 위치만 불러옴. 추후 조정

	return BoxOrigin;
}


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawnRow.h"
#include "ZombieSpawnVolume.generated.h"

class UBoxComponent;

UCLASS()
class NBC_CH3_TEAMPROJECT_API AZombieSpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	AZombieSpawnVolume();

	// 좀비 스폰
	void SpawnZombie(TSubclassOf<ACharacter> ZombieClass);

protected:
	// 스폰 액터 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	TObjectPtr<UBoxComponent> SpawningBox;


	FTimerHandle SpawnTimerHandle;

	virtual void BeginPlay() override;

	// 스폰 포인트
	FVector GetPointInVolume() const;
};

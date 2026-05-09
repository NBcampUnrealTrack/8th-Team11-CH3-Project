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
	//ACharacter* SpawnZombie();
	void SpawnZombie();

protected:
	// 스폰 액터 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	TObjectPtr<UBoxComponent> SpawningBox;
	// 스폰 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<TObjectPtr<UDataTable>> ZombieDataTable;

	FTimerHandle SpawnTimerHandle;

	virtual void BeginPlay() override;

	// 스폰 포인트
	FVector GetPointInVolume() const;
};

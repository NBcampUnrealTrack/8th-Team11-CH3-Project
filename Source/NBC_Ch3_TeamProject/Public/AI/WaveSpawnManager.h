#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WaveSpawnManager.generated.h"

class AZombieSpawnVolume;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NBC_CH3_TEAMPROJECT_API UWaveSpawnManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWaveSpawnManager();

	void SpawnZombie();

protected:
	virtual void BeginPlay() override;

	// 스폰 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<TObjectPtr<UDataTable>> ZombieDataTable;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};

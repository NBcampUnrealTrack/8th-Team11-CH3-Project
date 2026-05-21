#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossSpawnVolume.generated.h"

class UBoxComponent;

UCLASS()
class NBC_CH3_TEAMPROJECT_API ABossSpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:
	ABossSpawnVolume();

	// 보스 스폰
	void BossZombie(TSubclassOf<ACharacter> BossClass);

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

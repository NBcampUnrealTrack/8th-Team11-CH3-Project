#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BossSpawnRow.generated.h"

USTRUCT(BlueprintType)
struct FBossSpawnRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 보스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BossName;
	// 보스 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACharacter> BossClass;
	// 스폰 마리수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnNum = 0;
};

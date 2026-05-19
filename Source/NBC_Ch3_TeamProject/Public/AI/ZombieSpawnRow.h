#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ZombieSpawnRow.generated.h"


USTRUCT(BlueprintType)
struct FZombieSpawnRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 좀비 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;
	// 좀비 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACharacter> ZombieClass;
	// 스폰 마리수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnNum = 0;
};

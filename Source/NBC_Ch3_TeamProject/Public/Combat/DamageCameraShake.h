// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LegacyCameraShake.h"
#include "DamageCameraShake.generated.h"

// [장식 추가] 플레이어 피격 시 재생하는 카메라 셰이크. 생성자에서 진동 파라미터 설정.
UCLASS()
class NBC_CH3_TEAMPROJECT_API UDamageCameraShake : public ULegacyCameraShake
{
	GENERATED_BODY()

public:
	UDamageCameraShake();
};

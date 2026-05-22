// Fill out your copyright notice in the Description page of Project Settings.


#include "System/NBC_GameInstance.h"

void UNBC_GameInstance::ClearSavedData()
{
	SavedWeaponClasses.Empty();
	SavedCurrentWeaponIndex = -1;
	// [장식 수정] 진행도(NormalLevelsCleared, bIsInRewardLevel)는 여기서 리셋 금지.
	// 이 함수가 SaveWeaponsToInstance에서 매번 호출되어 보상맵 토글이 풀리는 버그가 있었음.
	// 진행도 초기화는 ResetProgression() 사용.
}

void UNBC_GameInstance::ResetProgression()
{
	NormalLevelsCleared = 0;
	bIsInRewardLevel = false;
}
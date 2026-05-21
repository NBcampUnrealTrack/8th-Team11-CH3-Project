// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "System/BaseDataAsset.h"
#include "CardDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class NBC_CH3_TEAMPROJECT_API UCardDataAsset : public UBaseDataAsset
{
	GENERATED_BODY()
public:

	// 카드 스탯들
	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Stat")
	float MaxHPModifier;

	// 이동속도 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Stat")
	float MoveSpeed;


};

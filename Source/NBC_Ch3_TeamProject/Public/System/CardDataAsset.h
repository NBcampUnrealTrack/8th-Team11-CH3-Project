// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CardDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class NBC_CH3_TEAMPROJECT_API UCardDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	// 카드 정보들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Info")
	FText CardName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Info")
	UTexture2D* CardIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Info")
	FText CardDescription;

	// 카드 스탯들
	// 공격력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Stat")
	float AttackPowerModifier;

	// 연사 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Stat")
	float RateOfFireModifier;

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Stat")
	float MaxHPModifier;

	// 탄창량 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Stat")
	float AmmoCapacityModifier;


};

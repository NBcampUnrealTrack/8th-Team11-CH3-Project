// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "Rifle.generated.h"

/**
 * 
 */
UCLASS()
class NBC_CH3_TEAMPROJECT_API ARifle : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	ARifle();
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere)
	float MinSpread;
	UPROPERTY(EditAnywhere)
	float MaxSpread;
	UPROPERTY(EditAnywhere)
	float SpreadIncrease;
	UPROPERTY(EditAnywhere)
	float CurrentSpread;
	UPROPERTY(EditAnywhere)
	float SpreadRecovery;
	
	virtual void Fire() override;
};

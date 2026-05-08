// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/BaseWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class NBC_CH3_TEAMPROJECT_API AShotgun : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	AShotgun();
	
	UPROPERTY(EditAnywhere)
	int32 PelletCount;
	
	UPROPERTY(EditAnywhere)
	float SpreadAngle;
	
	virtual void Fire() override;
};

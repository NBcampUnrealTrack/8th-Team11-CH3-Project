// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseDataAsset.generated.h"


UCLASS()
class NBC_CH3_TEAMPROJECT_API UBaseDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Base UI Data")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base UI Data")
	FText DisplayDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base UI Data")
	TObjectPtr<UTexture2D> Icon;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardSelectionWidget.generated.h"

class UCardSlotWidget;
class UBaseDataAsset;

UCLASS()
class NBC_CH3_TEAMPROJECT_API UCardSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 위젯 생성 시 카드 슬롯들의 클릭 이벤트 구독
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UCardSlotWidget* CardSlot_0;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UCardSlotWidget* CardSlot_1;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UCardSlotWidget* CardSlot_2;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card System", meta = (AllowPrivate = "true"))
	TArray<UBaseDataAsset*> TotalCardPool;

public:
	// 카드 풀 초기화
	UFUNCTION(BlueprintCallable, Category = "Card System")
	void SetTotalCardPool(const TArray<UBaseDataAsset*> CardPool) { TotalCardPool = CardPool; }

	// 카드 뽑기 
	UFUNCTION(BlueprintCallable, Category = "Card System")
	void RefreshCardSelection();

private:
	UFUNCTION()
	void HandleCardSelected(UBaseDataAsset* SelectedData);
};

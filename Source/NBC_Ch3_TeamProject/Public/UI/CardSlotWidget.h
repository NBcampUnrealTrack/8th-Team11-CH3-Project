// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardSlotWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UCardDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardSelected, UCardDataAsset*, SelectedData);

UCLASS()
class NBC_CH3_TEAMPROJECT_API UCardSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescText;

	UPROPERTY(meta = (BindWidget))
	UButton* CardButton;

	UPROPERTY(meta = (BindWidget))
	UImage* CardIcon;

	// 카드 데이터 초기화
	UFUNCTION(BlueprintCallable, Category = "Card UI")
	void SetupCard(UCardDataAsset* CardData);

	UPROPERTY(BlueprintAssignable, Category = "Card UI")
	FOnCardSelected OnCardSelected;

private:

	// 버튼 클릭 시 실행될 함수
	UFUNCTION()
	void HandleButtonClicked();

	// 현재 슬롯이 들고 있는 카드 데이터
	UPROPERTY()
	UCardDataAsset* CurrentCardData;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CardSlotWidget.h"
#include "System/BaseDataAsset.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"

void UCardSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 클릭 이벤트에 C++ 함수 바인드
	if (CardButton)
	{
		CardButton->OnClicked.AddDynamic(this, &UCardSlotWidget::HandleButtonClicked);
	}
}

void UCardSlotWidget::SetupCard(UBaseDataAsset* BaseData)
{
	if (!BaseData) return;

	CurrentCardData = BaseData;

	if (NameText)
	{
		NameText->SetText(BaseData->DisplayName);
	}

	if (DescText)
	{
		DescText->SetText(BaseData->DisplayDescription);
	}

	if (CardIcon)
	{
		CardIcon->SetBrushFromTexture(BaseData->Icon);
	}
}

void UCardSlotWidget::HandleButtonClicked()
{
	if (OnCardSelected.IsBound() && CurrentCardData)
	{
		OnCardSelected.Broadcast(CurrentCardData);
	}
}
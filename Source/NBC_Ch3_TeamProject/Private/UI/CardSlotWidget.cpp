// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CardSlotWidget.h"
#include "NBC_Ch3_TeamProject/Public/System/CardDataAsset.h"
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

void UCardSlotWidget::SetupCard(UCardDataAsset* CardData)
{
	if (!CardData) return;

	CurrentCardData = CardData;

	if (NameText)
	{
		NameText->SetText(CardData->CardName);
	}

	if (DescText)
	{
		DescText->SetText(CardData->CardDescription);
	}

	if (CardIcon)
	{
		CardIcon->SetBrushFromTexture(CardData->CardIcon);
	}
}

void UCardSlotWidget::HandleButtonClicked()
{
	if (OnCardSelected.IsBound() && CurrentCardData)
	{
		OnCardSelected.Broadcast(CurrentCardData);
	}
}
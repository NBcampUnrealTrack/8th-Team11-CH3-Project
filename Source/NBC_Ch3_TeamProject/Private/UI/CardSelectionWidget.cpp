// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CardSelectionWidget.h"
#include "NBC_Ch3_TeamProject/Public/System/CardDataAsset.h"
#include "NBC_Ch3_TeamProject/Public/UI/CardSlotWidget.h"
#include "Kismet/KismetArrayLibrary.h"

void UCardSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CardSlot_0)
		CardSlot_0->OnCardSelected.AddDynamic(this, &UCardSelectionWidget::HandleCardSelected);
	if (CardSlot_1)
		CardSlot_1->OnCardSelected.AddDynamic(this, &UCardSelectionWidget::HandleCardSelected);
	if(CardSlot_2)
		CardSlot_2->OnCardSelected.AddDynamic(this, &UCardSelectionWidget::HandleCardSelected);
}

void UCardSelectionWidget::RefreshCardSelection()
{
	if (TotalCardPool.Num() < 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("카드 풀에 카드가 부족합니다!"));
		return;
	}

	TArray<UCardDataAsset*> TempPool = TotalCardPool;

	// 카드 덱을 섞어줌 
	const int32 LastIndex = TempPool.Num() - 1;
	for (int32 i = 0; i < LastIndex; ++i)
	{
		int32 IndexToSwap = FMath::RandRange(0, LastIndex);
		if (i != IndexToSwap)
			TempPool.Swap(i, IndexToSwap);
	}

	if (CardSlot_0)
		CardSlot_0->SetupCard(TempPool[0]);
	if (CardSlot_1)
		CardSlot_1->SetupCard(TempPool[1]);
	if (CardSlot_2)
		CardSlot_2->SetupCard(TempPool[2]);
}

void UCardSelectionWidget::HandleCardSelected(UCardDataAsset* SelectedData)
{
	if (!SelectedData) return;

	// 선택 카드를 이용하여 플레이어 능력치 증강.
	// 플레이어 측 함수 필요 

	// 선택 카드 로그 찍기 

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("선택된 카드: %s"), *SelectedData->CardName.ToString()));

	RemoveFromParent();
}
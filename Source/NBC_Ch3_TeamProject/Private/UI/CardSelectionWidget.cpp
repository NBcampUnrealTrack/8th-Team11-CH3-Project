// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CardSelectionWidget.h"
#include "NBC_Ch3_TeamProject/Public/Player/WeaponRewardComponent.h"
#include "NBC_Ch3_TeamProject/Public/Combat/HealthComponent.h"
#include "NBC_Ch3_TeamProject/Public/System/CardDataAsset.h"
#include "NBC_Ch3_TeamProject/Public/Combat/WeaponConfig.h"
#include "NBC_Ch3_TeamProject/Public/UI/CardSlotWidget.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Player/PlayerCharacter.h"

void UCardSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}

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

	TArray<UBaseDataAsset*> TempPool = TotalCardPool;

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

void UCardSelectionWidget::HandleCardSelected(UBaseDataAsset* SelectedData)
{
	if (!SelectedData) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;

	// 선택 카드를 이용하여 플레이어 능력치 증강.
	// 플레이어 측 함수 필요 

	// 카드 데이터인 경우
	if (UCardDataAsset* CardData = Cast<UCardDataAsset>(SelectedData))
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(PC->GetPawn());
		if (!Player) return;

		UHealthComponent* HealthComp = Player->FindComponentByClass<UHealthComponent>();

		if (HealthComp)
		{
			HealthComp->IncreaseMaxHealth(CardData->MaxHPModifier);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CardWidget: Can'f find HealthComponent from Player"));
		}

		Player->IncreaseMovementSpeed(CardData->MoveSpeed);
	}

	// 무기 데이터인 경우
	if (UWeaponConfig* WeaponData = Cast<UWeaponConfig>(SelectedData))
	{
		UWeaponRewardComponent* RewardComp = PC->GetPawn()->FindComponentByClass<UWeaponRewardComponent>();

		if (RewardComp)
		{
			RewardComp->ApplyWeaponConfigReward(WeaponData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Can't Find WeaponRewardComponent"));
		}

	}


	// 선택 카드 로그 찍기 
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Selected Card: %s"), *SelectedData->DisplayName.ToString()));

	RemoveFromParent();

	if (PC)
	{
		PC->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
}
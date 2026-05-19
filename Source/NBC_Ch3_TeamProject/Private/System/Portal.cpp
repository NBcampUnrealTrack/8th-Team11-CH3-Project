// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Portal.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NBC_Ch3_TeamProject/Public/System/NBC_GameInstance.h"
#include "NBC_Ch3_TeamProject/Public/System/NBC_GameState.h"

// Sets default values
APortal::APortal()
{	
	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("USceneComponent"));
	SetRootComponent(RootSceneComp);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootSceneComp);
	CollisionSphere->SetSphereRadius(150.0f);

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(RootSceneComp);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnPortalOverlap);
}

void APortal::OnPortalOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
							  int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		UNBC_GameInstance* GI = Cast<UNBC_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		ANBC_GameState* GS = GetWorld()->GetGameState<ANBC_GameState>();

		// 데이터 저장 
		if (GI && GS)
		{
			GI->SetSavedTotalScore(GS->GetCurrentScore());
			GI->SetSavedCurrentWave(GS->CurrentWave);
			// 추후 카드 저장
		}

		// 상점 레벨로 이동
		if (!TargetLevelName.IsNone())
		{
			UGameplayStatics::OpenLevel(this, TargetLevelName);
		}
	}
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Portal.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NBC_Ch3_TeamProject/Public/System/NBC_GameInstance.h"
#include "NBC_Ch3_TeamProject/Public/System/NBC_GameState.h"
#include "Player/WeaponRewardComponent.h"
#include "Player/PlayerCharacter.h"

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

		// [장식 복원] 점수와 웨이브 정보 저장
		if (GI && GS)
		{
			GI->SetSavedTotalScore(GS->GetCurrentScore());
			GI->SetSavedCurrentWave(GS->CurrentWave);
			// [장식 복원] 무기 카드 저장
		}

		// [장식 복원] 무기 세이브
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
		{
			UWeaponRewardComponent* RewardComp = Player->FindComponentByClass<UWeaponRewardComponent>();
			if (RewardComp)
			{
				RewardComp->SaveWeaponsToInstance();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("PORTAL: Can't Find UWeaponRewardComponent"));
			}

		}


		// [장식 수정] 노멀 ↔ 보상 토글, 노멀 N회 후 보스로 분기
		FName NextLevel = NAME_None;
		if (GI)
		{
			// [장식 진단] 분기 직전 상태 로그 — 문제 재현 시 주석 해제
			//UE_LOG(LogTemp, Warning, TEXT("[PORTAL] 진입 BEFORE: IsInRewardLevel=%s, NormalLevelsCleared=%d, LoopCount=%d, NormalLevelName=%s, RewardLevelName=%s, BossLevelName=%s"),
			//	GI->IsInRewardLevel() ? TEXT("true") : TEXT("false"),
			//	GI->GetNormalLevelsCleared(),
			//	NormalLevelLoopCount,
			//	*NormalLevelName.ToString(),
			//	*RewardLevelName.ToString(),
			//	*BossLevelName.ToString());

			if (!GI->IsInRewardLevel())
			{
				// 노멀맵에 있었음 → 보상맵으로 이동, 노멀 클리어 카운트 증가
				NextLevel = RewardLevelName;
				GI->IncNormalLevelsCleared();
				GI->SetInRewardLevel(true);
				//UE_LOG(LogTemp, Warning, TEXT("[PORTAL] 분기: 노멀 → 보상"));
			}
			else
			{
				// 보상맵에 있었음 → 누적 클리어 보고 다음 노멀 또는 보스
				const int32 Cleared = GI->GetNormalLevelsCleared();
				if (Cleared < NormalLevelLoopCount)
				{
					NextLevel = NormalLevelName;
					GI->SetInRewardLevel(false);
					//UE_LOG(LogTemp, Warning, TEXT("[PORTAL] 분기: 보상 → 노멀 (Cleared=%d)"), Cleared);
				}
				else
				{
					NextLevel = BossLevelName;
					GI->ResetNormalLevelsCleared();
					GI->SetInRewardLevel(false);
					//UE_LOG(LogTemp, Warning, TEXT("[PORTAL] 분기: 보상 → 보스 (Cleared=%d)"), Cleared);
				}
			}

			// [장식 진단] 분기 직후 상태 로그 — 문제 재현 시 주석 해제
			//UE_LOG(LogTemp, Warning, TEXT("[PORTAL] 진입 AFTER : IsInRewardLevel=%s, NormalLevelsCleared=%d, NextLevel=%s"),
			//	GI->IsInRewardLevel() ? TEXT("true") : TEXT("false"),
			//	GI->GetNormalLevelsCleared(),
			//	*NextLevel.ToString());
		}

		if (!NextLevel.IsNone())
		{
			UGameplayStatics::OpenLevel(this, NextLevel);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PORTAL: NextLevel is None — NormalLevelName/BossLevelName 디테일 확인 필요"));
		}
	}
}


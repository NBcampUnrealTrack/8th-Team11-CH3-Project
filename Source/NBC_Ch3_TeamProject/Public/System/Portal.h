// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

UCLASS()
class NBC_CH3_TEAMPROJECT_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortal();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* RootSceneComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* PortalMesh;


	// 시작 레벨 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	FName StartLevel;

	// [장식 수정] 단일 TargetLevelName → 노멀/보상/보스 레벨 분기 멤버로 교체
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FName NormalLevelName;

	// [장식 추가] 보상맵 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FName RewardLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FName BossLevelName;

	// 보스로 넘어가기까지 반복할 노멀 레벨 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level", meta=(ClampMin="1"))
	int32 NormalLevelLoopCount = 5;

	UFUNCTION()
	void OnPortalOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
						 int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

class USkeletalMeshComponent;

UCLASS()
class NBC_CH3_TEAMPROJECT_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
public:	
	ABaseWeapon();
	
	UPROPERTY(visibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent *WeaponMesh;

	//무기 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	FName WeaponName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Stat")
	float Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Stat")
	float FireRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Stat")
	float Range;
	
	// 탄약 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Ammp")
	int32 MaxBulletCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Ammo")
	int32 CurrentBulletCount;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
	bool bIsFire = true;
	
	//과열
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|OverHeat")
	int32 MaxReloadCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|OverHeat")
	int32 CurrentReloadCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|OverHeat")
	bool bIsOverHeat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|OverHeat")
	float OverheatCooldown;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Aim")
	float AimingSpread;
	
	UFUNCTION(BlueprintCallable)
	virtual void Fire();
	
	void Reload();
	void OnOverHeatEnd();
	
	FTimerHandle FireTimer;
	FTimerHandle OverheatTimer;
	
	void ResetFireCooldown();
};

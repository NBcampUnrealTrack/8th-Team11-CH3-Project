// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Rifle.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacter.h"

ARifle::ARifle()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Damage = 10.0f;
	FireRate = 0.1f;
	MaxBulletCount = 30;
	MinSpread = 1.0f;
	MaxSpread = 8.0f;
	SpreadIncrease = 0.5f;
	SpreadRecovery = 1.0f;
	
	CurrentSpread = MinSpread;
}

void ARifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CurrentSpread > MinSpread)
	{
		CurrentSpread -= SpreadRecovery * DeltaTime;
		CurrentSpread = FMath::Max(CurrentSpread, MinSpread);
	}
}

void ARifle::Fire()
{
	// if (!bIsFire) return;
	// if (CurrentBulletCount <= 0) return;
	// if (bIsOverHeat) return;
	//
	// bool bOwnerAiming = false;
	// if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner()))
	// {
	// 	bOwnerAiming = PlayerCharacter->bIsAiming;
	// }
	//
	// FVector Start = GetActorLocation();
	//
	// float ActualSpread = CurrentSpread;
	// if (bOwnerAiming)
	// {
	// 	ActualSpread *= AimingSpread;
	// }
	//
	// FVector RandomDir = FMath::VRandCone(
	// 	GetActorForwardVector(),
	// 	FMath::DegreesToRadians(ActualSpread));
	//
	// FVector End = Start + (RandomDir * 5000.0f);
	//
	// FHitResult HitResult;
	// FCollisionQueryParams Params;
	// Params.AddIgnoredActor(this);
	// Params.AddIgnoredActor(GetOwner());
	//
	// bool bHit = GetWorld()->LineTraceSingleByChannel(
	// 	HitResult,
	// 	Start,
	// 	End,
	// 	ECC_Visibility,
	// 	Params);
	//
	// DrawDebugLine(GetWorld(), Start, End,
	// 	bHit ? FColor::Red : FColor::Green,
	// 	false, 1.0f, 0, 1.0f);
	//
	// if (bHit && HitResult.GetActor())
	// {
	// 	UGameplayStatics::ApplyDamage(
	// 		HitResult.GetActor(),
	// 		Damage,
	// 		GetOwner()->GetInstigatorController(),
	// 		this,
	// 		UDamageType::StaticClass());
	// }
	//
	// CurrentSpread = FMath::Min(CurrentSpread + SpreadIncrease, MaxSpread);
	//
	//
	//
	// CurrentBulletCount--;
	// if (CurrentBulletCount <= 0)
	// {
	// 	Reload();
	// }
	// bIsFire = false;
	// GetWorld()->GetTimerManager().SetTimer(
	// 	FireTimer,
	// 	this,
	// 	&ABaseWeapon::ResetFireCooldown,
	// 	FireRate,
	// 	false);
}

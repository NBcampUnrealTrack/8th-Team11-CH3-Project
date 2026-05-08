// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Shotgun.h"
#include "Kismet/GameplayStatics.h"

AShotgun::AShotgun()
{
	Damage = 15.0f;
	FireRate = 1.0f;
	MaxBulletCount = 1;
	PelletCount = 8;
	SpreadAngle = 20.0f;
}

void AShotgun::Fire()
{
	if (!bIsFire) return;
	if (CurrentBulletCount <= 0) return;
	
	FVector Start = GetActorLocation();
	FVector BaseDirection = GetActorForwardVector();
	
	for (int i = 0; i < PelletCount; ++i)
	{
		FVector RandomDir = FMath::VRandCone(
			BaseDirection,
			FMath::DegreesToRadians(SpreadAngle));
		
		FVector End = Start + (RandomDir * 5000.0f);
		
		//라인트레이스
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(GetOwner());
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult, Start, End, ECC_Visibility, Params);
		
		DrawDebugLine(GetWorld(), Start, End,
			bHit ? FColor::Red : FColor::Green,
			false, 1.0f, 0, 1.0f);
		
		if (bHit && HitResult.GetActor())
		{
			UGameplayStatics::ApplyDamage(
				HitResult.GetActor(), Damage,
				GetOwner()->GetInstigatorController(),
				this, UDamageType::StaticClass());
		}
	}
	CurrentBulletCount--;
	bIsFire = false;
	GetWorld()->GetTimerManager().SetTimer(
		FireTimer, 
		this,
		&ABaseWeapon::ResetFireCooldown,
		FireRate, 
		false);
}

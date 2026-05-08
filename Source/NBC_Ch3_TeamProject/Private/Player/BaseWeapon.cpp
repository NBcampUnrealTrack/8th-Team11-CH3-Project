// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseWeapon.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	
	//무기 콜리젼
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	Damage = 10.0f;
	FireRate = 0.2f;
	MaxBulletCount = 30;
	
	MaxReloadCount = 3;
	CurrentReloadCount = 0;
	bIsOverHeat = false;
	OverheatCooldown = 10.0f;
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	CurrentBulletCount = MaxBulletCount;
}

void ABaseWeapon::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("Fire called! bCanFire: %d, Bullets: %d"), 
		bIsFire, CurrentBulletCount);
	
	if (!bIsFire) return;
	if (CurrentBulletCount <= 0) return;
	if (bIsOverHeat) return;
	
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Fire");
	
	//발사위치
	FVector Start = GetActorLocation();
	//발사 방향
	FVector End = Start + (GetActorForwardVector() * 5000.0f);
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(GetOwner());
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, Start, End, ECC_Visibility, CollisionParams);
	
	DrawDebugLine(GetWorld(), Start, End, 
	   bHit ? FColor::Red : FColor::Green, 
	   false, 1.0f, 0, 1.0f);
	
	if (bHit && HitResult.GetActor())
	{
		UGameplayStatics::ApplyDamage(
			HitResult.GetActor(),
			Damage,
			GetOwner()->GetInstigatorController(),
			this,
			UDamageType::StaticClass());
	}
	
	CurrentBulletCount--;
	if (CurrentBulletCount <= 0)
	{
		Reload();
	}
	
	bIsFire = false;
	
	GetWorld()->GetTimerManager().SetTimer(
		FireTimer,
		this,
		&ABaseWeapon::ResetFireCooldown,
		FireRate);
}

void ABaseWeapon::Reload()
{
	CurrentBulletCount = MaxBulletCount;
	UE_LOG(LogTemp, Warning, TEXT("Reload! Count: %d / %d"), 
		CurrentReloadCount, MaxReloadCount);
	if (CurrentReloadCount >= MaxReloadCount)
	{
		// 과열 진입!
		bIsOverHeat = true;
        
		UE_LOG(LogTemp, Warning, TEXT("OVERHEATED! %f seconds cooldown"), OverheatCooldown);
        
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 3.0f, FColor::Red,
				FString::Printf(TEXT("OVERHEATED! %.0fs cooldown"), OverheatCooldown));
		}
        
		// 10초 후 과열 해제
		GetWorld()->GetTimerManager().SetTimer(
			OverheatTimer, this,
			&ABaseWeapon::OnOverHeatEnd,
			OverheatCooldown, false);
        
		return;
	}
	
	CurrentBulletCount = MaxBulletCount;
    
	UE_LOG(LogTemp, Warning, TEXT("Ammo refilled: %d"), CurrentBulletCount);
	
}

void ABaseWeapon::OnOverHeatEnd()
{
	bIsOverHeat = false;
	CurrentReloadCount = 0;
	CurrentBulletCount = MaxBulletCount;
    
	UE_LOG(LogTemp, Warning, TEXT("Overheat ended! Ready to fire"));
    
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.0f, FColor::Green,
			TEXT("Weapon cooled down!"));
	}
}

void ABaseWeapon::ResetFireCooldown()
{
	bIsFire = true;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
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
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	CurrentBulletCount = MaxBulletCount;
}

void ABaseWeapon::Fire()
{
	if (!bIsFire) return;
	if (CurrentBulletCount <= 0) return;
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
	bIsFire = false;
	
	GetWorld()->GetTimerManager().SetTimer(
		FireTimer,
		this,
		&ABaseWeapon::ResetFireCooldown,
		FireRate);
}

void ABaseWeapon::ResetFireCooldown()
{
	bIsFire = true;
}

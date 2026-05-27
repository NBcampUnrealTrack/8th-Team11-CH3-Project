// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Combat/WeaponComponent.h"
#include "GameFramework/Controller.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	
	//무기 콜리젼
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	OverheatParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("OverHeatParticle"));
	OverheatParticleSystem->SetupAttachment(WeaponMesh);
	OverheatParticleSystem->SetAutoActivate(false);
	
	Damage = 10.0f;
	FireRate = 0.2f;
	MaxBulletCount = 30;
	
	MaxReloadCount = 3;
	CurrentReloadCount = 0;
	bIsOverHeat = false;
	OverheatCooldown = 10.0f;
	AimingSpread = 0.3f;
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	CurrentBulletCount = MaxBulletCount;
	WeaponMesh->SetRelativeScale3D(MeshScale);
	WeaponMesh->SetRelativeRotation(MeshRotation);
}

void ABaseWeapon::Fire()
{
	// 데드 코드 확인 후 본체 주석 — WeaponComponent로 단일화. BP/AnimNotify가 호출하면 아래 위임 경로로 라우팅.
	/*
	if (!bIsFire) return;
	if (CurrentBulletCount <= 0) return;
	if (bIsOverHeat) return;

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Fire");

	FVector Start = GetActorLocation();
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

	CurrentBulletCount = FMath::Max(CurrentBulletCount - 1, 0);

	bIsFire = false;

	GetWorld()->GetTimerManager().SetTimer(
		FireTimer,
		this,
		&ABaseWeapon::ResetFireCooldown,
		FireRate);
	*/

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UWeaponComponent* WC = OwnerActor->FindComponentByClass<UWeaponComponent>();
	if (!WC) return;

	const FVector Muzzle = WeaponMesh && WeaponMesh->DoesSocketExist(TEXT("Muzzle"))
		? WeaponMesh->GetSocketLocation(TEXT("Muzzle"))
		: GetActorLocation();
	AController* Ctrl = OwnerActor->GetInstigatorController();
	const FRotator Aim = Ctrl ? Ctrl->GetControlRotation() : OwnerActor->GetActorRotation();

	UE_LOG(LogTemp, Warning, TEXT("[BaseWeapon::Fire] BP route -> WeaponComponent::TryFire (Weapon=%s, Owner=%s)"),
		*GetName(), *OwnerActor->GetName());

	WC->TryFire(Muzzle, Aim, OwnerActor, 1.0f);
}

void ABaseWeapon::Reload()
{
	if (bIsOverHeat) return;

	CurrentReloadCount++;

	UE_LOG(LogTemp, Warning, TEXT("Reload! Count: %d / %d"),
		CurrentReloadCount, MaxReloadCount);

	if (CurrentReloadCount >= MaxReloadCount)
	{
		bIsOverHeat = true;
		if (OverheatParticleSystem) OverheatParticleSystem->ActivateSystem();
		UE_LOG(LogTemp, Warning, TEXT("OVERHEATED! %f seconds cooldown"), OverheatCooldown);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::Red,
				FString::Printf(TEXT("OVERHEATED! %.0fs cooldown"), OverheatCooldown));
		}

		GetWorld()->GetTimerManager().SetTimer(
			OverheatTimer,
			this,
			&ABaseWeapon::OnOverHeatEnd,
			OverheatCooldown,
			false);

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
	if (OverheatParticleSystem) OverheatParticleSystem->DeactivateSystem();
}

void ABaseWeapon::ResetFireCooldown()
{
	bIsFire = true;
}

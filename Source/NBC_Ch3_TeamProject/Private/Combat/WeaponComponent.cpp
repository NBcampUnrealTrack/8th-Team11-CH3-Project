#include "Combat/WeaponComponent.h"

#include "Combat/CombatTypes.h"        // ECC_Weapon
#include "Combat/HealthComponent.h"    // 적중 시 ApplyDamage 위임
#include "Combat/HitReactComponent.h"
#include "Combat/WeaponConfig.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::EquipWeapon(UWeaponConfig* NewWeapon)
{
	CurrentWeapon = NewWeapon;
	PendingRecoilPitch = 0.f;
	PendingRecoilYaw   = 0.f;

	OnWeaponEquipped.Broadcast(NewWeapon);
}

bool UWeaponComponent::CanFireNow() const
{
	const UWorld* World = GetWorld();
	if (!World) { return false; }

	const float Now      = World->GetTimeSeconds();
	const float Interval = CurrentWeapon ? CurrentWeapon->FireInterval : 0.15f;
	return (Now - LastFireTime) >= Interval;
}

bool UWeaponComponent::TryFire(const FVector& MuzzleLocation, const FRotator& AimRotation,
                               AActor* DamageInstigator, float SpreadMultiplier)
{
	UWorld* World = GetWorld();
	if (!World) { return false; }

	// CurrentWeapon이 GC됐거나 dangling이면 폴백 사용 — IsValid로 강제 체크.
	UWeaponConfig* Weapon = IsValid(CurrentWeapon) ? CurrentWeapon : nullptr;
	if (!Weapon && CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Weapon] CurrentWeapon is dangling — clearing"));
		CurrentWeapon = nullptr;
	}

	const float Now      = World->GetTimeSeconds();
	const float Interval = Weapon ? Weapon->FireInterval : 0.15f;
	if (Now - LastFireTime < Interval) { return false; }
	LastFireTime = Now;

	const FVector Forward = AimRotation.Vector();

	// Weapon 미장착 시 25dmg/단발 폴백.
	const int32 Pellets   = Weapon ? Weapon->PelletCount     : 1;
	const float SpreadDeg = Weapon ? Weapon->SpreadDegrees   : 0.f;
	const float Range     = Weapon ? Weapon->Range           : 5000.f;
	const float Damage    = Weapon ? Weapon->DamagePerPellet : 25.f;
	const float SpreadRad = FMath::DegreesToRadians(SpreadDeg * SpreadMultiplier);

	FVector  LastImpact   = MuzzleLocation + Forward * Range;
	AActor*  LastHitActor = nullptr;

	for (int32 i = 0; i < Pellets; ++i)
	{
		const FVector Dir = (Pellets == 1)
			? Forward
			: FMath::VRandCone(Forward, SpreadRad);

		FHitResult Hit;
		bool bHit = false;
		FireSinglePellet(MuzzleLocation, Dir, Range, Damage, DamageInstigator, Hit, bHit);

		if (bHit)
		{
			LastImpact   = Hit.ImpactPoint;
			LastHitActor = Hit.GetActor();
		}
	}

	if (Weapon)
	{
		PendingRecoilPitch += Weapon->RecoilPitch;
		const float J = Weapon->RecoilYawJitter;
		PendingRecoilYaw  += FMath::FRandRange(-J, J);
	}

	if (Weapon && Weapon->FireSound.IsValid())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Weapon->FireSound.Get(), MuzzleLocation);
	}

	OnWeaponFired.Broadcast(LastImpact, LastHitActor);
	return true;
}

void UWeaponComponent::FireSinglePellet(const FVector& Start, const FVector& Dir, float Range, float Damage,
                                        AActor* DamageInstigator, FHitResult& OutHit, bool& bOutHit)
{
	UWorld* World = GetWorld();
	if (!World) { bOutHit = false; return; }

	const FVector End = Start + Dir * Range;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponFire), false, DamageInstigator);
	Params.bReturnPhysicalMaterial = false;

	bOutHit = World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Weapon, Params);

#if !(UE_BUILD_SHIPPING)
	if (bDrawWeaponDebug)
	{
		DrawDebugLine(World, Start, bOutHit ? OutHit.ImpactPoint : End,
			bOutHit ? FColor::Red : FColor::Green, false, 0.4f, 0, 1.f);
		if (bOutHit)
		{
			DrawDebugSphere(World, OutHit.ImpactPoint, 8.f, 8, FColor::Yellow, false, 0.4f);
		}
	}
#endif

	if (bOutHit)
	{
		ApplyHitDamage(OutHit, Damage, DamageInstigator);
	}
}

void UWeaponComponent::ApplyHitDamage(const FHitResult& Hit, float Damage, AActor* DamageInstigator)
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor) { return; }

	// HealthComponent를 가진 액터(플레이어/좀비/파괴 가능 오브젝트)는 직접 위임.
	// BaseCharacter 강결합을 피하기 위해 동적 탐색 방식 사용.
	if (UHealthComponent* Health = HitActor->FindComponentByClass<UHealthComponent>())
	{
		if (!Health->IsDead())
		{
			Health->ApplyDamage(Damage);
			UE_LOG(LogTemp, Warning, TEXT("[Weapon] Hit %s for %.1f"), *HitActor->GetName(), Damage);

			if (UHitReactComponent* HitReact = HitActor->FindComponentByClass<UHitReactComponent>())
			{
				HitReact->PlayHitReact(Hit.BoneName, -Hit.ImpactNormal);
			}
		}
		return;
	}

	// HealthComponent 없으면 표준 PointDamage로 폴백 (월드 오브젝트 등).
	AController* InstigatorCtrl = nullptr;
	if (APawn* Pawn = Cast<APawn>(DamageInstigator))
	{
		InstigatorCtrl = Pawn->GetController();
	}
	const FVector Origin = DamageInstigator ? DamageInstigator->GetActorLocation() : FVector(Hit.TraceStart);
	UGameplayStatics::ApplyPointDamage(
		HitActor, Damage,
		(Hit.ImpactPoint - Origin).GetSafeNormal(),
		Hit, InstigatorCtrl, DamageInstigator, nullptr);
}

void UWeaponComponent::TickRecoil(float DeltaTime, float& OutPitchDelta, float& OutYawDelta)
{
	OutPitchDelta = 0.f;
	OutYawDelta   = 0.f;

	if (PendingRecoilPitch > KINDA_SMALL_NUMBER || FMath::Abs(PendingRecoilYaw) > KINDA_SMALL_NUMBER)
	{
		// AddControllerPitchInput는 음수가 위쪽.
		OutPitchDelta = -PendingRecoilPitch;
		OutYawDelta   = PendingRecoilYaw;

		const float Recover = CurrentWeapon ? CurrentWeapon->RecoilRecoverPerSec : 4.f;
		PendingRecoilPitch = FMath::FInterpTo(PendingRecoilPitch, 0.f, DeltaTime, Recover);
		PendingRecoilYaw   = FMath::FInterpTo(PendingRecoilYaw,   0.f, DeltaTime, Recover);
	}
}

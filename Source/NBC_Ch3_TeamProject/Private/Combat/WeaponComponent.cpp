#include "Combat/WeaponComponent.h"

#include "Combat/CombatTypes.h"        // ECC_Weapon
#include "Combat/HealthComponent.h"    // 적중 시 ApplyDamage 위임
#include "Combat/HitReactComponent.h"
#include "Combat/WeaponConfig.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkinnedAsset.h"
#include "ReferenceSkeleton.h"
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

// 발사 단일 진입점. 좌클릭·BP Fire·AnimNotify 모두 이 함수로 모인다.
// 흐름: FireInterval 컷 → 펠릿 루프(LineTrace + ApplyHitDamage) → 반동 누적 → 발사음 → OnWeaponFired.
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

	// 연사 컷 — FireInterval 미경과 시 즉시 false (DPS 한계).
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

	// 펠릿 루프 — 라이플은 1발(=Forward), 샷건은 VRandCone으로 산탄.
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

// 펠릿 1발 — ECC_Weapon 채널로 LineTrace, 적중 시 ApplyHitDamage 위임.
// Instigator를 트레이스 무시 액터로 넘겨 자기 자신 자해 방지.
void UWeaponComponent::FireSinglePellet(const FVector& Start, const FVector& Dir, float Range, float Damage,
                                        AActor* DamageInstigator, FHitResult& OutHit, bool& bOutHit)
{
	UWorld* World = GetWorld();
	if (!World) { bOutHit = false; return; }

	const FVector End = Start + Dir * Range;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponFire), false, DamageInstigator);
	Params.bReturnPhysicalMaterial = false;

	bOutHit = World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Weapon, Params);

	if (bDrawWeaponDebug)
	{
		DrawDebugLine(World, Start, bOutHit ? OutHit.ImpactPoint : End,
			bOutHit ? FColor::Red : FColor::Green, false, 0.4f, 0, 1.f);
		if (bOutHit)
		{
			DrawDebugSphere(World, OutHit.ImpactPoint, 8.f, 8, FColor::Yellow, false, 0.4f);
		}
	}

	if (bOutHit)
	{
		ApplyHitDamage(OutHit, Damage, DamageInstigator);
	}
}

// 적중 후 디스패치 — 본 해결 → 체력 → HitReact → 히트스톱 순. 한 곳에서 묶여있어 VFX·셰이크 추가 시 여기만 본다.
void UWeaponComponent::ApplyHitDamage(const FHitResult& Hit, float Damage, AActor* DamageInstigator)
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor) { return; }

	// BoneName 폴백 — 캡슐 등 본 정보 없는 컴포넌트에 맞아 BoneName=None일 때 가장 가까운 본 검색.
	FName ResolvedBone = Hit.BoneName;
	USkeletalMeshComponent* SkelMesh = HitActor->FindComponentByClass<USkeletalMeshComponent>();
	if (ResolvedBone.IsNone() && SkelMesh)
	{
		FVector BoneLoc;
		ResolvedBone = SkelMesh->FindClosestBone_K2(Hit.ImpactPoint, BoneLoc);

		// _End 본은 시각 효과 미미 → 부모 본으로 거슬러 올라감 (8단계 가드로 무한 루프 방지).
		int32 SafetyCounter = 8;
		while (!ResolvedBone.IsNone() && SafetyCounter-- > 0
			&& ResolvedBone.ToString().EndsWith(TEXT("_End")))
		{
			const FName Parent = SkelMesh->GetParentBone(ResolvedBone);
			if (Parent.IsNone() || Parent == ResolvedBone) break;
			ResolvedBone = Parent;
		}
	}

	// Unsafe 루트 본(Hips/Spine/spine_01 등)이 결정되면 SetAllBodiesBelow가 캐릭터 전체를 라그돌화하므로,
	// ImpactPoint에 가장 가까운 자식 본으로 내려가 시각 효과를 유지하면서 눕는 현상 방지.
	if (SkelMesh && !ResolvedBone.IsNone())
	{
		static const TSet<FName> UnsafeBones = {
			TEXT("Hips"), TEXT("Spine"), TEXT("Spine1"), TEXT("Spine2"),
			TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
			TEXT("root"),
		};

		int32 DescendCounter = 6;
		while (!ResolvedBone.IsNone() && UnsafeBones.Contains(ResolvedBone) && DescendCounter-- > 0)
		{
			const USkinnedAsset* SkinnedAsset = SkelMesh->GetSkinnedAsset();
			if (!SkinnedAsset) break;
			const FReferenceSkeleton& RefSkel = SkinnedAsset->GetRefSkeleton();
			const int32 ParentIdx = RefSkel.FindBoneIndex(ResolvedBone);
			if (ParentIdx == INDEX_NONE) break;

			float BestDistSq = FLT_MAX;
			FName BestChild = NAME_None;
			for (int32 i = 0; i < RefSkel.GetNum(); ++i)
			{
				if (RefSkel.GetParentIndex(i) != ParentIdx) continue;
				const FName ChildName = RefSkel.GetBoneName(i);
				const FVector ChildLoc = SkelMesh->GetBoneLocation(ChildName);
				const float DistSq = FVector::DistSquared(ChildLoc, Hit.ImpactPoint);
				if (DistSq < BestDistSq) { BestDistSq = DistSq; BestChild = ChildName; }
			}
			if (BestChild.IsNone()) break;
			ResolvedBone = BestChild;
		}
	}

	// HealthComponent를 가진 액터(플레이어/좀비/파괴 가능 오브젝트)는 직접 위임.
	// BaseCharacter 강결합을 피하기 위해 동적 탐색 방식 사용.
	if (UHealthComponent* Health = HitActor->FindComponentByClass<UHealthComponent>())
	{
		if (!Health->IsDead())
		{
			// ① 체력 차감
			Health->ApplyDamage(Damage);
			UE_LOG(LogTemp, Warning, TEXT("[Weapon] Hit %s for %.1f"), *HitActor->GetName(), Damage);

			// ② HitReact — 임펄스 방향은 -ImpactNormal(피격면 안쪽으로 밀어내기).
			if (UHitReactComponent* HitReact = HitActor->FindComponentByClass<UHitReactComponent>())
			{
				HitReact->PlayHitReact(ResolvedBone, -Hit.ImpactNormal);
			}

			// ③ 히트스톱 — 피격 액터·AIController·Instigator 동시 정지.
			ApplyHitStop(HitActor, DamageInstigator);
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

// 반동 회복 — Pawn Tick에서 호출. 누적치를 컨트롤러 입력으로 한 번 적용하고 FInterpTo로 점진 감쇠.
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

// 히트스톱 — 피격 액터·그 AIController·Instigator 셋의 CustomTimeDilation을 일시 변경.
// AIController가 빠지면 Pawn은 멈춰도 BT의 의사결정 Tick은 정상 속도로 돌아 추격 명령이 계속 누적된다 (시각적 끊김 없는 추격).
void UWeaponComponent::ApplyHitStop(AActor* HitActor, AActor* Instigator)
{
	UWorld* World = GetWorld();
	if (!World || !HitActor) { return; }
	if (HitStopDuration <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitStop] SKIP — HitStopDuration=%.3f (<=0)"), HitStopDuration);
		return;
	}

	// Pawn의 AIController도 함께 멈춰야 BT 의사결정이 같이 느려짐
	AController* HitController = nullptr;
	if (APawn* HitPawn = Cast<APawn>(HitActor))
	{
		HitController = HitPawn->GetController();
	}

	// 3중 적용 — Pawn(애니·이동), AIController(BT·BrainComponent), Instigator(시각적 무게감).
	HitActor->CustomTimeDilation = HitStopTimeScale;
	if (HitController)
	{
		HitController->CustomTimeDilation = HitStopTimeScale;
	}
	if (Instigator && Instigator != HitActor)
	{
		Instigator->CustomTimeDilation = HitStopTimeScale;
	}

	// 캡슐 ECC_Pawn 응답 일시 Ignore — 멈춘 좀비를 뒤의 좀비가 추격하며 캡슐로 밀어버리는 현상 방지.
	// CustomTimeDilation은 본인 Tick만 늦출 뿐 다른 좀비의 push까지 막지 못함.
	UCapsuleComponent* HitCapsule = nullptr;
	ECollisionResponse PrevPawnResp = ECR_Block;
	if (ACharacter* HitChar = Cast<ACharacter>(HitActor))
	{
		HitCapsule = HitChar->GetCapsuleComponent();
		if (HitCapsule)
		{
			PrevPawnResp = HitCapsule->GetCollisionResponseToChannel(ECC_Pawn);
			HitCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		}
	}

	// 복귀 타이머는 WeakLambda로 캡처 — 좀비가 도중에 파괴돼도 Crash 없음.
	TWeakObjectPtr<AActor> WeakHit  = HitActor;
	TWeakObjectPtr<AActor> WeakCtrl = HitController;
	TWeakObjectPtr<AActor> WeakInst = Instigator;
	TWeakObjectPtr<UCapsuleComponent> WeakCapsule = HitCapsule;
	const ECollisionResponse RestoreResp = PrevPawnResp;

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [WeakHit, WeakCtrl, WeakInst, WeakCapsule, RestoreResp]()
		{
			if (WeakHit.IsValid())  { WeakHit->CustomTimeDilation  = 1.f; }
			if (WeakCtrl.IsValid()) { WeakCtrl->CustomTimeDilation = 1.f; }
			if (WeakInst.IsValid()) { WeakInst->CustomTimeDilation = 1.f; }
			if (WeakCapsule.IsValid()) { WeakCapsule->SetCollisionResponseToChannel(ECC_Pawn, RestoreResp); }
		}),
		HitStopDuration,
		false);
}

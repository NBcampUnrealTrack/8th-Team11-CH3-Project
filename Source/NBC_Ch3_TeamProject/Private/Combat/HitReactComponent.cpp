#include "Combat/HitReactComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UHitReactComponent::UHitReactComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Mixamo(Hips/Spine/Spine1/Spine2)와 UE5 표준(pelvis/spine_01~03/root) 모두 차단 —
	// 이 본들은 SetAllBodiesBelow 시 상체 전체가 라그돌화되어 캐릭터가 눕는 현상을 일으킴.
	// WeaponComponent의 자식 본 폴백이 모두 실패한 경우의 마지막 방어선.
	ExcludedBones = {
		TEXT("Hips"), TEXT("Spine"), TEXT("Spine1"), TEXT("Spine2"),
		TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
		TEXT("root"),
	};
}

void UHitReactComponent::BeginPlay()
{
	Super::BeginPlay();

	// SkeletalMesh 1회 캐시 — 매 PlayHitReact 마다 FindComponentByClass 회피.
	if (AActor* Owner = GetOwner())
	{
		CachedMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	}

	// Curve를 Timeline에 바인딩 — Update에서 BlendWeight 갱신, Finished에서 SimulatePhysics OFF.
	if (BlendWeightCurve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("HandleTimelineUpdate"));
		BlendTimeline.AddInterpFloat(BlendWeightCurve, UpdateDelegate);

		FOnTimelineEvent FinishedDelegate;
		FinishedDelegate.BindUFunction(this, FName("HandleTimelineFinished"));
		BlendTimeline.SetTimelineFinishedFunc(FinishedDelegate);
	}
}

void UHitReactComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	BlendTimeline.TickTimeline(DeltaTime);
}

// 본 단위 피격 반응 시작. 흐름: 가드 검사 → 이전 본 정리(연사) → 상태 갱신 → Timeline 시작 → 임펄스 예약.
void UHitReactComponent::PlayHitReact(FName BoneName, const FVector& ImpulseDir)
{
	// 가드 — 컴포넌트 의존성·입력 유효성·정책(루트 본 제외) 검사.
	if (!CachedMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[HitReact] SKIP — CachedMesh is NULL"));
		return;
	}
	if (!BlendWeightCurve)
	{
		UE_LOG(LogTemp, Error, TEXT("[HitReact] SKIP — BlendWeightCurve is NULL"));
		return;
	}
	if (BoneName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitReact] SKIP — BoneName is None"));
		return;
	}
	if (ExcludedBones.Contains(BoneName))
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitReact] SKIP — %s is in ExcludedBones"), *BoneName.ToString());
		return;
	}

	// 연사 RESTART — 이전 본 진행 중 다른 본 맞으면 이전 본 즉시 정리 후 새 본으로 재시작.
	// (이게 없으면 첫 발만 흔들리고 후속 발은 SKIP 되어 게임이 느려 보임.)
	if (bIsReacting && !CurrentBoneName.IsNone() && CurrentBoneName != BoneName)
	{
		CachedMesh->SetAllBodiesBelowPhysicsBlendWeight(CurrentBoneName, 0.f);
		CachedMesh->SetAllBodiesBelowSimulatePhysics(CurrentBoneName, false);
	}

	bIsReacting = true;
	CurrentBoneName = BoneName;
	PendingImpulseDir = ImpulseDir.GetSafeNormal();

	// Curve 첫 키부터 재생 — 같은 본 연속 시도는 자연스러운 재시작 효과.
	BlendTimeline.PlayFromStart();
	OnHitReactStarted.Broadcast(BoneName);

	// 임펄스는 ImpulseDelay(0.048s) 후 적용 — Curve 피크 직전에 타격감 극대화.
	GetOwner()->GetWorldTimerManager().SetTimer(
		ImpulseTimerHandle, this,
		&UHitReactComponent::ApplyDelayedImpulse,
		ImpulseDelay, false);
}

// Timeline 매 프레임 — Curve(0~1)를 PhysicsBlendWeight로 적용. 0→1→0 형태의 종 곡선이라 자연스러운 진입·복귀.
void UHitReactComponent::HandleTimelineUpdate(float Value)
{
	if (!CachedMesh || CurrentBoneName.IsNone()) return;

	if (Value > 0.f)
	{
		CachedMesh->SetAllBodiesBelowSimulatePhysics(CurrentBoneName, true);
		CachedMesh->SetAllBodiesBelowPhysicsBlendWeight(CurrentBoneName, Value);
	}
}

// Timeline 종료 — SimulatePhysics OFF + Weight 0으로 애니메이션 자세 완전 복귀.
void UHitReactComponent::HandleTimelineFinished()
{
	if (CachedMesh && !CurrentBoneName.IsNone())
	{
		CachedMesh->SetAllBodiesBelowPhysicsBlendWeight(CurrentBoneName, 0.f);
		CachedMesh->SetAllBodiesBelowSimulatePhysics(CurrentBoneName, false);
	}
	bIsReacting = false;
	CurrentBoneName = NAME_None;
	OnHitReactFinished.Broadcast();
}

// 지연 임펄스 적용 — bAccelChange=true로 질량 무시(작은 본도 일관된 흔들림).
void UHitReactComponent::ApplyDelayedImpulse()
{
	if (!CachedMesh || CurrentBoneName.IsNone()) return;

	CachedMesh->AddImpulse(
		PendingImpulseDir * ImpulseStrength,
		CurrentBoneName,
		true);
}

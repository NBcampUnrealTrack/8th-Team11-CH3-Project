#include "Combat/HitReactComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UHitReactComponent::UHitReactComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	ExcludedBones = { TEXT("pelvis") };
}

void UHitReactComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		CachedMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	}

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

void UHitReactComponent::PlayHitReact(FName BoneName, const FVector& ImpulseDir)
{
	if (bIsReacting) return;
	if (!CachedMesh || !BlendWeightCurve) return;
	if (BoneName.IsNone()) return;
	if (ExcludedBones.Contains(BoneName)) return;

	bIsReacting = true;
	CurrentBoneName = BoneName;
	PendingImpulseDir = ImpulseDir.GetSafeNormal();

	BlendTimeline.PlayFromStart();
	OnHitReactStarted.Broadcast(BoneName);

	GetOwner()->GetWorldTimerManager().SetTimer(
		ImpulseTimerHandle, this,
		&UHitReactComponent::ApplyDelayedImpulse,
		ImpulseDelay, false);
}

void UHitReactComponent::HandleTimelineUpdate(float Value)
{
	if (!CachedMesh || CurrentBoneName.IsNone()) return;

	if (Value > 0.f)
	{
		CachedMesh->SetAllBodiesBelowSimulatePhysics(CurrentBoneName, true);
		CachedMesh->SetAllBodiesBelowPhysicsBlendWeight(CurrentBoneName, Value);
	}
}

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

void UHitReactComponent::ApplyDelayedImpulse()
{
	if (!CachedMesh || CurrentBoneName.IsNone()) return;

	CachedMesh->AddImpulse(
		PendingImpulseDir * ImpulseStrength,
		CurrentBoneName,
		true);
}

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "HitReactComponent.generated.h"

class UCurveFloat;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitReactStarted, FName, BoneName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitReactFinished);

// 본 단위 물리 블렌드 기반 피격 반응 컴포넌트.
// 부착된 액터에 USkeletalMeshComponent가 있어야 동작한다.
UCLASS(Blueprintable, ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class NBC_CH3_TEAMPROJECT_API UHitReactComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitReactComponent();

	// 피격 본과 임펄스 방향(발사 방향 정규화 벡터)을 받아 타임라인 시작.
	UFUNCTION(BlueprintCallable, Category="HitReact")
	void PlayHitReact(FName BoneName, const FVector& ImpulseDir);

	UFUNCTION(BlueprintPure, Category="HitReact")
	bool IsReacting() const { return bIsReacting; }

	UPROPERTY(BlueprintAssignable, Category="HitReact|Events")
	FOnHitReactStarted OnHitReactStarted;

	UPROPERTY(BlueprintAssignable, Category="HitReact|Events")
	FOnHitReactFinished OnHitReactFinished;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 0~1 블렌드 가중치 커브. 길이 = HitReact 지속시간.
	UPROPERTY(EditDefaultsOnly, Category="HitReact")
	TObjectPtr<UCurveFloat> BlendWeightCurve;

	UPROPERTY(EditDefaultsOnly, Category="HitReact")
	float ImpulseStrength = 10000.f;

	// 임펄스 적용 지연. 커브 두 번째 키 시간(0.048s)과 일치시키면 피크 직전에 타격감 극대화.
	UPROPERTY(EditDefaultsOnly, Category="HitReact")
	float ImpulseDelay = 0.048f;

	// 이 본들은 HitReact 무시 (pelvis 등 루트 본 전체 흔들림 방지).
	UPROPERTY(EditDefaultsOnly, Category="HitReact")
	TArray<FName> ExcludedBones;

private:
	UPROPERTY() TObjectPtr<USkeletalMeshComponent> CachedMesh;

	FTimeline BlendTimeline;
	FName CurrentBoneName;
	FVector PendingImpulseDir = FVector::ZeroVector;
	bool bIsReacting = false;

	UFUNCTION() void HandleTimelineUpdate(float Value);
	UFUNCTION() void HandleTimelineFinished();

	void ApplyDelayedImpulse();
	FTimerHandle ImpulseTimerHandle;
};

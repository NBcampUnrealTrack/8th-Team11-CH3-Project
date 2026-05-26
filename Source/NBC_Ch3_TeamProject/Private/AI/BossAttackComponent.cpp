#include "AI/BossAttackComponent.h"
#include "Combat/HealthComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"


UBossAttackComponent::UBossAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsAttacking = false;
	TargetSocketName = NAME_None;
	TraceRadius = 50.f;
	LastSocketLocation = FVector::ZeroVector;
	AttackDamage = 30.f;
}

void UBossAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	BossCharacter = Cast<ACharacter>(GetOwner());
}

void UBossAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UBossAttackComponent::StartMeleeTrace(FName InSocketName, float InRadius)
{
	if (!BossCharacter || !BossCharacter->GetMesh()) return;

	bIsAttacking = true;
	TargetSocketName = InSocketName;
	TraceRadius = InRadius;

	// 공격 시작 시점에 이미 맞았던 액터 리스트를 초기화
	AlreadyHitActors.Empty();

	LastSocketLocation = BossCharacter->GetMesh()->GetSocketLocation(TargetSocketName);
}

void UBossAttackComponent::EndMeleeTrace()
{
	bIsAttacking = false;
	AlreadyHitActors.Empty();
}

void UBossAttackComponent::ExecuteRadialSlam(FVector CenterLocation, float Radius, float DamageAmount)
{
	if (!BossCharacter)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(BossCharacter);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	// 단발성 구체 오버랩 검출
	bool bOverlap = World->OverlapMultiByObjectType(
		OverlapResults,
		CenterLocation,
		FQuat::Identity,
		ObjectParams,
		SphereShape,
		QueryParams
	);

	// 디버그용 구체
	DrawDebugSphere(World, CenterLocation, Radius, 32, FColor::Orange, false, 2.0f, 0, 1.5f);

	if (bOverlap)
	{
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			AActor* HitActor = Overlap.GetActor();
			if (HitActor)
			{
				if (UHealthComponent* Health = HitActor->FindComponentByClass<UHealthComponent>())
				{
					if (!Health->IsDead())
					{
						Health->ApplyDamage(DamageAmount);
					}
				}

				UGameplayStatics::ApplyDamage(
					HitActor,
					DamageAmount,
					BossCharacter->GetController(),
					BossCharacter,
					UDamageType::StaticClass()
				);
			}
		}
	}
}

// NotifyState에서 매 프레임 호출
void UBossAttackComponent::ExecuteMeleeTrace()
{
	if (!bIsAttacking || !BossCharacter || !BossCharacter->GetMesh()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 현재 프레임의 소켓 위치
	FVector CurrentSocketLocation = BossCharacter->GetMesh()->GetSocketLocation(TargetSocketName);

	// 중복 프레임 제거
	if (CurrentSocketLocation.Equals(LastSocketLocation, 0.1f)) return;

	TArray<FHitResult> HitResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(TraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(BossCharacter); // 보스 자신은 제외

	// 이미 맞은 애들도 트레이스 계산에서 제외
	for (AActor* HitActor : AlreadyHitActors)
	{
		if (HitActor)
		{
			QueryParams.AddIgnoredActor(HitActor);
		}
	}

	// 플레이어 판정을 위해 ECC_Pawn을 기본 사용
	// 추후 플레이어 피격용 전역 커스텀 채널 만들 예정(예: ECC_GameTraceChannel2)
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	bool bHit = World->SweepMultiByObjectType(
		HitResults,
		LastSocketLocation,
		CurrentSocketLocation,
		FQuat::Identity,
		ObjectParams,
		SphereShape,
		QueryParams
	);

	// 디버그 라인
	 DrawDebugSphere(World, CurrentSocketLocation, TraceRadius, 8, FColor::Red, false, 1.0f);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !AlreadyHitActors.Contains(HitActor))
			{
				// 중복 타격 리스트에 추가
				AlreadyHitActors.Add(HitActor);

				if (UHealthComponent* Health = HitActor->FindComponentByClass<UHealthComponent>())
				{
					if (!Health->IsDead())
					{
						Health->ApplyDamage(AttackDamage);
					}
				}

				// 플레이어의 TakeDamage / HealthComponent에게 전달
				UGameplayStatics::ApplyDamage(
					HitActor,
					AttackDamage, // 대미지 값 
					BossCharacter->GetController(),
					BossCharacter,
					UDamageType::StaticClass()
				);
			}
		}
	}

	// 다음 틱 연산을 위해 현재 소켓 위치를 지난 프레임 위치로 갱신
	LastSocketLocation = CurrentSocketLocation;
}


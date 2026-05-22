#include "AI/BTTask_Attack.h"
#include "AI/ZombieCharacter.h"
#include "AI/ZombieAIController.h"
#include "Combat/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack Player");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	AZombieAIController* ZombieAIController = Cast<AZombieAIController>(OwnerComp.GetAIOwner());
	AZombieCharacter* ZombieCharacter = Cast<AZombieCharacter>(ZombieAIController->GetPawn());
	if (!ZombieAIController || !ZombieCharacter)
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerCharacter)
	{
		ZombieAIController->SetFocus(PlayerCharacter);

		// 몽타주 재생
		UAnimInstance* AnimInstance = ZombieCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance && ZombieCharacter->AttackMontage.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, ZombieCharacter->AttackMontage.Num() - 1);
			ZombieAIController->MontageTime = AnimInstance->Montage_Play(ZombieCharacter->AttackMontage[RandomIndex]);
			ZombieAIController->StopMovement();
		}

		// [장식 추가] 거리·각도 게이트 통과 시 데미지 적용. WeaponComponent::ApplyHitDamage 패턴을 미러링.
		const FVector ZombieLoc = ZombieCharacter->GetActorLocation();
		const FVector PlayerLoc = PlayerCharacter->GetActorLocation();
		const float DistSq = FVector::DistSquared2D(ZombieLoc, PlayerLoc);
		const float Dist = FMath::Sqrt(DistSq);
		const float MaxDistSq = FMath::Square(ZombieCharacter->AttackRange);

		if (DistSq <= MaxDistSq)
		{
			FVector ToPlayer = PlayerLoc - ZombieLoc;
			ToPlayer.Z = 0.f;
			ToPlayer.Normalize();

			FVector Forward = ZombieCharacter->GetActorForwardVector();
			Forward.Z = 0.f;
			Forward.Normalize();

			const float Dot = FVector::DotProduct(Forward, ToPlayer);
			const float CosHalf = FMath::Cos(FMath::DegreesToRadians(ZombieCharacter->AttackHalfAngleDeg));
			const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));

			if (Dot >= CosHalf)
			{
				if (UHealthComponent* PlayerHealth = PlayerCharacter->FindComponentByClass<UHealthComponent>())
				{
					UE_LOG(LogCombat, Warning, TEXT("[BTTask_Attack] %s 게이트 PASS — Dist=%.1f/%.1f, Angle=%.1f°/%.1f°, Damage=%.1f"),
						*GetNameSafe(ZombieCharacter), Dist, ZombieCharacter->AttackRange, AngleDeg, ZombieCharacter->AttackHalfAngleDeg, ZombieCharacter->AttackDamage);
					PlayerHealth->ApplyDamage(ZombieCharacter->AttackDamage);
				}
				else
				{
					UE_LOG(LogCombat, Warning, TEXT("[BTTask_Attack] %s 게이트 PASS — 그러나 Player에 UHealthComponent 없음"), *GetNameSafe(ZombieCharacter));
				}
			}
			else
			{
				UE_LOG(LogCombat, Verbose, TEXT("[BTTask_Attack] %s 게이트 FAIL (각도) — Angle=%.1f° > Half=%.1f°"),
					*GetNameSafe(ZombieCharacter), AngleDeg, ZombieCharacter->AttackHalfAngleDeg);
			}
		}
		else
		{
			UE_LOG(LogCombat, Verbose, TEXT("[BTTask_Attack] %s 게이트 FAIL (거리) — Dist=%.1f > Range=%.1f"),
				*GetNameSafe(ZombieCharacter), Dist, ZombieCharacter->AttackRange);
		}

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

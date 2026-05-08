#include "AI/BTTask_Death.h"
#include "AI/ZombieCharacter.h"
#include "AI/ZombieAIController.h"

UBTTask_Death::UBTTask_Death()
{
	NodeName = TEXT("Zombie Death");
}

EBTNodeResult::Type UBTTask_Death::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AZombieAIController* ZombieAIController = Cast<AZombieAIController>(OwnerComp.GetAIOwner());
	AZombieCharacter* ZombieCharacter = Cast<AZombieCharacter>(ZombieAIController->GetPawn());
	if (!ZombieAIController || !ZombieCharacter)
	{
		return EBTNodeResult::Failed;
	}
	UAnimInstance* AnimInstance = ZombieCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && ZombieCharacter->DeathMontage.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ZombieCharacter->DeathMontage.Num() - 1);
		AnimInstance->Montage_Play(ZombieCharacter->DeathMontage[RandomIndex]);
	}

	return EBTNodeResult::Succeeded;
}

#include "AI/BTTask_Attack.h"
#include "AI/ZombieCharacter.h"
#include "AI/ZombieAIController.h"
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

		// ��Ÿ�� ���
		UAnimInstance* AnimInstance = ZombieCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance && ZombieCharacter->AttackMontage.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, ZombieCharacter->AttackMontage.Num() - 1);
			ZombieAIController->MontageTime = AnimInstance->Montage_Play(ZombieCharacter->AttackMontage[RandomIndex]);
			ZombieAIController->StopMovement();
		}

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

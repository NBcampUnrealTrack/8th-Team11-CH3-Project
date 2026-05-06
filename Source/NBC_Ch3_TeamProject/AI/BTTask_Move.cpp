#include "BTTask_Move.h"
#include "ZombieAIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Move::UBTTask_Move()
{
	NodeName = TEXT("MoveAndFaceTarget");
	AcceptanceRadius = 70.f;
}

EBTNodeResult::Type UBTTask_Move::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AZombieAIController* ZombieAIController = Cast<AZombieAIController>(OwnerComp.GetAIOwner());
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!ZombieAIController || !BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* PlayerCharacter = Cast<ACharacter>(BlackboardComp->GetValueAsObject(BBKeys::TargetActor));
	if (!PlayerCharacter)
	{
		return EBTNodeResult::Failed;
	}

	ZombieAIController->MoveToActor(PlayerCharacter, AcceptanceRadius);

	return EBTNodeResult::Succeeded;
}

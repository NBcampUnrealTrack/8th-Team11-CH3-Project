#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Move.generated.h"

UCLASS()
class NBC_CH3_TEAMPROJECT_API UBTTask_Move : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Move();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	float AcceptanceRadius;
};

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Death.generated.h"

UCLASS()
class NBC_CH3_TEAMPROJECT_API UBTTask_Death : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Death();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

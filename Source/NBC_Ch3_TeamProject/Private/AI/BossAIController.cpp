#include "AI/BossAIController.h"
#include "AI/BossCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

ABossAIController::ABossAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	bIsAttaking = false;
	MontageTime = 0.f;
	BossCharacter = nullptr;
	PlayerCharacter = nullptr;

	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void ABossAIController::BeginPlay()
{
	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

void ABossAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!PlayerCharacter || !BossCharacter)
	{
		return;
	}

	FVector BossLocation = BossCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	float Distance = FVector::Dist(BossLocation, PlayerLocation);

	// 블랙보드에 값 저장
	Blackboard->SetValueAsFloat(BBKeys_Boss::DistanceToPlayer, Distance);
	Blackboard->SetValueAsBool(BBKeys_Boss::bIsInSight, false);

	if (Distance <= 150.f)
	{
		bool bIsInSightBoss = IsCanAttackSight();
		// 범위내에 들어왔고 시야안에 있으면 True값을 블랙보드에 전달해서 BTTask_Attack 실행
		if (bIsAttaking)
		{
			Blackboard->SetValueAsBool(BBKeys_Boss::bIsInSight, bIsInSightBoss);
		}
		if (!bIsAttaking)// 범위내에 들어왔지만 시야내에 없는 경우 플레이어를 향해 회전을 해준다.
		{
			FRotator BossRotation = BossCharacter->GetActorRotation();
			FVector BossToPlayerDirection = PlayerLocation - BossLocation;
			FRotator TargetRotation = BossToPlayerDirection.Rotation();
			FRotator NewRotaion = BossRotation;

			NewRotaion.Yaw = FMath::RInterpTo(BossRotation, TargetRotation, DeltaSeconds, 5.f).Yaw;
			BossCharacter->SetActorRotation(NewRotaion);
		}
		UE_LOG(LogTemp, Warning, TEXT("A: %d, B: %d, C: %f"), bIsInSightBoss, bIsAttaking, Distance);
	}
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Possess할 때 
	BossCharacter = Cast<ABossCharacter>(GetPawn());

	// BP 자식 클래스에서 에셋을 할당했는지 확인 후 실행
	if (BTAsset)
	{
		// UseBlackboard는 내부적으로 Blackboard 데이터 에셋을 기반으로 초기화
		if (BTAsset->BlackboardAsset)
		{
			Blackboard->InitializeBlackboard(*(BTAsset->BlackboardAsset));
		}
		RunBehaviorTree(BTAsset);
	}

	// BeginPlay()에 넣으면 좀비를 직접 배치할 땐 되지만 스폰할 땐 안됨(Possess 되기 전에 실행을 해서 BB에 안들어감)
	// BTTask_MoveAndFaceTarget에서 쓸 BB 오브젝트 변수에 값 넣어줌
	Blackboard->SetValueAsObject(BBKeys_Boss::TargetActor, PlayerCharacter);
}

bool ABossAIController::IsCanAttackSight()
{
	if (!PlayerCharacter || !BossCharacter)
	{
		return false;
	}

	// 좀비의 정면 단위 방향 벡터
	FVector BossFowardDirection = BossCharacter->GetActorForwardVector();
	BossFowardDirection.Z = 0.f;
	BossFowardDirection.Normalize();
	// 좀비 - 플레이어 단위 방향 벡터
	FVector BossLocation = BossCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector BossToPlayerDirection = PlayerLocation - BossLocation;
	BossToPlayerDirection.Z = 0.f;
	BossToPlayerDirection.Normalize();

	float DotValue = FVector::DotProduct(BossFowardDirection, BossToPlayerDirection);

	// 시야안에 들어왔으면 공격중으로 상태를 바꾸고(bIsAttaking) 애니메이션 재생 시간 뒤에 공격이 끝났다고 바꿔줌
	if (DotValue >= 0.999f)
	{
		bIsAttaking = true;
		GetWorldTimerManager().SetTimer(
			AttackTimer,
			[this]()
			{
				bIsAttaking = false;
			},
			MontageTime,
			false
		);
		return true;
	}
	return false;
}



#include "AI/ZombieAIController.h"
#include "AI/ZombieCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AZombieAIController::AZombieAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerCharacter = nullptr;
	ZombieCharacter = nullptr;

	bIsAttaking = false;
	MontageTime = 0.f;

	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void AZombieAIController::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

void AZombieAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!PlayerCharacter || !ZombieCharacter)
	{
		return;
	}


	FVector ZombieLocation = ZombieCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	float Distance = FVector::Dist(ZombieLocation, PlayerLocation);

	// 블랙보드에 값 저장
	Blackboard->SetValueAsFloat(BBKeys::DistanceToPlayer, Distance);
	Blackboard->SetValueAsBool(BBKeys::bIsInSight, false);

	if (Distance <= 150.f)
	{
		bool bIsInSightZombie = IsCanAttackSight();
		// 범위내에 들어왔고 시야안에 있으면 True값을 블랙보드에 전달해서 BTTask_Attack 실행
		if (bIsAttaking)
		{
			Blackboard->SetValueAsBool(BBKeys::bIsInSight, bIsInSightZombie);
		}
		if (!bIsAttaking)// 범위내에 들어왔지만 시야내에 없는 경우 플레이어를 향해 회전을 해준다.
		{
			FRotator ZombieRotation = ZombieCharacter->GetActorRotation();
			FVector ZombieToPlayerDirection = PlayerLocation - ZombieLocation;
			FRotator TargetRotation = ZombieToPlayerDirection.Rotation();
			FRotator NewRotaion = ZombieRotation;

			NewRotaion.Yaw = FMath::RInterpTo(ZombieRotation, TargetRotation, DeltaSeconds, 5.f).Yaw;
			ZombieCharacter->SetActorRotation(NewRotaion);
		}
		//UE_LOG(LogTemp, Warning, TEXT("A: %d, B: %d, C: %f"), bIsInSightZombie, bIsAttaking, Distance);
	}
}

void AZombieAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Possess할 때 
	ZombieCharacter = Cast<AZombieCharacter>(GetPawn());
	
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
	Blackboard->SetValueAsObject(BBKeys::TargetActor, PlayerCharacter);
}

bool AZombieAIController::IsCanAttackSight()
{
	if (!PlayerCharacter || !ZombieCharacter)
	{
		return false;
	}

	// 좀비의 정면 단위 방향 벡터
	FVector ZombieFowardDirection = ZombieCharacter->GetActorForwardVector();
	ZombieFowardDirection.Z = 0.f;
	ZombieFowardDirection.Normalize();
	// 좀비 - 플레이어 단위 방향 벡터
	FVector ZombieLocation = ZombieCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector ZombieToPlayerDirection = PlayerLocation - ZombieLocation;
	ZombieToPlayerDirection.Z = 0.f;
	ZombieToPlayerDirection.Normalize();

	float DotValue = FVector::DotProduct(ZombieFowardDirection, ZombieToPlayerDirection);
	// 위와 같은 기능
	// float DotValue = ZombieFowardDirection | ZombieToPlayerDirection;

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

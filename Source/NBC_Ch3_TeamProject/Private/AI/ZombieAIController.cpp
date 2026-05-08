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
	ZombieCharacter = Cast<AZombieCharacter>(GetPawn());
	// BTTask_MoveAndFaceTarget���� �� BB ������Ʈ ������ �� �־���(�÷��̾ �Ѱ��شٰ� ���� ��)
	Blackboard->SetValueAsObject(BBKeys::TargetActor, PlayerCharacter);
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

	// �������忡 �� ����
	Blackboard->SetValueAsFloat(BBKeys::DistanceToPlayer, Distance);
	Blackboard->SetValueAsFloat(BBKeys::CurrentHP, ZombieCharacter->CurrentHP);
	Blackboard->SetValueAsBool(BBKeys::bIsInSight, false);

	if (Distance <= 150.f)
	{
		bool bIsInSightZombie = IsCanAttackSight();
		// �������� ���԰� �þ߾ȿ� ������ True���� �������忡 �����ؼ� BTTask_Attack ����
		if (bIsAttaking)
		{
			Blackboard->SetValueAsBool(BBKeys::bIsInSight, bIsInSightZombie);
		}
		if (!bIsAttaking)// �������� �������� �þ߳��� ���� ��� �÷��̾ ���� ȸ���� ���ش�.
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
	// BP �ڽ� Ŭ�������� ������ �Ҵ��ߴ��� Ȯ�� �� ����
	if (BTAsset)
	{
		// UseBlackboard�� ���������� Blackboard ������ ������ ������� �ʱ�ȭ
		if (BTAsset->BlackboardAsset)
		{
			Blackboard->InitializeBlackboard(*(BTAsset->BlackboardAsset));
		}
		RunBehaviorTree(BTAsset);
	}
}

bool AZombieAIController::IsCanAttackSight()
{
	if (!PlayerCharacter || !ZombieCharacter)
	{
		return false;
	}

	// ������ ���� ���� ���� ����
	FVector ZombieFowardDirection = ZombieCharacter->GetActorForwardVector();
	ZombieFowardDirection.Z = 0.f;
	ZombieFowardDirection.Normalize();

	// ���� - �÷��̾� ���� ���� ����
	FVector ZombieLocation = ZombieCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector ZombieToPlayerDirection = PlayerLocation - ZombieLocation;
	ZombieToPlayerDirection.Z = 0.f;
	ZombieToPlayerDirection.Normalize();

	// �� ���� ���� ���� ����
	float DotValue = FVector::DotProduct(ZombieFowardDirection, ZombieToPlayerDirection);
	// float DotValue = ZombieFowardDirection | ZombieToPlayerDirection; // ���� ���

	// �þ߾ȿ� �������� ���������� ���¸� �ٲٰ�(bIsAttaking) �ִϸ��̼� ��� �ð� �ڿ� false�� �ٲ���
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

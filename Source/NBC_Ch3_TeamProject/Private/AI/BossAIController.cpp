#include "AI/BossAIController.h"
#include "AI/BossCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"

ABossAIController::ABossAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	bIsAttaking = false;
	bIsInSightBoss = false;
	MontageTime = 0.f;
	BossCharacter = nullptr;
	PlayerCharacter = nullptr;
	bIsCanWalk = false;
	BossXYSpeed = 0.f;
	Distance = 0.f;
	PhaseTwoJumpRandomIndex = 1;
	PhaseThreeJumpRandomIndex = 1;
}

void ABossAIController::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABossAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!PlayerCharacter || !BossCharacter)
	{
		return;
	}

	FVector BossrVelocity = BossCharacter->GetCharacterMovement()->Velocity;
	BossXYSpeed = BossrVelocity.Size2D();
	if (BossXYSpeed > 0)
	{
		bIsCanWalk = true;
	}
	else 
	{
		bIsCanWalk = false;
	}

	EMovementMode MovementMode = BossCharacter->GetCharacterMovement()->MovementMode;
	UE_LOG(LogTemp, Warning, TEXT("MovementMode: %s"), *UEnum::GetValueAsString(MovementMode));

	FVector BossLocation = BossCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	Distance = FVector::Dist(BossLocation, PlayerLocation);
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("bIsInSightBoss: %d, bIsAttaking: %d, C: %f"), bIsInSightBoss, bIsAttaking, MontageTime));

	if (Distance <= 350.f)
	{
		bIsInSightBoss = IsCanAttackSight();
		
		if (!bIsAttaking)// 범위내에 들어왔지만 시야내에 없는 경우 플레이어를 향해 회전을 해준다.
		{
			UE_LOG(LogTemp, Warning, TEXT("TurnToPlayer"));
			TurnToPlayer(DeltaSeconds);
		}
	}
	else
	{
		bIsInSightBoss = false;

		if (BossCharacter->BossPhase == EBossPhase::Phase2)
		{
			PhaseTwoJumpRandomIndex = FMath::RandRange(0, 4); // 1/5 확률로 점프 공격
			if (PhaseTwoJumpRandomIndex == 0)
			{
				bIsInSightBoss = IsCanJumpAttackSight();
			}
		}
		if (BossCharacter->BossPhase == EBossPhase::Phase3)
		{
			PhaseThreeJumpRandomIndex = FMath::RandRange(0, 2); // 1/3 확률로 점프 공격
			if (PhaseThreeJumpRandomIndex == 0)
			{
				bIsInSightBoss = IsCanJumpAttackSight();
			}
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("bIsInSightBoss: %d, bIsAttaking: %d, C: %f"), bIsInSightBoss, bIsAttaking, Distance);
}

void ABossAIController::TurnToPlayer(float DeltaSeconds)
{
	FVector BossLocation = BossCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	FRotator BossRotation = BossCharacter->GetActorRotation();
	FVector BossToPlayerDirection = PlayerLocation - BossLocation;
	FRotator TargetRotation = BossToPlayerDirection.Rotation();
	FRotator NewRotaion = BossRotation;

	NewRotaion.Yaw = FMath::RInterpTo(BossRotation, TargetRotation, DeltaSeconds, 5.f).Yaw;
	BossCharacter->SetActorRotation(NewRotaion);
}

void ABossAIController::BossMoveToActor()
{
	BossCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	BossCharacter->GetCharacterMovement()->bCheatFlying = false;
	MoveToActor(PlayerCharacter, 150.f);
}

void ABossAIController::BossAttack()
{
	if (BossCharacter->BossPhase == EBossPhase::Phase1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Phase1"));
		UAnimInstance* AnimInstance = BossCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance && BossCharacter->AttackMontagePhaseOne.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, BossCharacter->AttackMontagePhaseOne.Num() - 1);
			MontageTime = AnimInstance->Montage_Play(BossCharacter->AttackMontagePhaseOne[RandomIndex]);
			StopMovement();
		}
	}

	if (BossCharacter->BossPhase == EBossPhase::Phase2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Phase2"));

		UAnimInstance* AnimInstance = BossCharacter->GetMesh()->GetAnimInstance();
		if (PhaseTwoJumpRandomIndex == 0)
		{
			BossJumpAttack();
			PhaseTwoJumpRandomIndex = 1;
		}
		else 
		{
			if (AnimInstance && BossCharacter->AttackMontagePhaseTwoToThree.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, BossCharacter->AttackMontagePhaseTwoToThree.Num() - 1);
				MontageTime = AnimInstance->Montage_Play(BossCharacter->AttackMontagePhaseTwoToThree[RandomIndex]);
				StopMovement();
			}
		}
	}

	if (BossCharacter->BossPhase == EBossPhase::Phase3)
	{
		UE_LOG(LogTemp, Warning, TEXT("Phase3"));
		
		UAnimInstance* AnimInstance = BossCharacter->GetMesh()->GetAnimInstance();
		if (PhaseThreeJumpRandomIndex == 0)
		{
			BossJumpAttack();
			PhaseThreeJumpRandomIndex = 1;
		}
		else
		{
			if (AnimInstance && BossCharacter->AttackMontagePhaseTwoToThree.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, BossCharacter->AttackMontagePhaseTwoToThree.Num() - 1);
				MontageTime = AnimInstance->Montage_Play(BossCharacter->AttackMontagePhaseTwoToThree[RandomIndex]);
				StopMovement();
			}
		}
	}
}

void ABossAIController::ChangePhaseTwo()
{
	UAnimInstance* AnimInstance = BossCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && BossCharacter->ChangePhaseTwo)
	{
		MontageTime = AnimInstance->Montage_Play(BossCharacter->ChangePhaseTwo);
	}
}

void ABossAIController::ChangePhaseThree()
{
	UAnimInstance* AnimInstance = BossCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && BossCharacter->ChangePhaseThree)
	{
		MontageTime = AnimInstance->Montage_Play(BossCharacter->ChangePhaseThree);
	}
}

void ABossAIController::BossJumpAttack()
{
	// Flying을 안해주면 점프 준비자세(z축이 살짝 내려갔다가 올라오기 때문에 착지했다고 판정)에서
	// Ground로 인식해버리고 캡슐이 땅에 고정됨
	BossCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	BossCharacter->GetCharacterMovement()->bCheatFlying = true;
	UAnimInstance* AnimInstance = BossCharacter->GetMesh()->GetAnimInstance();

	if (Distance < 250.f)
	{
		if (AnimInstance && BossCharacter->JumpAttackMontageOne)
		{
			MontageTime = AnimInstance->Montage_Play(BossCharacter->JumpAttackMontageOne);
		}
	}
	else if (Distance < 750.f)
	{
		if (AnimInstance && BossCharacter->JumpAttackMontageTwo)
		{
			MontageTime = AnimInstance->Montage_Play(BossCharacter->JumpAttackMontageTwo);
		}
	}
	else if (Distance < 1250.f)
	{
		if (AnimInstance && BossCharacter->JumpAttackMontageThree)
		{
			MontageTime = AnimInstance->Montage_Play(BossCharacter->JumpAttackMontageThree);
		}
	}
	else
	{
		if (AnimInstance && BossCharacter->JumpAttackMontageFour)
		{
			MontageTime = AnimInstance->Montage_Play(BossCharacter->JumpAttackMontageFour);
		}
	}
	
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	BossCharacter = Cast<ABossCharacter>(GetPawn());

}

bool ABossAIController::IsCanAttackSight()
{
	if (!PlayerCharacter || !BossCharacter)
	{
		return false;
	}

	// 보스의 정면 단위 방향 벡터
	FVector BossFowardDirection = BossCharacter->GetActorForwardVector();
	BossFowardDirection.Z = 0.f;
	BossFowardDirection.Normalize();
	// 보스 - 플레이어 단위 방향 벡터
	FVector BossLocation = BossCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector BossToPlayerDirection = PlayerLocation - BossLocation;
	BossToPlayerDirection.Z = 0.f;
	BossToPlayerDirection.Normalize();

	float DotValue = FVector::DotProduct(BossFowardDirection, BossToPlayerDirection);

	if (DotValue >= 0.900f)
	{
		// 시야안에 들어왔으면 공격중으로 상태를 바꾸고(bIsAttaking) 애니메이션 재생 시간 뒤에 공격이 끝났다고 바꿔줌
		bIsAttaking = true;
		UE_LOG(LogTemp, Warning, TEXT("Attacking1"));
		GetWorldTimerManager().SetTimer(
			AttackTimer,
			[this]()
			{
				UE_LOG(LogTemp, Warning, TEXT("Attacking2"));
				bIsAttaking = false;
			},
			MontageTime,
			false
		);
		return true;
	}
	
	return false;
}

bool ABossAIController::IsCanJumpAttackSight()
{
	if (!PlayerCharacter || !BossCharacter)
	{
		return false;
	}

	// 보스의 정면 단위 방향 벡터
	FVector BossFowardDirection = BossCharacter->GetActorForwardVector();
	BossFowardDirection.Z = 0.f;
	BossFowardDirection.Normalize();
	// 보스 - 플레이어 단위 방향 벡터
	FVector BossLocation = BossCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector BossToPlayerDirection = PlayerLocation - BossLocation;
	BossToPlayerDirection.Z = 0.f;
	BossToPlayerDirection.Normalize();

	float DotValue = FVector::DotProduct(BossFowardDirection, BossToPlayerDirection);

	if (DotValue >= 0.999f)
	{
		// 시야안에 들어왔으면 공격중으로 상태를 바꾸고(bIsAttaking) 애니메이션 재생 시간 뒤에 공격이 끝났다고 바꿔줌
		bIsAttaking = true;
		UE_LOG(LogTemp, Warning, TEXT("JumpAtacking1"));
		GetWorldTimerManager().SetTimer(
			AttackTimer,
			[this]()
			{
				UE_LOG(LogTemp, Warning, TEXT("JumpAttacking2"));
				bIsAttaking = false;
			},
			MontageTime,
			false
		);
		return true;
	}

	return false;
}


void ABossAIController::TriggerPhaseTransition()
{
	// 2. 클래스 이름(StateTreeAIComponent)으로 블루프린트에 추가된 컴포넌트를 안전하게 검색합니다.
	UClass* TargetClass = StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/Script/GameplayStateTreeModule.StateTreeAIComponent"));
	if (!TargetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("1"));
		return;
	}
	
	UActorComponent* FoundComponent = GetComponentByClass(TargetClass);

	if (FoundComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("2"));
		// 3. 내부 함수인 "SendStateTreeEvent"를 이름으로 직접 찾아옵니다.
		UFunction* SendEventFunc = FoundComponent->GetClass()->FindFunctionByName(TEXT("SendStateTreeEvent"));
		if (SendEventFunc)
		{
			UE_LOG(LogTemp, Warning, TEXT("3"));
			// State Tree 루트 트랜지션에 등록한 태그와 동일하게 맞춰줍니다.
			FGameplayTag TransitionTag = FGameplayTag::RequestGameplayTag(FName("Boss.Event.PhaseTransition"));

			// SendStateTreeEvent(FGameplayTag EventTag) 함수의 인자 구조를 매핑합니다.
			struct FStateTreeEventParameters
			{
				FGameplayTag EventTag;
			};

			FStateTreeEventParameters Params;
			Params.EventTag = TransitionTag;

			// 리플렉션으로 함수를 강제 실행합니다.
			FoundComponent->ProcessEvent(SendEventFunc, &Params);

			UE_LOG(LogTemp, Warning, TEXT("Successfully bypassed header errors and sent Phase Transition Tag!"));
		}
	}
}
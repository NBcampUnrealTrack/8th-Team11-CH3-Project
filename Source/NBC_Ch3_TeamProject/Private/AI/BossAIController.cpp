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
	//UE_LOG(LogTemp, Warning, TEXT("MovementMode: %s"), *UEnum::GetValueAsString(MovementMode));

	FVector BossLocation = BossCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	Distance = FVector::Dist(BossLocation, PlayerLocation);
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("bIsInSightBoss: %d, bIsAttaking: %d, C: %f"), bIsInSightBoss, bIsAttaking, MontageTime));

	if (Distance <= 350.f)
	{
		bIsInSightBoss = IsCanAttackSight();
		
		if (!bIsAttaking)// 범위내에 들어왔지만 시야내에 없는 경우 플레이어를 향해 회전을 해준다.
		{
			//UE_LOG(LogTemp, Warning, TEXT("TurnToPlayer"));
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
		//UE_LOG(LogTemp, Warning, TEXT("Phase1"));
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
		//UE_LOG(LogTemp, Warning, TEXT("Phase2"));

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
		//UE_LOG(LogTemp, Warning, TEXT("Phase3"));
		
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
		//UE_LOG(LogTemp, Warning, TEXT("Attacking1"));
		GetWorldTimerManager().SetTimer(
			AttackTimer,
			[this]()
			{
				//UE_LOG(LogTemp, Warning, TEXT("Attacking2"));
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
		//UE_LOG(LogTemp, Warning, TEXT("JumpAtacking1"));
		GetWorldTimerManager().SetTimer(
			AttackTimer,
			[this]()
			{
				//UE_LOG(LogTemp, Warning, TEXT("JumpAttacking2"));
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
	// 1. 월드 및 기본 방어
	if (!GetWorld()) return;

	// 2. StateTreeAIComponent 클래스 동적 로드
	UClass* TargetClass = StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/Script/GameplayStateTreeModule.StateTreeAIComponent"));
	if (!TargetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[PhaseTransition] StateTreeAIComponent 클래스를 로드하지 못했습니다!"));
		return;
	}

	// 3. 컨트롤러에 해당 컴포넌트가 실제로 붙어있는지 확인
	UActorComponent* FoundComponent = GetComponentByClass(TargetClass);
	if (!FoundComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[PhaseTransition] 이 컨트롤러에 StateTreeAIComponent가 배치되지 않았습니다. 블루프린트를 확인하세요!"));
		return;
	}

	// 4. 컴포넌트의 클래스 정보가 안전한지 한 번 더 확인
	UClass* ComponentClass = FoundComponent->GetClass();
	if (!ComponentClass) return;

	// 5. "SendStateTreeEvent" 함수가 실제로 존재하는지 확인
	UFunction* SendEventFunc = ComponentClass->FindFunctionByName(TEXT("SendStateTreeEvent"));
	if (!SendEventFunc)
	{
		UE_LOG(LogTemp, Error, TEXT("[PhaseTransition] SendStateTreeEvent 함수를 찾을 수 없습니다!"));
		return;
	}

	// 6. 모든 게 안전함이 확인되었으니 실행
	FGameplayTag TransitionTag = FGameplayTag::RequestGameplayTag(FName("Boss.Event.PhaseTransition"));

	// FStateTreeEvent 메모리 정렬에 맞춘 구조체 (태그 + 오리진)
	struct FDynamicStateTreeEvent
	{
		FGameplayTag Tag;
		FName Origin;
	};

	FDynamicStateTreeEvent Params;
	Params.Tag = TransitionTag;
	Params.Origin = NAME_None;

	// 최종 실행
	FoundComponent->ProcessEvent(SendEventFunc, &Params);

	UE_LOG(LogTemp, Warning, TEXT("[PhaseTransition] 페이즈 태그를 안전하게 전달했습니다."));
}
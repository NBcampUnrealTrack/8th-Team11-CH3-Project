#include "AI/ZombieCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->AddRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->AddRelativeRotation(FRotator(0.f, -90.f, 0.f));

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = 50.f;

		// 회전 속도 (Yaw) 조절 - 뚝뚝 끊기는 현상 방지
		MoveComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f); // 수치가 낮을수록 회전이 부드러워짐
		// 이동 방향으로 자동 회전 설정
		MoveComp->bOrientRotationToMovement = true;
		// 컨트롤러 회전 사용 안 함 (캐릭터가 휙 돌아가는 것 방지)
		bUseControllerRotationYaw = false;
		// 군중 회피(RVO Avoidance) 활성화 - 좀비끼리 겹치는 현상 방지
		MoveComp->bUseRVOAvoidance = true;
		MoveComp->AvoidanceWeight = 0.3f;
	}
}

float AZombieCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.f, MaxHP);

	if (CurrentHP == 0.f)
	{
		//OnDeath();
	}

	return ActualDamage;
}

//void AZombieCharacter::OnDeath()
//{
//	AZombieAIController* ZombieAIController = Cast<AZombieAIController>(GetController());
//	if (!ZombieAIController)
//	{
//		return;
//	}
//	ZombieAIController->StopMovement();
//	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//
//	SetLifeSpan(3.0f);
//}
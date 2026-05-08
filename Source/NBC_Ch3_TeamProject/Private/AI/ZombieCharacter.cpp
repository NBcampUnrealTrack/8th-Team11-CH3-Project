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

		// ȸ�� �ӵ� (Yaw) ���� - �Ҷ� ����� ���� ����
		MoveComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f); // ��ġ�� �������� ȸ���� �ε巯����
		// �̵� �������� �ڵ� ȸ�� ����
		MoveComp->bOrientRotationToMovement = true;
		// ��Ʈ�ѷ� ȸ�� ��� �� �� (ĳ���Ͱ� �� ���ư��� �� ����)
		bUseControllerRotationYaw = false;
		// ���� ȸ��(RVO Avoidance) Ȱ��ȭ - ���񳢸� ��ġ�� ���� ����
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
#include "AI/BossCharacter.h"
#include "AI/BossAIController.h"
#include "Combat/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ABossCharacter::ABossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	BossAI = nullptr;
	BossPhase = EBossPhase::Phase1;
	CurrentPhase = 0;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	BossMaxHealth = HealthComponent->GetMaxHealth();
	BossCurrentHealth = BossMaxHealth;

	GetMesh()->AddRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->AddRelativeRotation(FRotator(0.f, -90.f, 0.f));

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = 500.f;

		// 회전 속도 (Yaw) 조절 - 뚝뚝 끊기는 현상 방지
		MoveComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f); // 수치가 낮을수록 회전이 부드러워짐
		// 이동 방향으로 자동 회전 설정
		MoveComp->bOrientRotationToMovement = true;
		// 컨트롤러 회전 사용 안 함 (캐릭터가 휙 돌아가는 것 방지)
		bUseControllerRotationYaw = false;
	}
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	BossAI = Cast<ABossAIController>(GetController());
}

void ABossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (BossCurrentHealth > 0.7f * BossMaxHealth && CurrentPhase == 0)
	{
		CurrentPhase = 1;
		BossPhase = EBossPhase::Phase1;
		if (BossAI)
		{
			BossAI->TriggerPhaseTransition();
		}
	}
	else if (BossCurrentHealth <= 0.7f * BossMaxHealth && BossCurrentHealth > 0.3f * BossMaxHealth && CurrentPhase == 1)
	{
		CurrentPhase = 2;
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
		BossPhase = EBossPhase::Phase2;
		if (BossAI)
		{
			BossAI->TriggerPhaseTransition();
		}
	}
	else if (BossCurrentHealth <= 0.3f * BossMaxHealth && CurrentPhase == 2)
	{
		CurrentPhase = 3;
		GetCharacterMovement()->MaxWalkSpeed = 900.f;
		BossPhase = EBossPhase::Phase3;
		if (BossAI)
		{
			BossAI->TriggerPhaseTransition();
		}
	}
}

void ABossCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ABossCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	return ActualDamage;
}


void ABossCharacter::OnHit(float CurrentHealth)
{

}

void ABossCharacter::OnDeath()
{

}
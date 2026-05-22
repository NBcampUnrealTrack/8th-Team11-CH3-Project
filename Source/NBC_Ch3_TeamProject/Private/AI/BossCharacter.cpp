#include "AI/BossCharacter.h"
#include "AI/BossAIController.h"
#include "System/NBC_GameMode.h"
#include "Combat/HealthComponent.h"
#include "Combat/HitReactComponent.h"
#include "Combat/CombatTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ABossCharacter::ABossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	BossAI = nullptr;
	BossPhase = EBossPhase::Phase1;
	CurrentPhase = 0;

	HitReactComponent = CreateDefaultSubobject<UHitReactComponent>(TEXT("HitReactComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	

	GetMesh()->AddRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->AddRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// Mesh 콜리전 — 라인트레이스로 본 정보 받고 물리 시뮬레이션 가능하도록 Query and Physics
	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionObjectType(ECC_Pawn);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_Weapon, ECR_Block);
	MeshComp->SetGenerateOverlapEvents(true);

	// Capsule은 라인트레이스 무시 — Mesh가 본 단위로 받아야 HitReact가 본별로 동작
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CapsuleComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CapsuleComp->SetCollisionResponseToChannel(ECC_Weapon, ECR_Ignore);

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

	BossMaxHealth = HealthComponent->GetMaxHealth();
	BossCurrentHealth = BossMaxHealth;
	
	BossAI = Cast<ABossAIController>(GetController());
	HealthComponent->OnHealthChanged.AddDynamic(this, &ABossCharacter::OnHit);
	HealthComponent->OnDeath.AddDynamic(this, &ABossCharacter::OnDeath);
}

void ABossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BossCurrentHealth = HealthComponent->GetCurrentHealth();

	if (BossCurrentHealth > 0.7f * BossMaxHealth && CurrentPhase == 0)
	{
		CurrentPhase = 1;
		BossPhase = EBossPhase::Phase1;
		UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *UEnum::GetValueAsString(BossPhase));
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
		UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *UEnum::GetValueAsString(BossPhase));
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
		UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *UEnum::GetValueAsString(BossPhase));
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
	if (!BossAI)
	{
		return;
	}

	// ABP 가져오기
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitMontage.Num() > 0)
	{
		// 몽타주 플레이
		int32 RandomIndex = FMath::RandRange(0, HitMontage.Num() - 1);
		AnimInstance->Montage_Play(HitMontage[RandomIndex]);
	}
}

void ABossCharacter::OnDeath()
{
	if (!BossAI)
	{
		return;
	}
	// 이동 멈추고 Collision 없애기
	BossAI->StopMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ABP 가져오기
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage.Num() > 0)
	{
		// 몽타주 플레이
		int32 RandomIndex = FMath::RandRange(0, DeathMontage.Num() - 1);
		AnimInstance->Montage_Play(DeathMontage[RandomIndex]);
	}

	// 몽타주 재생시간을 주기 위해 3초 후 Destroy() 실행
	SetLifeSpan(3.0f);
}
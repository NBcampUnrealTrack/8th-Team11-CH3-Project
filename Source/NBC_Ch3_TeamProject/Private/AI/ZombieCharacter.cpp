#include "AI/ZombieCharacter.h"
#include "AI/ZombieAIController.h"
#include "System/NBC_GameMode.h"
#include "Combat/HealthComponent.h"
#include "Combat/HitReactComponent.h"
#include "Combat/CombatTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HitReactComponent = CreateDefaultSubobject<UHitReactComponent>(TEXT("HitReactComponent"));

	GetMesh()->AddRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->AddRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// Mesh 콜리전 — 라인트레이스로 본 정보 받고 물리 시뮬레이션 가능하도록 Query and Physics
	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionObjectType(ECC_Pawn);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
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

void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChanged.AddDynamic(this, &AZombieCharacter::OnHit);
	HealthComponent->OnDeath.AddDynamic(this, &AZombieCharacter::OnDeath);
	if (UWorld* World = GetWorld())
	{
		// GameMode 가져오기
		ANBC_GameMode* MyGM = Cast<ANBC_GameMode>(World->GetAuthGameMode());

		if (MyGM)
		{
			HealthComponent->OnDeath.AddDynamic(MyGM, &ANBC_GameMode::OnMonsterKilled);
		}
	}
	
}

float AZombieCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	return ActualDamage;
}

void AZombieCharacter::OnHit(float CurrentHealth)
{
	AZombieAIController* ZombieAIController = Cast<AZombieAIController>(GetController());
	if (!ZombieAIController)
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

void AZombieCharacter::OnDeath()
{
	AZombieAIController* ZombieAIController = Cast<AZombieAIController>(GetController());
	if (!ZombieAIController)
	{
		return;
	}
	// 이동 멈추고 Collision 없애기
	ZombieAIController->StopMovement();
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



// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "Combat/WeaponComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/DamageCameraShake.h"
#include "Player/PlayerControllerClass.h"
#include "Player/BaseWeapon.h"
#include "Player/Rifle.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System/NBC_GameMode.h"
#include "Kismet/GameplayStatics.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	
	NormalSpeed = 300.0f;
	SprintMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintMultiplier;
	
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));

	// [장식 추가] 체력 컴포넌트 부착. BP에서 별도 부착하지 않도록 C++ 단일 소유.
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// [장식 추가] 피격 카메라 셰이크 기본값. BP에서 다른 셰이크로 덮어쓸 수 있음.
	DamageCameraShake = UDamageCameraShake::StaticClass();
	
	bIsFiring = false;
	bIsAiming = false;
	DefaultFOV = 90.0f;
	AimFOV = 45.0f;
	AimSpeed = 10.0f;
	DefaultArmLength = 300.0f;
	AimArmLength = 150.0f;
	DefaultSocketOffset = FVector(0.0f, 0.0f, 0.0f);
	AimSocketOffset = FVector(0.0f, 60.0f, 20.0f);
	
	
	CrouchSpeed = 200.0f;
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->CrouchedHalfHeight = 44.0f;
	
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// [장식 추가] 사망 + 체력 변경 이벤트 구독
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &APlayerCharacter::OnPlayerDeath);
		HealthComponent->OnHealthChanged.AddDynamic(this, &APlayerCharacter::OnPlayerHealthChanged);
		LastKnownHealth = HealthComponent->GetCurrentHealth();
	}

	CurrentWeaponIndex = -1;

	if (bGiveTestWeaponsOnBeginPlay)
	{
		for (TSubclassOf<ABaseWeapon> WeaponClass : TestStartWeapon)
		{
			AddWeaponToInventory(WeaponClass);
		}

		if (WeaponInventory.Num() > 0)
		{
			SwitchWeapon(0);
		}
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 발사 처리
    // 발사 처리
if (bIsFiring && WeaponComponent)
{
    if (WeaponInventory.IsValidIndex(CurrentWeaponIndex))
    {
        ABaseWeapon* CurWeapon = WeaponInventory[CurrentWeaponIndex];

        if (CurWeapon && !CurWeapon->bIsOverHeat && !bIsReloading && CurWeapon->CurrentBulletCount > 0)
        {
            //  1. 카메라 중앙에서 먼저 Line Trace
            APlayerController* PC = Cast<APlayerController>(GetController());
            FVector CamLoc;
            FRotator CamRot;
            PC->GetPlayerViewPoint(CamLoc, CamRot);

            FVector CamTraceEnd = CamLoc + CamRot.Vector() * 10000.0f;
            FHitResult CamHit;
            FCollisionQueryParams CamParams;
            CamParams.AddIgnoredActor(this);

            bool bCamHit = GetWorld()->LineTraceSingleByChannel(
                CamHit, CamLoc, CamTraceEnd, ECC_Visibility, CamParams);

            //  2. 카메라 트레이스 결과를 목표 지점으로
            FVector TargetPoint = bCamHit ? CamHit.ImpactPoint : CamTraceEnd;

            //  3. 총구 위치 가져오기
            FVector MuzzleLocation = GetActorLocation();
            if (CurWeapon->WeaponMesh && CurWeapon->WeaponMesh->DoesSocketExist(TEXT("Muzzle")))
            {
                MuzzleLocation = CurWeapon->WeaponMesh->GetSocketLocation(TEXT("Muzzle"));
            }

            //  4. 총구 → 목표 지점 방향으로 발사
            FRotator AimRotation = (TargetPoint - MuzzleLocation).GetSafeNormal().Rotation();

            float SpreadMult = bIsAiming ? 0.3f : 1.0f;
            if (ARifle* RifleWeapon = Cast<ARifle>(CurWeapon))
            {
                SpreadMult *= (RifleWeapon->CurrentSpread / RifleWeapon->MinSpread);
            }

            const bool bFired = WeaponComponent->TryFire(
                MuzzleLocation,
                AimRotation,
                this,
                SpreadMult
            );

            if (bFired)
            {
                if (ARifle* RifleWeapon = Cast<ARifle>(CurWeapon))
                {
                    RifleWeapon->CurrentSpread = FMath::Min(
                        RifleWeapon->CurrentSpread + RifleWeapon->SpreadIncrease,
                        RifleWeapon->MaxSpread
                    );
                }
                CurWeapon->CurrentBulletCount = FMath::Max(
                    CurWeapon->CurrentBulletCount - 1, 0);
            }
        }
    }
}

    // 반동 처리
    if (WeaponComponent)
    {
        float PitchDelta, YawDelta;
        WeaponComponent->TickRecoil(DeltaTime, PitchDelta, YawDelta);
        AddControllerPitchInput(PitchDelta);
        AddControllerYawInput(YawDelta);
    }

    // FOV 전환
    if (Camera)
    {
        float TargetFOV = bIsAiming ? AimFOV : DefaultFOV;
        float NewFOV = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaTime, AimSpeed);
        Camera->SetFieldOfView(NewFOV);
    }

    // SpringArm
    if (SpringArm)
    {
        float TargetLength = bIsAiming ? AimArmLength : DefaultArmLength;
        SpringArm->TargetArmLength = FMath::FInterpTo(
            SpringArm->TargetArmLength, TargetLength, DeltaTime, AimSpeed);

        FVector TargetOffset = bIsAiming ? AimSocketOffset : DefaultSocketOffset;
        SpringArm->SocketOffset = FMath::VInterpTo(
            SpringArm->SocketOffset, TargetOffset, DeltaTime, AimSpeed);
    }
}


void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (APlayerControllerClass* PlayerController = Cast<APlayerControllerClass>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerCharacter::Move);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerCharacter::Look);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerCharacter::StartJump);

				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&APlayerCharacter::StopJump);
			}
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerCharacter::StartSprint);

				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&APlayerCharacter::StopSprint);
			}
			// Fire - Started로 시작, Completed로 정지
			if (PlayerController->FireAction)
			{
				EnhancedInput->BindAction(
					PlayerController->FireAction,
					ETriggerEvent::Started,
					this,
					&APlayerCharacter::StartFire);
    
				EnhancedInput->BindAction(
					PlayerController->FireAction,
					ETriggerEvent::Completed,
					this,
					&APlayerCharacter::StopFire);
			}

			// 무기 교체 - Started (한 번만)
			if (PlayerController->NextWeaponAction)
			{
				EnhancedInput->BindAction(
					PlayerController->NextWeaponAction,
					ETriggerEvent::Started,
					this,
					&APlayerCharacter::SwitchToNextWeapon);
			}

			if (PlayerController->PrevWeaponAction)
			{
				EnhancedInput->BindAction(
					PlayerController->PrevWeaponAction,
					ETriggerEvent::Started,
					this,
					&APlayerCharacter::SwitchToPrevWeapon);
			}

			if (PlayerController->Weapon1Action)
			{
				EnhancedInput->BindAction(
					PlayerController->Weapon1Action,
					ETriggerEvent::Started,
					this,
					&APlayerCharacter::SelectWeapon1);
			}

			if (PlayerController->Weapon2Action)
			{
				EnhancedInput->BindAction(
					PlayerController->Weapon2Action,
					ETriggerEvent::Started,
					this,
					&APlayerCharacter::SelectWeapon2);
			}

			if (PlayerController->Weapon3Action)
			{
				EnhancedInput->BindAction(
					PlayerController->Weapon3Action,
					ETriggerEvent::Started,
					this,
					&APlayerCharacter::SelectWeapon3);
			}
			
			if (PlayerController->AimAction)
			{
				EnhancedInput->BindAction(
					PlayerController->AimAction,
					ETriggerEvent::Started,
					this,
					&APlayerCharacter::StartAim);
    
				EnhancedInput->BindAction(
					PlayerController->AimAction,
					ETriggerEvent::Completed,
					this,
					&APlayerCharacter::StopAim);
			}
			if (PlayerController->ReloadAction)
			{
				EnhancedInput->BindAction(
					PlayerController->ReloadAction,
					ETriggerEvent::Started,
					this,
					&APlayerCharacter::ReloadWeapon);
			}
			
			if (PlayerController->SitAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SitAction,
					ETriggerEvent::Started,
					this,
					&APlayerCharacter::StartCrouch);
				
				EnhancedInput->BindAction(
					PlayerController->SitAction,
					ETriggerEvent::Completed,
					this,
					&APlayerCharacter::StopCrouch);
			}
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
	if (!Controller) return;
	
	const FVector2D InputVector = value.Get<FVector2D>();
	
	if (!FMath::IsNearlyZero(InputVector.X))
	{
		AddMovementInput(GetActorForwardVector(), InputVector.X);
	}
	if (!FMath::IsNearlyZero(InputVector.Y))
	{
		AddMovementInput(GetActorRightVector(), InputVector.Y);
	}
}

void APlayerCharacter::StartJump(const FInputActionValue& value)
{
	if (value.Get<bool>())
	{
		Jump();
	}
}

void APlayerCharacter::StopJump(const FInputActionValue& value)
{
	if (!value.Get<bool>())
	{
		StopJumping();
	}
}

void APlayerCharacter::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();
	
	AddControllerYawInput(LookInput.X);
	
	AddControllerPitchInput(LookInput.Y);
}

void APlayerCharacter::StartSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void APlayerCharacter::StopSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	}
}

void APlayerCharacter::StartFire()
{
	UE_LOG(LogTemp, Warning, TEXT("StartFire called!")); 
	bIsFiring = true;
}

void APlayerCharacter::StopFire()
{
	UE_LOG(LogTemp, Warning, TEXT("StopFire called!")); 
	bIsFiring = false;
}

void APlayerCharacter::StartCrouch()
{
	Crouch();
	bIsCrouching = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
}

void APlayerCharacter::StopCrouch()
{
	UnCrouch();
	bIsCrouching = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void APlayerCharacter::ReloadWeapon()
{
	if (bIsReloading) return;

	if (!WeaponInventory.IsValidIndex(CurrentWeaponIndex)) return;

	ABaseWeapon* CurWeapon = WeaponInventory[CurrentWeaponIndex];
	if (!CurWeapon) return;

	if (CurWeapon->bIsOverHeat) return;
	if (CurWeapon->CurrentBulletCount >= CurWeapon->MaxBulletCount) return;

	bIsReloading = true;  // ← 모든 체크 통과 후 여기서 설정

	UAnimMontage* MontageToPlay = bIsCrouching ? CrouchReload : ReloadMontage;
	if (MontageToPlay)
	{
		float Duration = PlayAnimMontage(MontageToPlay);
		GetWorldTimerManager().SetTimer(
			ReloadTimer,
			this,
			&APlayerCharacter::FinishReload,
			Duration > 0.0f ? Duration : 0.1f,
			false);
	}
	else
	{
		FinishReload();
	}
}

void APlayerCharacter::FinishReload()
{
	if (!WeaponInventory.IsValidIndex(CurrentWeaponIndex)) return;
	ABaseWeapon* CurWeapon = WeaponInventory[CurrentWeaponIndex];
	if (CurWeapon) CurWeapon->Reload();
	bIsReloading = false;
}

void APlayerCharacter::SwitchWeapon(int32 index)
{
	// 잘못된 인덱스일 때 무시하기
	if (!WeaponInventory.IsValidIndex(index)) return;
	
	// 현재무기일 때 무시
	if (index == CurrentWeaponIndex) return;
	
	// 교체중일 때 무시
	if (bIsSwitch) return;
	
	// 무기를 안들고 있을 때 함수
	if (CurrentWeaponIndex == -1)
	{
		CurrentWeaponIndex = index;
		WeaponInventory[index]->SetActorHiddenInGame(false);
		
		if (WeaponComponent)
		{
			ABaseWeapon* CurrentWeapon = WeaponInventory[CurrentWeaponIndex];
			if (CurrentWeapon && CurrentWeapon->WeaponConfig)
			{
				WeaponComponent->EquipWeapon(CurrentWeapon->WeaponConfig);
			}
		}
		return;
	}
	
	
	bIsFiring = false;
	//교체 시작
	bIsSwitch = true;
	
	NextWeaponIndex = index;
	
	if (SwitchWeaponMontage && GetMesh()->GetAnimInstance())
	{
		float Duration = GetMesh()->GetAnimInstance()->Montage_Play(SwitchWeaponMontage);
		if (Duration > 0.0f)
		{
			GetWorldTimerManager().SetTimer(
				SwitchWeaponTimer,
				this,
				&APlayerCharacter::OnSwitchAnimComplete,
				Duration,
				false);
		}
		else
		{
			OnSwitchAnimComplete();
		}
	}
	else
	{
		OnSwitchAnimComplete();
	}
}

// 다음무기로
void APlayerCharacter::SwitchToNextWeapon()
{
	if (WeaponInventory.Num() == 0) return;
	
	int32 NextIndex = (CurrentWeaponIndex + 1) % WeaponInventory.Num();
	
	SwitchWeapon(NextIndex);
}

// 이전무기로
void APlayerCharacter::SwitchToPrevWeapon()
{
	if (WeaponInventory.Num() == 0) return;
	
	int32 PrevIndex = CurrentWeaponIndex - 1;
	
	if (PrevIndex < 0)
	{
		PrevIndex = WeaponInventory.Num() - 1;
	}
	
	SwitchWeapon(PrevIndex);
}

void APlayerCharacter::OnSwitchAnimComplete()
{
	if (!WeaponInventory.IsValidIndex(NextWeaponIndex))
	{
		bIsSwitch = false;
		return;
	}
	
	if (WeaponInventory.IsValidIndex(CurrentWeaponIndex))
	{
		WeaponInventory[CurrentWeaponIndex]->SetActorHiddenInGame(true);
	}
	
	CurrentWeaponIndex = NextWeaponIndex;
	
	WeaponInventory[CurrentWeaponIndex]->SetActorHiddenInGame(false);
	
	if (WeaponComponent && WeaponInventory.IsValidIndex(CurrentWeaponIndex))
	{
		ABaseWeapon* CurrentWeapon = WeaponInventory[CurrentWeaponIndex];
		if (CurrentWeapon && CurrentWeapon->WeaponConfig)
		{
			WeaponComponent->EquipWeapon(CurrentWeapon->WeaponConfig);
		}
	}
	
	bIsSwitch = false;
	
	UE_LOG(LogTemp, Warning, TEXT("Weapon switched to: %d"), CurrentWeaponIndex);
}

//숫자키1
void APlayerCharacter::SelectWeapon1()
{
	SwitchWeapon(0);
}

void APlayerCharacter::SelectWeapon2()
{
	SwitchWeapon(1);
}

void APlayerCharacter::SelectWeapon3()
{
	SwitchWeapon(2);
}

void APlayerCharacter::StartAim()
{
	bIsAiming = true;
	UE_LOG(LogTemp, Warning, TEXT("Aiming!"));
}

void APlayerCharacter::StopAim()
{
	bIsAiming = false;
	UE_LOG(LogTemp, Warning, TEXT("Aiming End!"));
}

// [장식 추가] 체력 감소 시 카메라 셰이크 발동 (회복은 무시)
void APlayerCharacter::OnPlayerHealthChanged(float NewHealth)
{
	const bool bDamaged = NewHealth < LastKnownHealth;
	LastKnownHealth = NewHealth;

	if (bDamaged && DamageCameraShake)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ClientStartCameraShake(DamageCameraShake);
		}
	}
}

// [장식 추가] 사망 시 GameMode에 게임오버 전환 위임 + 입력 차단
void APlayerCharacter::OnPlayerDeath()
{
	UE_LOG(LogCombat, Warning, TEXT("[Player] OnPlayerDeath — GameMode에 게임오버 위임"));

	bIsFiring = false;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	if (ANBC_GameMode* GM = Cast<ANBC_GameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->OnPlayerDied();
	}
}

// [창욱 수정] 합연산을 곱연산으로 수정 
void APlayerCharacter::IncreaseMovementSpeed(float Amount)
{
	if (Amount <= 0.0f) return;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement) return;

	Movement->MaxWalkSpeed *= Amount;
	Movement->MaxWalkSpeedCrouched += Amount * 0.5f; // 앉기 속도도 같이 증가
}

bool APlayerCharacter::AddWeaponToInventory(TSubclassOf<ABaseWeapon> WeaponClass)
{
	if (!WeaponClass || !GetWorld()) return false;
	
	FActorSpawnParameters SpawnParam;
	SpawnParam.Owner = this;
	SpawnParam.Instigator = this;
	
	ABaseWeapon* NewWeapon = GetWorld()->SpawnActor<ABaseWeapon>(
		WeaponClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParam);
	
	if (!NewWeapon) return false;
	
	NewWeapon->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TEXT("WeaponSocket"));
	
	NewWeapon->WeaponMesh->SetRelativeRotation(NewWeapon->MeshRotation);
	NewWeapon->SetActorHiddenInGame(true);
	WeaponInventory.Add(NewWeapon);
	return true;
}

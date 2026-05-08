// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "Player/PlayerControllerClass.h"
#include "Player/BaseWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	
	NormalSpeed = 600.0f;
	SprintMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintMultiplier;
	
	bIsFiring = false;
	
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	for (TSubclassOf<ABaseWeapon> Weapon : StartWeapon)
	{
		if (!Weapon) continue;
		
		FActorSpawnParameters SpawnParam;
		SpawnParam.Owner = this;
		
		ABaseWeapon* NewWeapon = GetWorld()->SpawnActor<ABaseWeapon>(
			Weapon,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParam);
		
		if (!NewWeapon) continue;
		
		NewWeapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("WeaponSocket"));
		
		NewWeapon->SetActorHiddenInGame(true);
		
		WeaponInventory.Add(NewWeapon);
	}
	
	CurrentWeaponIndex = -1;
	
	if (WeaponInventory.Num() > 0)
	{
		SwitchWeapon(0);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsFiring && WeaponInventory.IsValidIndex(CurrentWeaponIndex))
	{
		WeaponInventory[CurrentWeaponIndex]->Fire();
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
	
	if (WeaponInventory.IsValidIndex(CurrentWeaponIndex))
	{
		WeaponInventory[CurrentWeaponIndex]->Fire();
	}
}

void APlayerCharacter::StopFire()
{
	UE_LOG(LogTemp, Warning, TEXT("StopFire called!")); 
	bIsFiring = false;
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
		return;
	}
	
	//교체 시작
	bIsSwitch = true;
	
	NextWeaponIndex = index;
	
	if (SwitchWeaponMontage && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(SwitchWeaponMontage);
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

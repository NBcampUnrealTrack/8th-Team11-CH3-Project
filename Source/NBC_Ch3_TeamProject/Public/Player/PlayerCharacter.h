// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ABaseWeapon;
class UWeaponComponent;

struct FInputActionValue;

UCLASS()
class NBC_CH3_TEAMPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	APlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UWeaponComponent* WeaponComponent;

	float MaxHealth;

	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintMultiplier;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed;

	//무기
	UPROPERTY(EditAnywhere, Category = "Weapon|Text")
	bool bGiveTestWeaponsOnBeginPlay = true;
	UPROPERTY(EditAnywhere, Category = "Weapon|Test")
	TArray<TSubclassOf<ABaseWeapon>> TestStartWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<ABaseWeapon*> WeaponInventory;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentWeaponIndex;
	
	// 교체상태
	UPROPERTY(visibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsSwitch;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 NextWeaponIndex;
	
	bool bIsFiring;
	
	// 조준상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim")
	bool bIsAiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	float DefaultFOV;  // 기본시야
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	float AimFOV;  // 조준 시야
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	float AimSpeed; // 조준 속도
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* SwitchWeaponMontage;
	

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void StartJump(const FInputActionValue& value);
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);
	UFUNCTION()
	void StartFire();
	UFUNCTION()
	void StopFire();
	
	
	//무기 함수
	UFUNCTION()
	void SwitchWeapon(int32 index);
	UFUNCTION()
	void SwitchToNextWeapon();
	UFUNCTION()
	void SwitchToPrevWeapon();
	
	//무기 교체 애니메이션(무기 교체 딜레이) 후 무기 변경 예정
	UFUNCTION()
	void OnSwitchAnimComplete();
	
	//1,2,3번으로 무기 스위칭
	void SelectWeapon1();
	void SelectWeapon2();
	void SelectWeapon3();
	
	// 조준 함수
	UFUNCTION()
	void StartAim();
	UFUNCTION()
	void StopAim();
	
	UFUNCTION(BlueprintCallable)
	bool AddWeaponToInventory(TSubclassOf<ABaseWeapon> WeaponClass);
};

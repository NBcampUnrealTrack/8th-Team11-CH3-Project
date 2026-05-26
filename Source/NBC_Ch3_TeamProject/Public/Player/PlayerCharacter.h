// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ABaseWeapon;
class UWeaponComponent;
class UHealthComponent;
class UCameraShakeBase;

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

	// [장식 추가] 플레이어 체력 컴포넌트 (좀비/보스 피격 진입점)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHealthComponent> HealthComponent;

	// [장식 추가] 피격 시 재생할 카메라 셰이크. BP에서 할당.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback")
	TSubclassOf<UCameraShakeBase> DamageCameraShake;

	// [장식 추가] WBP_HUD가 체력바·비네팅 바인딩에 사용하는 getter
	UFUNCTION(BlueprintPure, Category = "Combat")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintMultiplier;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchSpeed; // 앉기 스피드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsCrouching;

	//무기
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bGiveTestWeaponsOnBeginPlay = true;
	UPROPERTY(EditAnywhere, Category = "Weapon")
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
	FTimerHandle SwitchWeaponTimer;
	
	FTimerHandle ReloadTimer;
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	float DefaultArmLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	float AimArmLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	FVector AimSocketOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	FVector DefaultSocketOffset;

	
	bool bIsReloading = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* SwitchWeaponMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UAnimMontage* CrouchReload;
	

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
	UFUNCTION()
	void StartCrouch();
	UFUNCTION()
	void StopCrouch();
	UFUNCTION()
	void ReloadWeapon();
	UFUNCTION()
	void FinishReload();
	
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

	// [장식 추가] HealthComponent::OnDeath 바인딩용 사망 처리 핸들러
	UFUNCTION()
	void OnPlayerDeath();

	// [장식 추가] HealthComponent::OnHealthChanged 바인딩. 체력 감소 시 카메라 셰이크 발동.
	UFUNCTION()
	void OnPlayerHealthChanged(float NewHealth);

	// [창욱 수정] private에서 public으로 이동 
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void IncreaseMovementSpeed(float Amount);

private:
	// [장식 추가] 직전 체력 — 감소(피격)와 회복을 구분하기 위함
	float LastKnownHealth = 0.f;
};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

class UWeaponConfig;

// 무기 장착 이벤트 (해제 시 NewWeapon=nullptr).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, UWeaponConfig*, NewWeapon);

// 무기 발사 이벤트 (마지막 펠릿 임팩트 좌표 + 마지막 적중 액터).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFired, FVector, ImpactPoint, AActor*, HitActor);

// 무기 시스템 캡슐화. WeaponConfig 보관 + LineTrace 발사 + 데미지 적용 + 반동 누적/회복.
// HealthComponent를 가진 액터를 적중하면 HealthComponent::ApplyDamage 직접 호출,
// 그 외엔 표준 UE PointDamage로 폴백.
UCLASS(Blueprintable, ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class NBC_CH3_TEAMPROJECT_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

	// 무기 장착. CurrentWeapon 설정 + OnWeaponEquipped Broadcast.
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void EquipWeapon(UWeaponConfig* NewWeapon);

	// 발사 시도. FireInterval 컷이면 false.
	// MuzzleLocation: 트레이스 시작점, AimRotation: 정조준 방향,
	// DamageInstigator: 데미지 가한 액터(Pawn 권장), SpreadMultiplier: 동적 산탄 배율.
	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool TryFire(const FVector& MuzzleLocation, const FRotator& AimRotation,
	             AActor* DamageInstigator, float SpreadMultiplier = 1.0f);

	// 누적 반동 회복 + 이번 프레임 입력 델타 반환. Pawn이 컨트롤러에 적용한다.
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void TickRecoil(float DeltaTime, float& OutPitchDelta, float& OutYawDelta);

	UFUNCTION(BlueprintPure, Category="Weapon")
	UWeaponConfig* GetCurrentWeapon() const { return CurrentWeapon; }

	// FireInterval 경과 후 발사 가능 여부.
	UFUNCTION(BlueprintPure, Category="Weapon")
	bool CanFireNow() const;

	UPROPERTY(BlueprintAssignable, Category="Weapon")
	FOnWeaponEquipped OnWeaponEquipped;

	UPROPERTY(BlueprintAssignable, Category="Weapon")
	FOnWeaponFired OnWeaponFired;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	TObjectPtr<UWeaponConfig> CurrentWeapon;

	// 직전 발사 시각 (FireInterval 컷 판정용).
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon")
	float LastFireTime = -1000.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon")
	float PendingRecoilPitch = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon")
	float PendingRecoilYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Debug")
	bool bDrawWeaponDebug = true;

private:
	// 단일 펠릿 LineTrace + 데미지 (TryFire 산탄 루프에서 호출).
	void FireSinglePellet(const FVector& Start, const FVector& Dir, float Range, float Damage,
	                      AActor* DamageInstigator, FHitResult& OutHit, bool& bOutHit);

	// 적중한 액터에 데미지 적용. HealthComponent 우선, 없으면 PointDamage 폴백.
	void ApplyHitDamage(const FHitResult& Hit, float Damage, AActor* DamageInstigator);
};

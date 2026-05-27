#pragma once

#include "CoreMinimal.h"
#include "System/BaseDataAsset.h"
#include "WeaponConfig.generated.h"

class USoundBase;
class UCameraShakeBase;
class UTexture2D;

// 무기 분류 — 같은 컴포넌트에서 라이플/샷건/권총을 구분할 때 사용.
UENUM(BlueprintType)
enum class EWeaponClass : uint8
{
	Rifle    UMETA(DisplayName="Rifle"),
	Shotgun  UMETA(DisplayName="Shotgun"),
	Pistol   UMETA(DisplayName="Pistol")
};

// 무기 발사·반동·소음 파라미터 DataAsset.
// 인스턴스마다 별도 DA를 만들어 WeaponComponent에 할당한다.
UCLASS(BlueprintType)
class NBC_CH3_TEAMPROJECT_API UWeaponConfig : public UBaseDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Identity")
	EWeaponClass WeaponClass = EWeaponClass::Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Identity")
	FName WeaponId = NAME_None;

	// 한 번 발사할 때 나가는 펠릿 개수 (샷건은 8~12).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Fire", meta=(ClampMin="1"))
	int32 PelletCount = 1;

	// 펠릿이 퍼지는 반각(도). 0이면 정확히 정조준 방향.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Fire", meta=(ClampMin="0.0"))
	float SpreadDegrees = 0.f;

	// 트레이스 최대 사거리(cm).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Fire", meta=(ClampMin="0.0"))
	float Range = 5000.f;

	// 펠릿 1발당 데미지.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Fire", meta=(ClampMin="0.0"))
	float DamagePerPellet = 12.f;

	// 다음 사격까지 최소 간격(초). 연사속도의 역수.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Fire", meta=(ClampMin="0.05"))
	float FireInterval = 0.12f;

	// 발사당 위로 튀는 반동 양(deg).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Recoil", meta=(ClampMin="0.0"))
	float RecoilPitch = 1.5f;

	// 발사당 좌우 반동 jitter 폭(deg). +/- 이 범위에서 랜덤.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Recoil", meta=(ClampMin="0.0"))
	float RecoilYawJitter = 0.6f;

	// 초당 반동 회복 속도(FInterpTo 계수).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Recoil", meta=(ClampMin="0.1"))
	float RecoilRecoverPerSec = 4.f;

	// AI 청각 감지 반경(cm). MakeNoise 등에서 활용.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Noise", meta=(ClampMin="0.0"))
	float NoiseRadius = 4500.f;

	// 발사 사운드 (Soft 참조 — 메모리 절약).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|FX")
	TSoftObjectPtr<USoundBase> FireSound;

	// 카메라 흔들림 클래스 (Soft 참조).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|FX")
	TSoftClassPtr<UCameraShakeBase> CameraShakeClass;
};

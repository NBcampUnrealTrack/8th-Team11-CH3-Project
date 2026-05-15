#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BossCharacter.generated.h"

class UHealthComponent;

UCLASS()
class NBC_CH3_TEAMPROJECT_API ABossCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABossCharacter();

	// HealthComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;

	// GameplayStatics::ApplyDamage와 같은 파이프라인
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void OnHit(float CurrentHealth);
	UFUNCTION()
	void OnDeath();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};

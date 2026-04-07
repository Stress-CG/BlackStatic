#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSInfectedCharacter.generated.h"

class UBSNoiseEmitterComponent;
class UBSNoiseListenerComponent;
class UBSSurvivorStateComponent;

UCLASS()
class BLACKSTATIC_API ABSInfectedCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABSInfectedCharacter();

	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBSSurvivorStateComponent> SurvivorStateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBSNoiseListenerComponent> NoiseListenerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBSNoiseEmitterComponent> NoiseEmitterComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase0")
	EBSFaction Faction = EBSFaction::Infected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase0")
	float SightRadius = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase0")
	float AttackRange = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase0")
	float AttackDamage = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase0")
	float AttackCooldownSeconds = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase0")
	float RoamRadius = 950.0f;

	UFUNCTION(BlueprintCallable, Category = "Phase0")
	bool TryAttackTarget(AActor* TargetActor);

private:
	UFUNCTION()
	void HandleDeath();

	float LastAttackTimeSeconds = -1000.0f;
};

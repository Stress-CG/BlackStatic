#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Phase0/Interfaces/BSInteractableInterface.h"
#include "BSWaterPowerObjectiveActor.generated.h"

class ABSInfectedCharacter;
class UBSTaskDefinition;
class UBSNoiseEmitterComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class BLACKSTATIC_API ABSWaterPowerObjectiveActor : public AActor, public IBSInteractableInterface
{
	GENERATED_BODY()

public:
	ABSWaterPowerObjectiveActor();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void ActivateObjective(int32 PartySize);

	UFUNCTION(BlueprintPure, Category = "Objective")
	bool IsObjectiveActive() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	TObjectPtr<UBSTaskDefinition> TaskDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	TSubclassOf<ABSInfectedCharacter> InfectedClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	float ObjectiveDefenseRadius = 950.0f;

private:
	void SpawnThreatWave(int32 PartySize);

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TObjectPtr<UBSNoiseEmitterComponent> NoiseEmitterComponent;

	UPROPERTY()
	TArray<TObjectPtr<ABSInfectedCharacter>> SpawnedThreats;

	bool bObjectiveActive = false;
	bool bSiteCompleted = false;
};

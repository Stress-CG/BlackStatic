#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Phase0/Interfaces/BSInteractableInterface.h"
#include "BSTaskBoardActor.generated.h"

class ABSWaterPowerObjectiveActor;
class UBSTaskDefinition;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class BLACKSTATIC_API ABSTaskBoardActor : public AActor, public IBSInteractableInterface
{
	GENERATED_BODY()

public:
	ABSTaskBoardActor();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	TObjectPtr<UBSTaskDefinition> TaskDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	TObjectPtr<ABSWaterPowerObjectiveActor> LinkedObjective;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	float PartyAssemblyRadius = 600.0f;

private:
	int32 EstimatePartySize() const;

	UPROPERTY(VisibleAnywhere, Category = "Task")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Task")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};

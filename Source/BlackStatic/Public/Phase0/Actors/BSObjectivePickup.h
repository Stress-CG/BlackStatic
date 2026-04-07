#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Phase0/Interfaces/BSInteractableInterface.h"
#include "BSObjectivePickup.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UBSItemDefinition;

UCLASS()
class BLACKSTATIC_API ABSObjectivePickup : public AActor, public IBSInteractableInterface
{
	GENERATED_BODY()

public:
	ABSObjectivePickup();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UBSItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	float PickupNoiseLoudness = 0.15f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Phase0/Interfaces/BSInteractableInterface.h"
#include "BSExtractionPoint.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class BLACKSTATIC_API ABSExtractionPoint : public AActor, public IBSInteractableInterface
{
	GENERATED_BODY()

public:
	ABSExtractionPoint();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Extraction")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Extraction")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
